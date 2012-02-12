// ------------------------------ THINGS.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "things.h"
#include "globals.h"
#include "cars.h"
#include "userctl.h"

#include <stdio.h>
#include <llkey.h>
#include <sincos.h>

// ------------------------------

THN_PThingMap THN_ThingMap  = NULL;
THN_PThing    THN_ThingList = NULL;
THN_PThing    THN_ThinkerList = NULL;


PRIVATE THN_TBounds TreeBounds = {
    1 << 21,
    0, {
        { 0, 0 },
    },
};

PRIVATE struct {
//    word        ntype;
    byte        nTable;
    char        name[6];
    THN_PBounds bounds;
} Obstacles[] = {
    { 0, "tree00", &TreeBounds},
    { 0, "barr00", &TreeBounds},
    { 1, "barr00", &TreeBounds},
    { 2, "barr00", &TreeBounds},
    { 3, "barr00", &TreeBounds},
    { 4, "barr00", &TreeBounds},
    { 5, "barr00", &TreeBounds},
    { 6, "barr00", &TreeBounds},
    { 7, "barr00", &TreeBounds},
    { 8, "barr00", &TreeBounds},
    { 9, "barr00", &TreeBounds},
    {10, "barr00", &TreeBounds},
};

// ------------------------------
// For the first time, I wish I had C++'s member pointers. Oh, well...
// A pointer to 'link' member, instead of that pointer to a 'link' member of
// an *instance* (a real address), would do wonders here.
// For now, l is one of t->block, t->global or t->sector.

PRIVATE AddLink(THN_PThing t, THN_PLink l, THN_PThing *first) {
    assert(t != NULL);
    assert(l != NULL);
    assert(first != NULL);
    l->prev = NULL;
    l->next = *first;
    if (l->next != NULL) {
        if (l == &t->block) {        // Which link are we working on?
            assert(l->next->block.prev == NULL);
            l->next->block.prev = t;
        } else if (l == &t->global) {
            assert(l->next->global.prev == NULL);
            l->next->global.prev = t;
        } else if (l == &t->sector) {
            assert(l->next->sector.prev == NULL);
            l->next->sector.prev = t;
        } else
            BASE_Abort("Invalid pointer to member in AddLink");
    }
    *first = t;
}

PRIVATE DelLink(THN_PThing t, THN_PLink l, THN_PThing *first) {
    if (l->prev == NULL) {
        assert(*first == t);
        *first = l->next;
    } else {
        if (l == &t->block)         // Which link are we working on?
            l->prev->block.next = l->next;
        else if (l == &t->global)
            l->prev->global.next = l->next;
        else if (l == &t->sector)
            l->prev->sector.next = l->next;
        else
            BASE_Abort("Invalid pointer to member in DelLink");
    }
    if (l->next != NULL) {
        if (l == &t->block)         // Which link are we working on?
            l->next->block.prev = l->prev;
        else if (l == &t->global)
            l->next->global.prev = l->prev;
        else if (l == &t->sector)
            l->next->sector.prev = l->prev;
        else
            BASE_Abort("Invalid pointer to member in DelLink");
    }
}

// ------------------------------

PRIVATE void THN_SparksFunc(THN_PSparks s) {
    THN_PThing thn;
    dword clock;

    if (Paused)
        return;

    assert(s != NULL);
    thn = s->thn;
    assert(thn != NULL);
    clock = GameClock - s->clock;

    THN_MoveThing(thn, thn->x + s->dx, thn->y + s->dy);
    s->dx -= s->dx / 16;
    s->dy -= s->dy / 16;

#define SPARKS_LIFE (50)
    if (clock < SPARKS_LIFE) {
        thn->spr     = Sparks[clock*SIZEARRAY(Sparks)/SPARKS_LIFE];
    } else
        THN_DeleteThing(thn);
#undef SPARKS_LIFE
}

PRIVATE bool THN_SparksNew(THN_PThing t, int type, word flags) {
    THN_PSparks s;

    s = NEW(sizeof(*s));
    if (s == NULL)
        return FALSE;
    s->thn   = t;
    s->clock = GameClock;
    s->dx    = 0;
    s->dy    = 0;
    t->data    = s;
    t->routine = THN_SparksFunc;
    t->flags   = 0;
    t->spr     = Sparks[0];
    t->bounds  = NULL;
//    t->nTable  = 0;
    return TRUE;
}

    // ---------

