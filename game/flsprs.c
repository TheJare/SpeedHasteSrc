// ------------------------------ FLSPRS.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "flsprs.h"
#include <vertdraw.h>
#include <sincos.h>
#include <jclib.h>
#include <atan.h>
#include <rot3d.h>
#include <object3d.h>
#include <polygon.h>
#include <llscreen.h>
#include "globals.h"

#define PAL_LEVELS 16


PRIVATE int ProcessMesh(O3DM_PObject obj, sint32 tirecenter[4][3]) {
    int i, j;
    int tirev = 0;

        // Process object's scale factors.
    if (obj->scx == 0) obj->scx = 1 << 19;
    if (obj->scy == 0) obj->scy = 1 << 19;
    if (obj->scz == 0) obj->scz = 1 << 19;
    obj->scx >>= 1;
    obj->scy >>= 1;
    obj->scz >>= 1;
    for (j = 0; j < obj->nVerts; j++) {
        obj->verts[j].x = FPnMult(obj->verts[j].x - obj->dcx, obj->scx, 16);
        obj->verts[j].y = FPnMult(obj->verts[j].y - obj->dcy, obj->scy, 16);
        obj->verts[j].z = FPnMult(obj->verts[j].z - obj->dcz, obj->scz, 16);
    }
        // Find tires.
    if (obj->flags & FLOF_HASTIRES) {
        int nv, cx, cy, cz;
        if (obj->nVerts == 345)
            tirev = 28;
        else if (obj->nVerts == 182)
            tirev = 20;
        else if (obj->nVerts == 240)
            tirev = 28;
        nv = 0;
        if (tirev > 0) for (i = 0; i < 4; i++) {
            cx = cy = cz = 0;
            for (j = 0; j < tirev; j++) {
                cx += obj->verts[nv].x;
                cy += obj->verts[nv].y;
                cz += obj->verts[nv].z;
                nv++;
            }
            tirecenter[i][0] = cx/tirev;
            tirecenter[i][1] = cy/tirev;
            tirecenter[i][2] = cz/tirev;
        }
    }
    return tirev;
}


PUBLIC bool FS3_Load(FS3_PSprite s, const char *fname) {
    int i, j, totsp;
    FILE *f, *fs;
    char buf[200];
    FS2_PSprite psp;
    O3DM_PObject pob;

    assert(s != NULL);

    s->flags = 0;

        // Try to load a 3D object.
        // First, check if an object without trailing letters exists.
    sprintf(buf, "%s.I3D", fname);
    if ((s->obj = O3DM_LoadObject(buf)) != NULL) {
        s->flags = FLSF_VECTOR;
        s->nSprs = 1;
        s->tirev = ProcessMesh(s->obj, s->tirecenter);
        return TRUE;
    }
        // If not, check if an object with trailing letter.
    i = 0;
    for (;;) {  // exit condition is inside loop.
        sprintf(buf, "%s%c.I3D", fname, i+'A');
        if ((fs = JCLIB_Open(buf)) == NULL)
            break;
        JCLIB_Close(fs);
        i++;
    }
    if (i > 0) {
        s->flags = FLSF_VECTOR;
        s->nSprs = i;
        s->objs = NEW(sizeof(*s->objs)*i);
        if (s->objs == NULL)
            return FALSE;
        for (i = 0; i < s->nSprs; i++) {
            sprintf(buf, "%s%c.I3D", fname, i+'A');
            REQUIRE( (s->objs[i].obj = O3DM_LoadObject(buf)) != NULL);
            s->objs[i].tirev = ProcessMesh(s->objs[i].obj, s->objs[i].tirecenter);
        }
        return TRUE;
    }

        // So, it must be a bitmapped IS2 sprite.

        // First, check if a sprite without trailing letters exists.
    sprintf(buf, "%s.IS2", fname);
    fs = JCLIB_Open(buf);
    if (fs != NULL) {
        JCLIB_Close(fs);
        s->nSprs = 1;
        s->sprs = NEW(sizeof(*s->sprs));
        if (s->sprs == NULL)
            return FALSE;
        s->sprs[0].nSprs = 1;
        s->sprs[0].sprs = NEW(sizeof(*s->sprs[0].sprs));
        if (s->sprs[0].sprs == NULL) {
            DISPOSE(s->sprs);
            return FALSE;
        }
        s->sprs[0].sprs[0].sp = IS2_Load(buf);
        if (s->sprs[0].sprs[0].sp == NULL) {
            DISPOSE(s->sprs[0].sprs);
            DISPOSE(s->sprs);
            return FALSE;
        }
            // Aspect ratio and such stuff goes here.
        s->sprs[0].sprs[0].w = FPMultDiv(s->sprs[0].sprs[0].sp->xRatio,
                                         s->sprs[0].sprs[0].sp->w*0x2000,
                                         55 << 8);
        s->sprs[0].sprs[0].h = FPMultDiv(s->sprs[0].sprs[0].sp->yRatio,
                                         s->sprs[0].sprs[0].sp->h*0x2000,
                                         66 << 8);
        return TRUE;
    }

        // Not, so let's see what sprite frames can we find.

    i = 0;
    for (;;) {  // exit condition is inside loop.
        sprintf(buf, "%.6s%cA.IS2", fname, i+'A');
        if ((fs = JCLIB_Open(buf)) == NULL)
            break;
        JCLIB_Close(fs);
        i++;
    }
    if (i <= 0)
        BASE_Abort("Trying to read object %s\n", fname);
//        return FALSE;
    s->nSprs = i;
    s->sprs = NEW(sizeof(*s->sprs)*s->nSprs);
    if (s->sprs == NULL)
        return FALSE;

    totsp = 0;
    for (i = 0; i < s->nSprs; i++) {
        j = 1;      // 0 was already tested, so save some time.
        for (;;) {
            sprintf(buf, "%.6s%c%c.IS2", fname, i+'A', j+'A');
            if ((fs = JCLIB_Open(buf)) == NULL)
                break;
            JCLIB_Close(fs);
            j++;
        }
        assert(j > 0);
        s->sprs[i].nSprs = j;
        totsp += j;
    }
    psp = NEW(sizeof(*s->sprs->sprs)*totsp);

    for (i = 0; i < s->nSprs; i++) {
        s->sprs[i].sprs = psp;
        psp += s->sprs[i].nSprs;
        for (j = 0; j < s->sprs[i].nSprs; j++) {
            sprintf(buf, "%.6s%c%c.IS2", fname, i+'A', j+'A');
            s->sprs[i].sprs[j].sp = IS2_Load(buf);
            REQUIRE(s->sprs[i].sprs[j].sp != NULL);
                // Aspect ratio and such stuff goes here.
            s->sprs[i].sprs[j].w = FPMultDiv(s->sprs[i].sprs[j].sp->xRatio,
                                             s->sprs[i].sprs[j].sp->w*0x2000,
                                             55 << 8);
            s->sprs[i].sprs[j].h = FPMultDiv(s->sprs[i].sprs[j].sp->yRatio,
                                             s->sprs[i].sprs[j].sp->h*0x2000,
                                             66 << 8);
        }
    }
    return TRUE;
}

PUBLIC void FS3_Delete(FS3_PSprite s) {
    assert(s != NULL);
    if (s->flags & FLSF_VECTOR) {
        if (s->nSprs > 1) {
            int i;
            for (i = 0; i < s->nSprs; i++)
                O3DM_DeleteObject(s->objs[i].obj);
            DISPOSE(s->objs);
        } else
            O3DM_DeleteObject(s->obj);
    } else {
        int i, j;
        for (i = 0; i < s->nSprs; i++)
            for (j = 0; j < s->sprs[i].nSprs; j++)
                DISPOSE(s->sprs[i].sprs[j].sp);
        DISPOSE(s->sprs[0].sprs);
        DISPOSE(s->sprs);
    }
    DISPOSE(s);
}