PRIVATE void THN_SmokeFunc(THN_PSmoke s) {
    THN_PThing thn;
    dword clock;

    if (Paused)
        return;

    assert(s != NULL);
    thn = s->thn;
    assert(thn != NULL);
    clock = GameClock - s->clock;

    thn->z += s->dz;
//    THN_MoveThing(thn, thn->x + s->dx + ((RND_GetNum()&0x7FFF) << 5), thn->y + s->dy + ((RND_GetNum()&0x7FFF) << 5));
    THN_MoveThing(thn, thn->x + s->dx, thn->y + s->dy);
    s->dx -= s->dx / 16;
    s->dy -= s->dy / 16;

#define SMOKE_LIFE (50)
    if (clock < SMOKE_LIFE) {
        thn->spr     = Smoke[6*(thn->type & 3) + clock*SIZEARRAY(Smoke)/3/SMOKE_LIFE];
    } else
        THN_DeleteThing(thn);
#undef SMOKE_LIFE
}

PRIVATE bool THN_SmokeNew(THN_PThing t, int type, word flags) {
    THN_PSmoke s;

    s = NEW(sizeof(*s));
    if (s == NULL)
        return FALSE;
    s->thn   = t;
    s->clock = GameClock;
    s->dx    = 0;
    s->dy    = 0;
    s->dz    = 0;
    t->data    = s;
    t->routine = THN_SmokeFunc;
    t->flags   = 0;
    t->spr     = Smoke[6*(t->type & 3)];
    t->bounds  = NULL;
//    t->nTable  = 0;
    return TRUE;
}

// ------------------------------

static int NThings = 0;

PUBLIC void THN_InitMap(THN_PThingMap map) {
    int i;

    assert(map != NULL);
    THN_ThingMap = map;
    for (i = 0; i < 64*64; i++)
        map[i] = NULL;
    NThings = 0;
}

PUBLIC THN_PThing THN_AddThing(uint32 x, uint32 y, uint32 z, int type, word angle, word flags) {
    THN_PThing g;
    int i;

    if (THN_ThingMap == NULL)
        return NULL;
    g = NEW(sizeof(*g));
    if (g == NULL)
        return NULL;
    i = (x >> 26) + 64*(y >> 26);

    AddLink(g, &g->block,  &THN_ThingMap[i]);
    AddLink(g, &g->global, &THN_ThingList);
    g->sec = SEC_FindSector(&Map.sec, NULL, SEC_TOSEC(x), SEC_TOSEC(y));
    REQUIRE(g->sec != NULL);
    AddLink(g, &g->sector, &g->sec->list);
    g->type  = type;
    g->magic = 0;
    g->x = x;
    g->y = y;
    g->z = z;
    g->angle = angle;
    g->tirerot = 0;
    g->tiredir = 0;
    NThings++;

    switch ((word)type & 0xFF00) {
        case THNT_PLAYER:
            if (!(UCT_New(g, (byte)type, flags)))
                BASE_Abort("Can't create player");
            DelLink(g, &g->global, &THN_ThingList);
            AddLink(g, &g->global, &THN_ThinkerList);
            break;
        case THNT_CAR:
            if (!(CAR_New(g, (byte)type, flags)))
                BASE_Abort("Can't create robot car");
            DelLink(g, &g->global, &THN_ThingList);
            AddLink(g, &g->global, &THN_ThinkerList);
            break;
        case THNT_SPARKS:
            if (!(THN_SparksNew(g, (byte)type, flags)))
                BASE_Abort("Can't create sparks");
            DelLink(g, &g->global, &THN_ThingList);
            AddLink(g, &g->global, &THN_ThinkerList);
            break;
        case THNT_SMOKE:
            if (!(THN_SmokeNew(g, (byte)type, flags)))
                BASE_Abort("Can't create grass");
            DelLink(g, &g->global, &THN_ThingList);
            AddLink(g, &g->global, &THN_ThinkerList);
            break;
        case THNT_RACEPOS:
        case THNT_HUMANPOS: {
            char buf[200];
            g->data    = NULL;
            g->routine = NULL;
            g->flags   = 0;
            if (((word)type & 0xFF00) == THNT_RACEPOS)
                sprintf(buf, "racepos%d", type & 0x7);
            else
                sprintf(buf, "humansp%d", (type & 0x7)+1);
            g->spr = FS3_New(buf);
            if (g->spr != NULL)
                g->spr->flags |= FLSF_NOSCALE;
            g->bounds  = NULL;
            break;
        }
        default:
            g->data    = NULL;
            g->routine = NULL;
            type = type - THNT_OBSTACLE;
            g->flags   = 0; //(type < (200 << 8))? THNF_SOLID : 0;
            {
                char buf[200];

                type = (((word)type & 0xFF00) >> 4) + (type & 0xF);
                sprintf(buf, "XPR%03X", type);
                g->spr = FS3_New(buf);
            }
            g->bounds  = NULL; //&TreeBounds;
//            g->nTable  = 0; //(type & 0xF0) >> 4;
    }

    return g;
}