PUBLIC FS3_PSprite FS3_New(const char *fname) {
    NL_PName n;

    n = NL_AddName(&GL_NameTree, fname);
    REQUIRE(n != NULL);
    if (n->data == NULL) {
//        printf("Reading sprite \"%s\".\n", fname);
        n->data = NEW(sizeof(FS3_TSprite));
        REQUIRE(n->data != NULL);
        REQUIRE(FS3_Load((FS3_PSprite)n->data, fname));
    }
    return n->data;
}

PRIVATE bool DelSprite(NL_PName *n, void *data) {
    FS3_Delete((*n)->data);
    DISPOSE(*n);            // Eat the tree from the bottom up.
    return TRUE;
}

PUBLIC void FS3_End(void) {
    NL_WalkTree(&GL_NameTree, &DelSprite, NULL);
}

// =================================================

typedef struct FSP_SObject {
    sint32 x, y, z, depth;
    FS3_PSprite sp3;
    union {
        struct {
            sint16 w, h;
            word t;
            IS2_PSprite sp;
        };
        struct {
//            uint32 x, y, z;
            word angle;
            word tirerot, tiredir;  // Rotation and direction of tires
        };
    };
    struct FSP_SObject *next;
} FSP_TObject, *FSP_PObject;

#define MAX_OBJS 140

PRIVATE FSP_TObject Objs[MAX_OBJS];
PRIVATE int nObjsUsed = 0;
PRIVATE FSP_PObject first = NULL;

PRIVATE bool CalcThem = TRUE;

PRIVATE F3D_PCamera UseCam;
PRIVATE int UseScrX, UseScrY;

int FL_StatNPolys, FL_StatNSprs, FL_StatNObjs, FL_StatNFrontObjs, FL_StatNVecs;
int FL_StatNZHash;

PUBLIC void FSP_ClearObjs(int scrx, int scry, F3D_PCamera cam) {
    first = NULL;
    nObjsUsed = 0;
    CalcThem = TRUE; //(BASE_CheckArg("nocalc") <= 0);
    UseCam   = cam;
    UseScrX  = scrx;
    UseScrY  = scry;
/*
        FL_StatNPolys = 0;
        FL_StatNSprs = 0;
        FL_StatNObjs = 0;
        FL_StatNFrontObjs = 0;
        FL_StatNVecs = 0;
*/
}

PUBLIC void FSP_AddObj(sint32 x, sint32 y, sint32 z, word angle, int ntable,
                       FS3_PSprite sp, word tirerot, word tiredir) {
    word t;
    sint32 rx, ry, w, h;
    word a;
    FSP_PObject *p, g;

#define cam UseCam
#define scrx UseScrX
#define scry UseScrY

    if (!CalcThem)
        return;

    x = (sint32)((uint32)x >> 6) - (sint32)((uint32)cam->x >> 6);
    y = (sint32)((uint32)y >> 6) - (sint32)((uint32)cam->y >> 6);

    a = cam->angle;
    ry = FPMult(x, Cos(a)) + FPMult(y, Sin(a));
    if (ry < (4 << 14))
        return;

    if ((sp->flags & FLSF_VECTOR) && ry >= (20 << 20))
        return;

    rx = FPMult(y, Cos(a)) - FPMult(x, Sin(a));
    if (Abs32(rx/3) > ry)
        return;

    FL_StatNFrontObjs++;

    if (!(sp->flags & FLSF_VECTOR)) {
/*
        if (ry < (1 << 20))
            t = PAL_LEVELS;
        else if (ry >= (10 << 20))
            t = 16+PAL_LEVELS-1;
        else
            t = 16+(((PAL_LEVELS-2)*(ry-(1 << 20))/(10-1)) >> 20);
*/
        t = PAL_LEVELS;
        y =  scry + FPMultDiv((cam->data.h << 2) - (z >> 10), cam->data.focus, ry)
                  - cam->data.horizon;
        if (z < 0x100000 && y < scry)
            return;
        x = scrx + FPMultDiv(rx >> 4, cam->data.focus, ry);
    }
        // Insertion in Z-ordered list.
    p = &first;

    while (*p != NULL) {
        if ((*p)->depth <= ry)
            break;
        p = &((*p)->next);
    }

    g = *p;
    if (nObjsUsed >= MAX_OBJS) {  // Get rid of the farthest object.
        if (g == first)         // Unless the new object is even farther.
            return;
        *p = first;
        first = first->next;
    } else {
        *p = Objs + nObjsUsed;
        nObjsUsed++;
    }
    (*p)->next = g;

    g = *p;

        // Calc viewing angle.
    if (sp->flags & FLSF_VECTOR) {
        g->angle = angle;
        g->x = rx;
        g->y = ry;
        g->z = z;
        g->tirerot = tirerot;
        g->tiredir = tiredir;
    } else {
        int nh;
        int nv;
        word a;
        static struct {
            signed char nh, w;
        } rot9[16] = {
            {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1}, {6,1}, {7,1},
            {8,-1}, {7,-1}, {6,-1}, {5,-1}, {4,-1}, {3,-1}, {2,-1}, {1,-1},
        }, rot17[32] = {
            {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1}, {6,1}, {7,1},
            {8,1}, {9,1}, {10,1}, {11,1}, {12,1}, {13,1}, {14,1}, {15,1},
            {16,-1}, {15,-1}, {14,-1}, {13,-1}, {12,-1}, {11,-1}, {10,-1}, {9,-1},
            {8,-1}, {7,-1}, {6,-1}, {5,-1}, {4,-1}, {3,-1}, {2,-1}, {1,-1},
        };
        w = 1;
        if (sp->nSprs == 9) {
            a = - GetAngle(rx >> 4, ry >> 4)/2 +  angle - cam->angle + 65536/16/2 + 16384/2;
            a = a >> 12;
            nh = (int)rot9[a].nh;
            w =  (sint32)rot9[a].w;
        } else if (sp->nSprs == 17) {
//            a = /*- GetAngle(rx >> 4, ry >> 4)/2 + */ angle - cam->angle + 65536/32/2 + 16384/2;
            a = - GetAngle(rx >> 4, ry >> 4)/2 + angle - cam->angle + 65536/32/2 + 16384/2;
            a = a >> 11;
            nh = (int)rot17[a].nh;
            w =  (sint32)rot17[a].w;
        } else
            nh = 0;
        if (sp->sprs[nh].nSprs == 5) {
//            if (ry < (1 << 20))
            if ((cam->data.h << 2) - (z >> 10) < (1 << 13))
                a = 0;
            else
                a = GetAngle(ry >> 4, (cam->data.h << 2) - (z >> 10));
            if (a < (65536L*20/360))
                nv = 0;
            else if (a < (65536L*25/360))
                nv = 1;
            else if (a < (65536L*30/360))
                nv = 2;
            else if (a < (65536L*35/360))
                nv = 3;
            else
                nv = 4;
        } else
            nv = 0;
        g->sp = sp->sprs[nh].sprs[nv].sp;
        if (sp->flags & FLSF_NOSCALE) {
            w = w*(sint32)sp->sprs[nh].sprs[nv].sp->w;
            h =   sp->sprs[nh].sprs[nv].sp->h;
            t = PAL_LEVELS;
        } else {
            w = w*FPMultDiv((sint32)sp->sprs[nh].sprs[nv].w, cam->data.focus, ry);
            h =   FPMultDiv(sp->sprs[nh].sprs[nv].h, cam->data.focus, ry);
        }

        g->t = t;
        g->w = w;
        g->h = h;
        g->x = x;
        g->y = y;
    }
    g->depth = ry;
    g->sp3 = sp;

    FL_StatNObjs++;
}

#undef cam
#undef scrx
#undef scry

#define LIGHT_LEVELS 31