PUBLIC void THN_DeleteThing(THN_PThing t) {
    int i;
    i = (t->x >> 26) + 64*(t->y >> 26);
    if (THN_ThingMap == NULL)
        return;
    assert(t != NULL);

    DelLink(t, &t->block,  &THN_ThingMap[i]);
    if (t->routine != NULL)
        DelLink(t, &t->global, &THN_ThinkerList);
    else
        DelLink(t, &t->global, &THN_ThingList);
    DelLink(t, &t->sector,  &t->sec->list);
    NThings--;

    DISPOSE(t->data);
    DISPOSE(t);
}

extern THN_PThing *RepeScasd(THN_PThing *src, int n);
#pragma aux RepeScasd modify nomemory [EAX] parm [EDI] [ECX] value [EDI] = \
    "XOR    EAX,EAX" \
    "REPE SCASD"     \
    "JNZ bye"        \
    "MOV EDI,4"      \
    "bye: SUB EDI,4"

PUBLIC void THN_RunThings(void) {
    THN_PThing p, g;

    MagicNumba++;
    p = THN_ThinkerList;
    while (p != NULL) {
        g = p->global.next;
        if (p->magic != MagicNumba) {
            p->magic = MagicNumba;
            if (p->routine != NULL)
                   p->routine(p->data);
                // Note: 'p' may have been deleted, so don't access to it.
        }
        p = g;
    }
}

PUBLIC void THN_MoveThing(THN_PThing t, uint32 nx, uint32 ny) {
    int i, ni;
    SEC_PSector sec;

    if (THN_ThingMap == NULL)
        return;
    i = (t->x >> 26) + 64*(t->y >> 26);
    ni = (nx >> 26) + 64*(ny >> 26);

    if (i != ni) {                  // Changed map block?
            // First remove from the old block.
        DelLink(t, &t->block,  &THN_ThingMap[i]);
            // Then add to new block.
        AddLink(t, &t->block,  &THN_ThingMap[ni]);
    }
    t->x = nx;
    t->y = ny;
    sec = SEC_FindSector(&Map.sec, t->sec, SEC_TOSEC(t->x), SEC_TOSEC(t->y));
    REQUIRE(sec != NULL);
    if (sec != t->sec) {
        DelLink(t, &t->sector, &t->sec->list);
        AddLink(t, &t->sector, &sec->list);
        t->sec = sec;
    }
}

PUBLIC bool THN_CalcCollide(THN_PThing t, int *s1, int *s2, THN_PThing *s) {
    int i;
    int mx, my, Mx, My;

    if (!(t->flags & THNF_SOLID))
        return FALSE;

/*
    if (t->sec->flags == 0) {
        if (s != NULL)
            *s = NULL;
        return TRUE;
    }
*/
        // Find bounding rectangle of thing in block units.
    mx = (t->x - t->bounds->radius) >> 26;
    my = (t->y - t->bounds->radius) >> 26;
    Mx = (t->x + t->bounds->radius) >> 26;
    My = (t->y + t->bounds->radius) >> 26;

        // Clip rectangle values to map dimensions.
    if (mx < 0) mx = 0;
    if (my < 0) my = 0;
    if (Mx >= 64) Mx = 63;
    if (My >= 64) My = 63;

        // Collide-check with all blocks the thing is in.
    while (my <= My) {
        for (i = mx; i <= Mx; i++) {
            THN_PThing p;
            p = THN_ThingMap[i+64*my];
            while (p != NULL) {
                if ((t->flags & THNF_SOLID) && t != p) {
                    if (THN_CollideThing(t, p, s1, s2)) {
                        if (s != NULL)
                            *s = p;
                        return TRUE;
                    }
                }
                p = p->block.next;
            }
        }
        my++;
    }
    return FALSE;
}