PRIVATE void O3DMCalcLights(O3DM_PObject obj, dword ang) {
    int i, n1 = 0;
    dword l;
    O3DM_PVertex pv;
    assert(obj != NULL);

    l = 0;
    pv = obj->verts;
    if (GL_CarType == 0 && (obj->flags & FLOF_HASTIRES))
        n1 = 14*2*4;
    for (i = 0; i < n1; i++, pv++, l += (5 << 10))
        pv->l = (16 << 16) + FPMult(13 << 15, Sin(ang + l));
    for (; i < obj->nVerts; i++, pv++, l += (5 << 10))
        pv->l = (16 << 16) + FPMult(15 << 16, Sin(ang + l));
}


PRIVATE void O3DMDraw(F3D_PCamera c, O3DM_PObject obj, int scrx, int scry) {
    int i, j, nv;
    int ladd = *obj->rot;
    O3DM_PFace pl;

    for (pl = O3DM_OrderFaces(obj); pl != NULL; pl = pl->h.next) {
        sint32 dl;
        O3DM_PFaceVertex p;
        POLY_PFullVertex pv;
        O3DM_PFaceVertex lastv = NULL;

        FL_StatNPolys++;

        pv = POLY_ScrapPoly;                nv = 0;
        p = pl->verts + pl->h.nVerts - 1;   j = pl->h.nVerts-1;
        lastv = pl->verts;      // For wraparound correctly.
        while (j >= 0) {
#define PLANE 4000
            if (p->vert->rz < PLANE) {
                if (lastv->vert->rz > PLANE) {  // if == PLANE, new vertex would overlap lastv.
                    // We're behind, coming from being in front.
                    // So cut the edge.
                    sint32 d1, d2, x, y, l, tx, ty;
                    d1 = lastv->vert->rz - p->vert->rz;
                    d2 = PLANE - p->vert->rz;
                    x  = p->vert->rx + FPMultDiv(lastv->vert->rx - p->vert->rx, d2, d1);
                    y  = p->vert->ry + FPMultDiv(lastv->vert->ry - p->vert->ry, d2, d1);
                    tx = p->tx + FPMultDiv(lastv->tx - p->tx, d2, d1);
                    ty = p->ty + FPMultDiv(lastv->ty - p->ty, d2, d1);
                    x = scrx + FPMultDiv(x, c->data.focus, PLANE >> 2);
                    y = scry + FPMultDiv(y, c->data.focus, PLANE >> 2);
                    pv->x  = x;
                    pv->y  = y;
                    pv->tx = tx;
                    pv->ty = ty;
                    pv->l  = p->vert->l;
                    nv++; pv++;
                    lastv = p; p--; j--;
                }
                while (j >= 0 && p->vert->rz < PLANE) {
                    lastv = p; p--; j--;
                }
                if (j < 0)
                    p = pl->verts + pl->h.nVerts - 1;

                if (p->vert->rz > PLANE) {   // if == PLANE, new vertex would overlap lastv.
                    // We're in front, coming from behind
                    // So cut the edge.
                    sint32 d1, d2, x, y, l, tx, ty;

                    assert(lastv->vert->rz < PLANE);
                    d1 = p->vert->rz - lastv->vert->rz;
                    d2 = PLANE - lastv->vert->rz;
                    x  = lastv->vert->rx + FPMultDiv(p->vert->rx - lastv->vert->rx, d2, d1);
                    y  = lastv->vert->ry + FPMultDiv(p->vert->ry - lastv->vert->ry, d2, d1);
                    tx = lastv->tx + FPMultDiv(p->tx - lastv->tx, d2, d1);
                    ty = lastv->ty + FPMultDiv(p->ty - lastv->ty, d2, d1);
                    x = scrx + FPMultDiv(x, c->data.focus, PLANE >> 2);
                    y = scry + FPMultDiv(y, c->data.focus, PLANE >> 2);
                    pv->x  = x;
                    pv->y  = y;
                    pv->tx = tx;
                    pv->ty = ty;
                    pv->l  = p->vert->l;
                    nv++; pv++;
                }
                if (j < 0)
                    break;
            }
            pv->x  = p->vert->px;
            pv->y  = p->vert->py;
            pv->tx = p->tx;
            pv->ty = p->ty;
            pv->l  = p->vert->l;
            nv++; pv++;
            lastv = p; p--; j--;
        }
        if (nv < 3)
            continue;
/*
        if (pl->h.material->flags & O3DMF_TRANS) {
            POLY_ScrapPoly[0].l = (pl->h.material->ambient & LIGHT_LEVELS) << 16;
            POLY_TransDraw(POLY_ScrapPoly, nv);
        } else if (pl->h.material->texture == NULL || O3DM_MaxDetail < O3DD_TEXTURED) {
            if ((pl->h.material->flags & O3DMF_NOSHADE)
             || (pl->h.flags & O3DFF_FLAT)
             || O3DM_MaxDetail < O3DD_GOURAUD) {
                if (pl->h.material->flags & O3DMF_NOSHADE)
                    POLY_ScrapPoly[0].l = LIGHT_LEVELS << 16;
                POLY_SolidDraw(POLY_ScrapPoly, nv, pl->h.material->color);
            } else
                POLY_ShadeDraw(POLY_ScrapPoly, nv, pl->h.material->color);
//                POLY_GouraudDraw(POLY_ScrapPoly, nv, pl->h.material->color);
        } else {
            if ((pl->h.material->flags & O3DMF_NOSHADE)
             || O3DM_MaxDetail < O3DD_TEXLIGHT)
                if (pl->h.material->flags & O3DMF_HOLES)
                    POLY_HoleTexDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
                else
                    POLY_TextureDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
            else if ((pl->h.flags & O3DFF_FLAT)
                   || O3DM_MaxDetail < O3DD_TEXGOURAUD) {
                POLY_LightTexDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
            } else
                POLY_ShadeTexDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
        }
*/
        if (pl->h.material->flags & O3DMF_TRANS) {
            POLY_ScrapPoly[0].l = (pl->h.material->ambient & LIGHT_LEVELS) << 16;
            POLY_TransDraw(POLY_ScrapPoly, nv);
        } else if (pl->h.material->texture == NULL) {
            if (!PolygonDetail) {
                POLY_ScrapPoly[0].l = (LIGHT_LEVELS/2) << 16;
                POLY_SolidDraw(POLY_ScrapPoly, nv, pl->h.material->color);
            } else
                POLY_ShadeDraw(POLY_ScrapPoly, nv, pl->h.material->color);
        } else {
            if (!PolygonDetail || (pl->h.flags & O3DFF_FLAT) || (pl->h.material->flags & O3DMF_NOSHADE)) {
                if (pl->h.material->flags & O3DMF_HOLES)
                    POLY_HoleTexDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
                else
                    POLY_TextureDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
            } else
                POLY_ShadeTexDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
        }
    }
}