PUBLIC bool THN_CollideThing(THN_PThing t1, THN_PThing t2, int *s1, int *s2) {
    int i, j;
    uint32 dx, dy, r, d, r1, r2;
    uint32 x1, y1, x2, y2;
    sint32 dx1, dy1, dx1a, dy1a, dxdx1, dydy1,
           dx2, dy2, dx2a, dy2a, dxdx2, dydy2;
    sint32 A, B, R, R1, R2, rr;
    THN_PBoundsPoint p1,  p2;
    uint32           np1, np2;
    sint32           sina1, cosa1, sina2, cosa2;
    uint32           numcross, numcrossup, numcrossdn;

    if (t1->bounds == NULL || t2->bounds == NULL)
        return FALSE;

        // Initialize values.

    if (s1 != NULL)
        *s1 = -1;
    if (s2 != NULL)
        *s2 = -1;

    x1 = (uint32) t1->x >> 8;
    x2 = (uint32) t2->x >> 8;
    y1 = (uint32) t1->y >> 8;
    y2 = (uint32) t2->y >> 8;
    r  = (t1->bounds->radius + t2->bounds->radius) >> 8;
    dx = Abs32(x2 - x1);
    dy = Abs32(y2 - y1);

        // Test the bounding rectangle, to discard as many collides as
        // possible with the fastest calculations.

    if (dx >= r || dy >= r)
        return FALSE;

        // Test the bounding circle to discard more things quickly.

    r  = FP16Pow2(r);
    d  = FP16Pow2(dx) + FP16Pow2(dy);
    if (d >= r)
        return FALSE;

        // If it's 2 cylindrical objects, they collide.

    if (t1->bounds->nPoints == 0 && t2->bounds->nPoints == 0)
        return TRUE;

        // Their bounding circles collide, so it's pretty probable that the
        // next calculations end with a collision detection (therefore not
        // waste time with useless operations). Do them as detailed as
        // possible.

    if (t1->bounds->nPoints == 0) {
        THN_PThing t = t1;
        t1 = t2;
        t2 = t;

        // úú swap s1 and s2?

    }
    if (t2->bounds->nPoints == 0) {
        np1   = t1->bounds->nPoints;
        p1    = t1->bounds->points;
        sina1 = -Sin(t1->angle);
        cosa1 = Cos(t1->angle);
        p2    = NULL;
    } else {
        np1   = t1->bounds->nPoints;
        p1    = t1->bounds->points;
        np2   = t2->bounds->nPoints;
        p2    = t2->bounds->points;
        sina1 = -Sin(t1->angle);
        cosa1 = Cos(t1->angle);
        sina2 = -Sin(t2->angle);
        cosa2 = Cos(t2->angle);
    }

    if (p2 == NULL) {   // Shaped object against circular object.

        // First check if the center of the shaped object is inside the
        // circular object.

        r2 = FP16Pow2(t2->bounds->radius >> 8);
        if (r2 >= d)
            return TRUE;

        // Now, check if the edge of the shaped object collides with
        // the edge of the circular object.

        numcross = numcrossup = numcrossdn = 0;

        dx1a = x1 + FPMult(p1[np1-1].dx >> 8, cosa1) + FPMult(p1[np1-1].dy >> 8, sina1);
        dy1a = y1 - FPMult(p1[np1-1].dx >> 8, sina1) + FPMult(p1[np1-1].dy >> 8, cosa1);
        for (i = 0; i < np1; i++) {

            if (s1 != NULL) *s1 = i;

            // Check if the point is colliding.

            if (FP16Pow2(dx1a-x2) + FP16Pow2(dy1a-y2) < r2)
                return TRUE;

            // Get next point.

            dx1 = x1 + FPMult(p1[i].dx >> 8, cosa1) + FPMult(p1[i].dy >> 8, sina1);
            dy1 = y1 - FPMult(p1[i].dx >> 8, sina1) + FPMult(p1[i].dy >> 8, cosa1);

            // Get segment coords.

            dxdx1 = dx1-dx1a;
            dydy1 = dy1-dy1a;

            // 1st check:
            // Check if the segment crosses the center's vertical.
            // If so, check if it crosses above or below, so we can
            // later check if the center of the circular object was
            // inside the shaped object.

            if ( (dx1a > dx1) && ( (dx1a <= x2 && dx1 >  x2) || (dx1a >= x2 && dx1 <  x2) )
              || (dx1a < dx1) && ( (dx1a <  x2 && dx1 >= x2) || (dx1a >  x2 && dx1 <= x2) )) {

                if (dy1a > y2 && dy1 > y2) {
                    numcross++;
                    numcrossup++;
                } else if (dy1a < y2 && dy1 < y2) {
                    numcross++;
                    numcrossdn++;
                } else {
                    sint32 yp = dy1a + FPMultDiv((x2-dx1a), dydy1, dxdx1);

                    if (yp > y2) {
                        numcross++;
                        numcrossup++;
                    } else if (yp < y2) {
                        numcross++;
                        numcrossdn++;
                    } else
                        return TRUE;
                }                   
            } else if (dx1a == x2 && dx1 == x2 &&
                       ((dy1a >= y2 && dy1 <= y2) ||
                        (dy1a <= y2 && dy1 >= y2)) )
                return TRUE;

            // 2nd check:
            // Check if the segment's line collides with the circular object.

            A =   dydy1;
            B = - dxdx1;
            if (FP16Pow2((FP16Mult(A, x2 - dx1a) + FP16Mult(B, y2 - dy1a))) <
                FP16Mult(FP16Pow2(A) + FP16Pow2(B), r2)) {

                // Line collides. Check if it collides within the segment.

                if (Abs32(dydy1) > Abs32(dxdx1)) {

                    R = FP24Div(dxdx1, dydy1);
                    R = y2-dy1a + FP24Mult(R, x2-dx1a);
                    if (Sgn(R) == Sgn(dydy1) && Abs32(R) < Abs32(dydy1))
                        return TRUE;

                } else {

                    R = FP24Div(dydy1, dxdx1);
                    R = x2-dx1a + FP24Mult(R, y2-dy1a);
                    if (Sgn(R) == Sgn(dxdx1) && Abs32(R) < Abs32(dxdx1))
                        return TRUE;

                }
            }    


            // No colision yet. Go for the next.

            dx1a = dx1;
            dy1a = dy1;
        }
        
        // And now, check if the center of the circular object is inside
        // the shaped object.

        if ((numcrossup & 1) != 0 && (numcrossdn & 1) != 0)
            return TRUE;

    } else {   // 2 shaped objects.

        // First, check if the center of the second object is inside
        // the first object.

        numcross = numcrossup = numcrossdn = 0;

        dx1a = x1 + FPMult(p1[np1-1].dx >> 8, cosa1) + FPMult(p1[np1-1].dy >> 8, sina1);
        dy1a = y1 - FPMult(p1[np1-1].dx >> 8, sina1) + FPMult(p1[np1-1].dy >> 8, cosa1);
        for (i = 0; i < np1; i++) {

            // Get next point.

            dx1 = x1 + FPMult(p1[i].dx >> 8, cosa1) + FPMult(p1[i].dy >> 8, sina1);
            dy1 = y1 - FPMult(p1[i].dx >> 8, sina1) + FPMult(p1[i].dy >> 8, cosa1);
            
            // Get segment coords.

            dxdx1 = dx1-dx1a;
            dydy1 = dy1-dy1a;

            // 1st check:
            // Check if the segment crosses the center's vertical.
            // If so, check if it crosses above or below, so we can
            // later check if the center of the circular object was
            // inside the shaped object.

            if ( (dx1a > dx1) && ( (dx1a <= x2 && dx1 >  x2) || (dx1a >= x2 && dx1 <  x2) )
              || (dx1a < dx1) && ( (dx1a <  x2 && dx1 >= x2) || (dx1a >  x2 && dx1 <= x2) )) {

                if (dy1a > y2 && dy1 > y2) {
                    numcross++;
                    numcrossup++;
                } else if (dy1a < y2 && dy1 < y2) {
                    numcross++;
                    numcrossdn++;
                } else {
                    sint32 yp = dy1a + FPMultDiv((x2-dx1a), dydy1, dxdx1);

                    if (yp > y2) {
                        numcross++;
                        numcrossup++;
                    } else if (yp < y2) {
                        numcross++;
                        numcrossdn++;
                    } else
                        return TRUE;
                }                   
            } else if (dx1a == x2 && dx1 == x2 &&
                       ((dy1a >= y2 && dy1 <= y2) ||
                        (dy1a <= y2 && dy1 >= y2)) )
                return TRUE;

            // Go for the next.

            dx1a = dx1;
            dy1a = dy1;
        }

        if ((numcrossup & 1) != 0 && (numcrossdn & 1) != 0)
            return TRUE;

        // Now, check if the center of the first object is inside
        // the second object.

        numcross = numcrossup = numcrossdn = 0;

        dx2a = x2 + FPMult(p2[np2-1].dx >> 8, cosa2) + FPMult(p2[np2-1].dy >> 8, sina2);
        dy2a = y2 - FPMult(p2[np2-1].dx >> 8, sina2) + FPMult(p2[np2-1].dy >> 8, cosa2);
        for (i = 0; i < np2; i++) {

            // Get next point.

            dx2 = x2 + FPMult(p2[i].dx >> 8, cosa2) + FPMult(p2[i].dy >> 8, sina2);
            dy2 = y2 - FPMult(p2[i].dx >> 8, sina2) + FPMult(p2[i].dy >> 8, cosa2);
            
            // Get segment coords.

            dxdx2 = dx2-dx2a;
            dydy2 = dy2-dy2a;

            // Check if the segment crosses the center's vertical.
            // If so, check if it crosses above or below, so we can
            // later check if the center of the circular object was
            // inside the shaped object.

            if ( (dx2a > dx2) && ( (dx2a <= x1 && dx2 >  x1) || (dx2a >= x1 && dx2 <  x1) )
              || (dx2a < dx2) && ( (dx2a <  x1 && dx2 >= x1) || (dx2a >  x1 && dx2 <= x1) )) {

                if (dy2a > y1 && dy2 > y1) {
                    numcross++;
                    numcrossup++;
                } else if (dy2a < y1 && dy2 < y1) {
                    numcross++;
                    numcrossdn++;
                } else {
                    sint32 yp = dy2a + FPMultDiv((x1-dx2a), dydy2, dxdx2);

                    if (yp > y1) {
                        numcross++;
                        numcrossup++;
                    } else if (yp < y1) {
                        numcross++;
                        numcrossdn++;
                    } else
                        return TRUE;
                }                   
            } else if (dx2a == x1 && dx2 == x1 &&
                       ((dy2a >= y1 && dy2 <= y1) ||
                        (dy2a <= y1 && dy2 >= y1)) )
                return TRUE;

            // No colision yet. Go for the next.

            dx2a = dx2;
            dy2a = dy2;
        }

        if ((numcrossup & 1) != 0 && (numcrossdn & 1) != 0)
            return TRUE;

        // And now, check if the edges of both objects collide.

        dx1a = x1 + FPMult(p1[np1-1].dx >> 8, cosa1) + FPMult(p1[np1-1].dy >> 8, sina1);
        dy1a = y1 - FPMult(p1[np1-1].dx >> 8, sina1) + FPMult(p1[np1-1].dy >> 8, cosa1);
        for (i = 0; i < np1; i++) {

            if (s1 != NULL) *s1 = i;

            // Get next point.

            dx1 = x1 + FPMult(p1[i].dx >> 8, cosa1) + FPMult(p1[i].dy >> 8, sina1);
            dy1 = y1 - FPMult(p1[i].dx >> 8, sina1) + FPMult(p1[i].dy >> 8, cosa1);
            
            // Get segment coords.

            dxdx1 = dx1-dx1a;
            dydy1 = dy1-dy1a;
            
            if (Abs32(dxdx1) > Abs32(dydy1))
                R1 = FP24Div(dydy1, dxdx1);
            else
                R1 = FP24Div(dxdx1, dydy1);

            dx2a = x2 + FPMult(p2[np2-1].dx >> 8, cosa2) + FPMult(p2[np2-1].dy >> 8, sina2);
            dy2a = y2 - FPMult(p2[np2-1].dx >> 8, sina2) + FPMult(p2[np2-1].dy >> 8, cosa2);
            for (j = 0; j < np2; j++) {

                if (s2 != NULL) *s2 = j;

                // Get next point.

                dx2 = x2 + FPMult(p2[j].dx >> 8, cosa2) + FPMult(p2[j].dy >> 8, sina2);
                dy2 = y2 - FPMult(p2[j].dx >> 8, sina2) + FPMult(p2[j].dy >> 8, cosa2);

                // Get segment coords.

                dxdx2 = dx2-dx2a;
                dydy2 = dy2-dy2a;

                // Check if both segments collide.

                if (Abs32(dxdx2) > Abs32(dydy2)) {
                    R2 = FP24Div(dydy2, dxdx2);
                    if (Abs32(dxdx1) > Abs32(dydy1)) {
                        R = dy2a-dy1a - FP24Mult(R2, dx2a-dx1a);
                        rr = FP24Mult(dxdx1, R1-R2);
                        if (Sgn(R) == Sgn(rr) && Abs32(R) <= Abs32(rr)) {
                            R = dy2a-dy1a - FP24Mult(R1, dx2a-dx1a);
                            rr = FP24Mult(dxdx2, R1-R2);
                            if (Sgn(R) == Sgn(rr) && Abs32(R) <= Abs32(rr))
                                return TRUE;
                        }
                    } else {
                        R = dy1a-dy2a + FP24Mult(R2, dx2a-dx1a);
                        rr = FP24Mult(dydy1, FP24Mult(R1, R2) - (1<<24));
                        if (Sgn(R) == Sgn(rr) && Abs32(R) <= Abs32(rr)) {
                            R = dx1a-dx2a + FP24Mult(R1, dy2a-dy1a);
                            rr = FP24Mult(-dxdx2, FP24Mult(R1, R2) - (1<<24));
                            if (Sgn(R) == Sgn(rr) && Abs32(R) <= Abs32(rr))
                                return TRUE;
                        }
                    }
                } else {
                    R2 = FP24Div(dxdx2, dydy2);
                    if (Abs32(dxdx1) > Abs32(dydy1)) {
                        R = dx1a-dx2a + FP24Mult(R2, dy2a-dy1a);
                        rr = FP24Mult(dxdx1, FP24Mult(R1, R2) - (1<<24));
                        if (Sgn(R) == Sgn(rr) && Abs32(R) <= Abs32(rr)) {
                            R = dy1a-dy2a + FP24Mult(R1, dx2a-dx1a);
                            rr = FP24Mult(-dydy2, FP24Mult(R1, R2) - (1<<24));
                            if (Sgn(R) == Sgn(rr) && Abs32(R) <= Abs32(rr))
                                return TRUE;
                        }
                    } else {
                        R  = dx2a-dx1a - FP24Mult(R2, dy2a-dy1a);
                        rr = FP24Mult(dydy1, R1-R2);
                        if (Sgn(R) == Sgn(rr) && Abs32(R) <= Abs32(rr)) {
                            R  = dx2a-dx1a - FP24Mult(R1, dy2a-dy1a);
                            rr = FP24Mult(dydy2, R1-R2);
                            if (Sgn(R) == Sgn(rr) && Abs32(R) <= Abs32(rr))
                                return TRUE;
                        }
                    }
                }

                // No colision yet. Go for the next.

                dx2a = dx2;
                dy2a = dy2;
            }

            // No colision yet. Go for the next.

            dx1a = dx1;
            dy1a = dy1;
        }
    }

    return FALSE;
}

// ------------------------------ THINGS.C ----------------------------