PRIVATE void Draw3D(F3D_PCamera c, FSP_PObject sp, int scrx, int scry) {
    int i, j;
    O3DM_PObject obj;
    O3DM_PVertex v;
    sint32 x, y, z;
    word a;
    sint32 spx, spz, ca, sa, minx, maxx;
    sint32 ctirer, stirer, ctired, stired;
    int tirev;
    R3D_TAngleValue rot;
    sint32 (*tirecenter)[3];
    bool pdet;

    pdet = PolygonDetail;

    a = 0x8000 - c->angle + sp->angle;
    rot = sp->angle;

        // Object's center position is in sp->x, y, z.
        // It is relative to the camera, and >> 6.

        // Optimization variables.
    spx = sp->x >> 4;
    spz = (c->data.h << 2) - (sp->z >> 10);
    scry -= c->data.horizon;
    ca = Cos(a);
    sa = Sin(a);

    ctirer = Cos(sp->tirerot);
    stirer = Sin(sp->tirerot);
    ctired = Cos(sp->tiredir);
    stired = Sin(sp->tiredir);

    if (sp->sp3->nSprs == 1) {
        obj = sp->sp3->obj;
        tirev = sp->sp3->tirev;
        tirecenter = sp->sp3->tirecenter;
    } else {
        int k;
        k = 0;//FPMultDiv((sp->y),sp->sp3->nSprs,(12 << 20));
        if (ExtraDetail == 0 && sp->y > ((2+BackgroundDetail/2) << 20))
            k = 1;
        if (k > 0)
            PolygonDetail = FALSE;
        obj = sp->sp3->objs[k].obj;
        tirev = sp->sp3->objs[k].tirev;
        tirecenter = sp->sp3->objs[k].tirecenter;
    }
//    obj->rot = &rot;

    minx =  0x7FFFFFFF;
    maxx = -0x7FFFFFFF;
    v = obj->verts;
    for (i = 0; i < obj->nVerts; i++) {
            // OK, sprite coords are X,Y in the plane and Z vertical.
            // Vector coords are X,Z plane and Y vertical.
        sint32 dz;

        x = v->x;
        y = v->y;
        z = v->z;

        if ((obj->flags & FLOF_HASTIRES) && i < tirev*4) {
            int ntire = i/tirev;
            sint32 rx, ry, rz;

            ry = y - tirecenter[ntire][1];
            rz = z - tirecenter[ntire][2];
            y = tirecenter[ntire][1] + FPMult(ry, ctirer) - FPMult(rz, stirer);
            z = tirecenter[ntire][2] + FPMult(rz, ctirer) + FPMult(ry, stirer);

            if (ntire < 2) {
                rx = x - tirecenter[ntire][0];
                rz = z - tirecenter[ntire][2];
                x = tirecenter[ntire][0] + FPMult(rx, ctired) - FPMult(rz, stired);
                z = tirecenter[ntire][2] + FPMult(rz, ctired) + FPMult(rx, stired);
            }
        }

        dz = (sp->y >> 4) + FPMult(z, ca) - FPMult(x, sa);
        v->rz = dz;
        if (dz < (90 << 4))
            dz = (90 << 4);
        v->rx = (spx + FPMult(x, ca) + FPMult(z, sa)) >> (4+2);
        v->ry = (spz - y) >> (4+2);
        v->px = scrx + FPMultDiv(v->rx, c->data.focus, dz >> 2);
        if (minx > v->px) minx = v->px;
        if (maxx < v->px) maxx = v->px;
        v->py = scry + FPMultDiv(v->ry, c->data.focus, dz >> 2);

            // So that ordering goes OK.
        v++;
    }
        // If no vertices in the object are visible, don't care.
    if (minx >= POLY_MaxX || maxx <= POLY_MinX) {
        PolygonDetail = pdet;
        return;
    }

    O3DM_ParseVisibility(obj);
    O3DM_OrderByZ = (sp->y < (1 << (32-6 - 4 + ExtraDetail)));
    if (PolygonDetail)
        O3DMCalcLights(obj, rot);
    O3DMDraw(c, obj, scrx, scry);
    PolygonDetail = pdet;
}

PUBLIC void FSP_DumpObjs(F3D_PCamera cam, const byte (*trans)[256], int scrx, int scry) {
    FSP_PObject p;

    p = first;
    DRW_Tile = FALSE;
    while (p != NULL) {
        if (!(p->sp3->flags & FLSF_VECTOR)) {
            DRW_TranslatePtr = trans[p->t];
//            if (BASE_CheckArg("nosprs") <= 0)
                IS2_Draw(p->sp, p->x, p->y, (p->w & ~1), p->h & ~1);
            FL_StatNSprs++;
        } else {
            // Vector rendering pipeline.
//            if (BASE_CheckArg("novectors") <= 0)
                Draw3D(cam, p, scrx, scry);
            FL_StatNVecs++;
        }
        p = p->next;
    }
}
