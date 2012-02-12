// ------------------------------ USERCTL.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "userctl.h"
#include "globals.h"

#include <llscreen.h>
#include <llkey.h>
#include <sincos.h>
#include <atan.h>
#include <lbm.h>
#include <sqrt.h>
#include <string.h>

#include <object3d.h>

#include "race.h"

#include "models.h"

PRIVATE THN_TBounds PlayerBounds = {
//    1 << 24,
//    0
    0xB00000,
    4, {
        {  0xB00000, 0x380000 },
        {  0xB00000, -0x380000 },
        { -0x800000, -0x380000 },
        { -0x800000, 0x380000 },
    }
};

PUBLIC bool UCT_New(THN_PThing t, int type, word flags) {
    PLY_PPlayer p;
    static char carname[] = "carrn0";

    p = NEW(sizeof(*p));
    if (p == NULL)
        return FALSE;
    t->data    = p;
    t->routine = &UCT_DoControl;
    t->flags   = THNF_SOLID;
    carname[3] = '0' + GL_CarType;
    carname[5] = '0' + (((byte)type) >> 4);       // Up to 16 different cars.
    t->spr     = FS3_New(carname);
    t->bounds  = &PlayerBounds;
/*
    t->nTable  = 0; //type & 0xF;
    if (t->nTable >= 11)
        BASE_Abort("Unkown color %d", t->nTable);
*/
    p->thn   = t;
    p->flags = flags ;// | THNF_VECTOR;
    p->movangle = t->angle;
    p->ma    = t->angle;
    p->va    = 0;
    p->v     = 0;
    p->gear  = 1;
    p->revo  = 0;
    p->tires = 46;       // 16 is REALLY hard to control.
    p->reverse  = 0;
    p->slidcounter = 0;
    p->status = PLYST_RACING;
    REQUIRE(RCS_NRacers < RCS_MaxRacers);
    p->racer = RCS_Racers + RCS_NRacers;
    p->racer->thn = p->thn;
    if (GL_TimedRace)
        p->racer->laptimeleft = GL_RaceTime[0];
    else
        p->racer->laptimeleft = 0;
    RCS_NRacers++;

/*
    if (GL_CarType == 1) PlayerBounds.radius = 3 << 21;
    else                 PlayerBounds.radius = 3 << 21;
*/
    return TRUE;
}

PUBLIC void UCT_GetUserControl(UCT_PUserControl ctl, dword clock, UCT_PInputConfig cfg) {
    ctl->scancode = LLK_LastScan;
    ctl->control = 0;

        // Always check the keyboard.
    if (LLK_Keys[cfg->kup])
        ctl->control |= UCTL_UP;
    if (LLK_Keys[cfg->kdn])
        ctl->control |= UCTL_DOWN;
    if (LLK_Keys[cfg->kleft])
        ctl->control |= UCTL_LEFT;
    if (LLK_Keys[cfg->kright])
        ctl->control |= UCTL_RIGHT;
    if (LLK_Keys[cfg->kgear])
        ctl->control |= UCTL_GEAR;
    if (cfg->source == UCTS_JOYA) {
/*
        if (GL_JoyAY < -8) ctl->control |= UCTL_UP;
        if (GL_JoyAY >  8) ctl->control |= UCTL_DOWN;
        if (GL_JoyAX < -8) ctl->control |= UCTL_LEFT;
        if (GL_JoyAX >  8) ctl->control |= UCTL_RIGHT;
*/
        ctl->analogX = GL_JoyAX;
        ctl->analogY = -GL_JoyAY;
        if (GL_JoyABut & 1) ctl->control |= UCTL_GEARUP;
        if (GL_JoyABut & 2) ctl->control |= UCTL_GEARDN;
            //ctl->scancode = cfg->kgear;
    } else if (cfg->source == UCTS_JOYB) {
/*
        if (GL_JoyBY < -8) ctl->control |= UCTL_UP;
        if (GL_JoyBY >  8) ctl->control |= UCTL_DOWN;
        if (GL_JoyBX < -8) ctl->control |= UCTL_LEFT;
        if (GL_JoyBX >  8) ctl->control |= UCTL_RIGHT;
*/
        ctl->analogX = GL_JoyBX;
        ctl->analogY = -GL_JoyBY;
        if (GL_JoyBBut & 1) ctl->control |= UCTL_GEARUP;
        if (GL_JoyBBut & 2) ctl->control |= UCTL_GEARDN;
    } else {
        ctl->analogX = 0;
        ctl->analogY = 0;
    }
}

PRIVATE struct {
    sint32 ratio;       // 16.16 speed/revo ratio.
    sint32 accel;
    sint32 minrevo, maxrevo;
} GearInfo[] = {
     {-0x4000, 0x8000, 0, 1 << 28},
     {0x0000, 0x20000, 0, 1 << 15},
     {0x2000, 0x10000, 0,       0x0D << 18},
     {0x4000, 0x07000, 4 << 18, 0x0D << 18},
     {0x6000, 0x04000, 5 << 18, 0x0E << 18},
     {0x8000, 0x02800, 5 << 18, 0x0D << 18},
     {0xC000, 0x01C00, 5 << 18, 0x0D << 18},
    {0x10000, 0x01000, 6 << 18, 0x0D << 18},
};

#define MAXGEAR (SIZEARRAY(GearInfo))

PRIVATE void GearChange(PLY_PPlayer p) {
    THN_PThing t;
    uint32 x, y;

    if (RND_GetNum() % 2)
        return;
    x = p->thn->x - FPMult(1 << 23, Cos(p->thn->angle));
    y = p->thn->y - FPMult(1 << 23, Sin(p->thn->angle));
    if (p->cartype == 0) {
        t = THN_AddThing(x, y, 0, THNT_SPARKS, 0, 0);
        if (t != NULL) {
            ((THN_PSparks)t->data)->dx = FPMult(p->v, Cos(p->thn->angle))/4;
            ((THN_PSparks)t->data)->dy = FPMult(p->v, Sin(p->thn->angle))/4;
        }
    }
}


extern int Crack = 0, WallAngle = 0;
extern int LastConsoleCtl = 0, LastOtherCtl = 0;
extern int ConsoleUp = 0, OtherUp = 0;


PUBLIC void UCT_DoControl(PLY_PPlayer p) {
    UCT_PUserControl ctl;
    sint32 a, maxva, maxa;
    sint32 dx, dy;
    sint32 oldx, oldy;
    UCT_PInputConfig cfg;
    SEC_PSector sec;
    int braking;
    sint32 dv;

    if (Paused)
        return;
    if (RaceStarted && p->status == PLYST_RACING) {
        int nl = p->racer->nlap;
        if (nl >= SIZEARRAY(p->racer->laptime))
            nl = SIZEARRAY(p->racer->laptime)-1;
        p->racer->totaltime++;
        p->racer->laptime[nl]++;
        if (p->racer->laptimeleft > 0) {
            p->racer->laptimeleft--;
            if (p->racer->laptimeleft == 0) {
                if (p->flags & PLYF_ISCONSOLE)
                    GAME_EFF_Start(SH_Vtal, 4, 256, 400, 0, RACE_EffTimeout);
                p->status = PLYST_TIMEOUT;
                p->racer->cltimeout = 5*70;
                p->racer->finished  = TRUE;
            }
        }
    }
    if (RaceStarted) {
        if (p->racer->cltimeout > 0)  p->racer->cltimeout--;
        if (p->racer->clexttime > 0)  p->racer->clexttime--;
        if (p->racer->clfinallap > 0) p->racer->clfinallap--;
        if (p->racer->clendrace > 0)  p->racer->clendrace--;
        if (p->racer->clendrace <= 0)
            GAME_EFF_UnlockChannel(SH_Vtal, 4);
    }

    cfg = &p->cfg;
    ctl = p->UserControl + (GlobalClock % MAX_USERCONTROLS);

    if (p->status == PLYST_BROKEN)
        memset(ctl, 0, sizeof(*ctl));

    if (p->flags & PLYF_ISCONSOLE) {
        LastConsoleCtl = ctl->control;
        if (ctl->control & UCTL_UP)
            ConsoleUp++;
    } else {
        LastOtherCtl = ctl->control;
        if (ctl->control & UCTL_UP)
            OtherUp++;
    }

        // Speed changes.
    if ((ctl->control & UCTL_UP) || ctl->analogY > 5) {
        if (GearInfo[p->gear].ratio >= 0) {
            if (!(p->flags & PLYF_SLIDING))
                p->revo += FPMultDiv(GearInfo[p->gear].accel, p->maxspeed, MDL_Cars[MDL_NCars-1].maxSpeed);
            else if (p->flags & PLYF_TRACTING)
                p->revo += GearInfo[p->gear].accel/2;
        } else if (RaceStarted) {
            p->revo -= GearInfo[p->gear].accel;
            if (p->revo <= 0)
                p->gear++;
        }
    }
    braking = 0;
    dv = 0;
    p->v = FPMultDiv(p->v, MDL_Cars[MDL_NCars-1].maxSpeed, p->maxspeed);
    if ((ctl->control & UCTL_DOWN) || ctl->analogY < -5) {
        braking = 1;
        if (GearInfo[p->gear].ratio > 0) {
            sint32 k, d;
            if (p->flags & PLYF_SLIDING)
                k =  (1 << 12) + (p->v >> 8);
            else
                k =  (1 << 12) + (p->v >> 8);
            d = k; // + k*(-ctl->analogY)/16;
            p->revo  = FP16Div(p->v - d, GearInfo[p->gear].ratio);
        } else if (RaceStarted && p->gear > 0 && GearInfo[p->gear].ratio == 0) {
            p->gear--;
            p->revo += GearInfo[p->gear].accel;
        } else if (GearInfo[p->gear].ratio < 0) {
            p->revo += GearInfo[p->gear].accel;
        }
    }
    if (!(p->flags & PLYF_SLIDING))
        p->revo -= GearInfo[p->gear].accel / 8;

    if (p->revo <= 0) {
        p->gear = 1;
        p->revo = 0;
    }
        // Allow gear changes only if race is on.
    if (RaceStarted) {
        if ((ctl->scancode == cfg->kgear && (ctl->control & UCTL_UP)) ||
            (ctl->scancode == cfg->kup   && (ctl->control & UCTL_GEAR)) ||
            (ctl->control & UCTL_GEARUP) ||
            (p->automatic && /*p->gear > 0 && */p->revo > GearInfo[p->gear].maxrevo)         // Automatic gear.
            ) {
            if (GearInfo[p->gear].ratio >= 0 && p->gear < MAXGEAR-1) {
                p->gear++;
                if (p->revo > (0x0D << 18))
                    GearChange(p);
                if (GearInfo[p->gear].ratio > 0)
                    p->revo = FP16Div(p->v, GearInfo[p->gear].ratio);
            }
        }
        if ((ctl->scancode == cfg->kgear && !(ctl->control & UCTL_UP)) ||
            (ctl->control & UCTL_GEARDN) ||
            (p->automatic && p->revo < GearInfo[p->gear].minrevo)         // Automatic gear.
            ) {
            if (p->gear > 2 && GearInfo[p->gear-1].ratio != 0) {
                sint32 r;

                r = FP16Div(p->v, GearInfo[p->gear-1].ratio);
                if (r > (1 << 22))
                    r = (1 << 22);
                if (r <= (1 << 22)) {
                    p->revo = r;
                    p->gear--;
                    if (GearInfo[p->gear].ratio > 0) {
                        if (p->revo > (0x0D << 18))
                            GearChange(p);
                    }
                }
            }
        }
    }
    if (p->revo > (1 << 22))
        p->revo = (1 << 22);

    {
        sint32 s;
        s = (p->v) << 8;
//        s = (p->v - (1 << 19)) << 8;
//        s = p->slidspeed << 8;
        if (s < 0)
            s = 0;
        if (p->cartype == 1)    // Derrape
            maxva = 40 - FPMult(55, s);
        else
            maxva = 40 - FPMult(45, s);
        if (maxva < 7)
            maxva = 7;
        if (p->slidcounter > 0) {
            maxva = maxva - FPMultDiv(maxva, p->slidcounter, 1 << 8);
            if (maxva < 0)
                maxva = 0;
        }
        if (p->cartype == 1)    // Derrape
            maxa  = 8*(120 - FPMult(160, s));
        else
            maxa  = 8*(120 - FPMult(160, s));
        if (p->cartype == 1) {  // Derrape
            if (maxa < 25*8)
                maxa = 25*8;
        } else {
            if (maxa < 25*8)
                maxa = 25*8;
        }
    }
    a = maxva;
    if ((ctl->control & UCTL_LEFT) || ctl->analogX < -4) {
        if (p->va < 0)
            p->va = 0;
    }
    if ((ctl->control & UCTL_RIGHT) || ctl->analogX > 4) {
        if (p->va > 0)
            p->va = 0;
        a = -a;         // Negative turn
    }
    if (!(ctl->control & (UCTL_RIGHT | UCTL_LEFT)) && Abs32(ctl->analogX) <= 4)
        a = 0;
    p->va += a;
/*
    if (Abs32(ctl->analogX) > 4 && Abs32(ctl->analogX) <= 8)
        p->va = maxa*(-ctl->analogX)/16;
    if (Abs32(ctl->analogX) > 8)
//        p->va += maxa*(-ctl->analogX)/16/4;
        p->va += 3*a*(-Sgn(ctl->analogX));
*/
    if (Abs32(ctl->analogX) > 4)
        maxa = maxa*(-ctl->analogX)/16;

    if (p->va < -maxa)
        p->va = -maxa;
    if (p->va > maxa)
        p->va = maxa;
    if (!(ctl->control & UCTL_LEFT) && !(ctl->control & UCTL_RIGHT) && Abs32(ctl->analogX) <= 4)
        p->va /= 2;

    p->v = FP16Mult(p->revo, GearInfo[p->gear].ratio);
    p->v = FPMultDiv(p->v, p->maxspeed, MDL_Cars[MDL_NCars-1].maxSpeed);

        // Do sliding/out-of-control stuff.
    if (p->slidcounter > 0) {
        p->revo = p->revo - (p->revo >> 6);
        p->slidcounter--;
        p->thn->angle += p->slidva;//Sgn(p->slidspeed);
        if (p->slidva > 2)
            p->slidva -= 2;
        else if (p->slidva < -2)
            p->slidva += 2;
        p->slidspeed = p->slidspeed - (p->slidspeed >> 7);
    } else {
        bool doslide;

        doslide = FALSE;

//        if (p->v > (3 << 19) && Abs32(p->va) >= (Abs32(maxa) - 2*8))
//        if (p->slidspeed > (3 << 19) && Abs32((1+braking)*p->va) >= (Abs32(maxa) - 2*8))
        if (p->v > (3 << 19) && Abs32((1+2*braking)*p->va) >= (Abs32(maxa) - 1*8))
            doslide = TRUE;

        if (p->cartype != 1)    // Derrape
            doslide = 0;

        if (doslide) {
            p->flags |= PLYF_SLIDING;
/*
            p->ma         +=  FPMultDiv(p->va,  p->v, (1 << 22));
            p->thn->angle +=  FPMultDiv(p->va*(4+8*braking)/4,  p->v, (1 << 22));
*/
            p->ma         +=  FPMultDiv(p->va,  p->v, (1 << (22+braking)));
//            p->ma         +=  FPMultDiv((sint16)(-p->ma + p->thn->angle)/16,  p->slidspeed, (1 << 22));
            p->thn->angle +=  FPMultDiv(p->va*(1+1*braking)/2,  p->v, (1 << 22));
            p->slidspeed  -=  FPMult((p->slidspeed >> 9), Cos(p->thn->angle-p->ma)) + (p->slidspeed >> 9);
            p->slidva = p->va;
        } else if ((p->flags & PLYF_SLIDING)) {
            sint32 k;
            p->flags |= PLYF_TRACTING;
            p->slidspeed  +=  FPMult((p->v-p->slidspeed)/128, Cos(p->thn->angle-p->ma));
            k =  ((sint16)(p->thn->angle - p->ma))/32;
            p->ma += k;
//            if (Abs32(k) < 0x400/32
            if (Abs32(k) < 0x400/64
             && (p->slidspeed == 0 || (p->v-p->slidspeed) < (1 << 19) || (p->v << 4)/p->slidspeed < (6 << 4)/5))
                p->flags &= ~(PLYF_SLIDING|PLYF_TRACTING);
        } else {
            p->flags &= ~(PLYF_SLIDING|PLYF_TRACTING);
            p->ma = p->thn->angle;
            p->slidspeed = p->v;
            p->slidva = 0;
        }
    }
/*
    if (p->slidcounter > 0) {
        dx = FPMult(Abs32(p->slidspeed), Cos(p->ma));
        dy = FPMult(Abs32(p->slidspeed), Sin(p->ma));
        p->thn->angle +=  FPMultDiv(p->va,  p->v, (1 << 22));
    } else {
        sint32 k;
        k = p->v - p->slidspeed;
        if (k < 0) k = 0;
        dx = FPMult(k, Cos(p->thn->angle))
           + FPMult(Abs32(p->slidspeed), Cos(p->ma));
        dy = FPMult(k, Sin(p->thn->angle))
           + FPMult(Abs32(p->slidspeed), Sin(p->ma));
        p->thn->angle +=  FPMultDiv(p->va,  k, (1 << 22));
    }
*/
    dx = FPMult((p->slidspeed), Cos(p->ma));
    dy = FPMult((p->slidspeed), Sin(p->ma));
    p->thn->angle +=  FPMultDiv(p->va,  p->v, (1 << 22));

        // Advance tires.
    p->thn->tirerot += (p->v / (32*(1 << 4)));
    if (p->cartype == 0)
        p->thn->tiredir = -4*p->va - FPMultDiv(16*p->va, (p->v), 1 << 22);
    else
        p->thn->tiredir = -8*p->va - FPMultDiv(32*p->va, (p->v), 1 << 22);

    oldx = p->thn->x;
    oldy = p->thn->y;
    sec = p->thn->sec;

    if (p->slidcounter <= 0) {
        if (Abs32(p->v) > (1 << 9))
            p->movangle = GetAngle(dx, -dy);
        else if (p->v > (1 << 17))
            p->movangle = GetAngle(dx >> 5, -dy >> 5);
        else
            p->movangle = p->thn->angle;
    }

    THN_MoveThing(p->thn,
                  p->thn->x + dx,
                  p->thn->y + dy);

    assert(p->thn->x = oldx + dx);
    assert(p->thn->y = oldy + dy);

    if (sec->flags == 0) {  // Car has somehow gone off road.
        p->slidcounter = 0;
        p->slidspeed = 0;
        p->ma = p->thn->angle;
    } else {        // Collision test, only when coming from the road.
        THN_PThing t;
        int nv;
            // first, collide with walls.
        for (nv = 0; nv < 4; nv++) {
            SEC_PSector vs;
            sint32 cx, cy;

            switch (nv) {
                case 0:
                    cx =  (3 << 21);
                    cy =  (3 << 21);
                    break;
                case 1:
                    cx = -(3 << 21);
                    cy =  (3 << 21);
                    break;
                case 2:
                    cx =  (3 << 21);
                    cy = -(3 << 21);
                    break;
                case 3:
                    cx = -(3 << 21);
                    cy = -(3 << 21);
                    break;
            }
            vs = SEC_FindSector(&Map.sec, p->thn->sec,
                    SEC_TOSEC(p->thn->x + cx),
                    SEC_TOSEC(p->thn->y + cy));

            if (sec != vs && vs->flags == 0) {
                int i;
                sint32 k, ca;

                if (p->flags & PLYF_ISCONSOLE)
                    GAME_EFF_Start(SH_Vtal, 3, 256,  120, 0, RACE_EffCrash);

                THN_AddThing(p->thn->x + cx*3/4, p->thn->y + cy*3/4, 0, THNT_SPARKS, 0, 0);
                {
                    sint32 dist, mindist, mini;
                    mindist = 0x3FFFFFFF;
                    mini = -1;
                    for (i = 0; i < sec->nv; i++) {
                        if (sec->v[i].otherside != vs)
                            ;//continue;
                        if (sec->v[i].tex == NULL)
                            continue;
                        dist = FPMult(Cos(sec->v[i].angle), SEC_TOSEC(p->thn->x)-sec->v[i].v0->x)
                             + FPMult(Sin(sec->v[i].angle), SEC_TOSEC(p->thn->y)-sec->v[i].v0->y);
                        if (Abs32(dist) < mindist) {
                            mindist = Abs32(dist);
                            mini = i;
                        }
                    }
                    i = mini;
                }
                if (i < 0 || i >= sec->nv) { // Big jump from sector (not found).
                    ca = 0x1000;
                    p->slidspeed = p->v >> 1;
                        // Go back to prev position
                    THN_MoveThing(p->thn, p->thn->x - dx - cx/16, p->thn->y - dy - cy/16);
                } else {
                    ca = (sint16)(2*(sec->v[i].angle - ((dword)p->ma/*+p->thn->angle*/)/1));
                    p->slidspeed = p->v >> 1;
                        // Go back to prev position
//                    THN_MoveThing(p->thn, p->thn->x - dx - cx/16, p->thn->y - dy - cy/16);
                    THN_MoveThing(p->thn, p->thn->x - dx - cx/4, p->thn->y - dy - cy/4);
                    if (p->cfg.kup == kUARROW) {
                        Crack = ca;
                        WallAngle = sec->v[i].angle;
                    }
                }
                p->ma += ca;
                k = p->revo - p->revo/50;
                p->slidcounter = 10+FPMultDiv(Abs32(p->v >> 14), Abs32(ca), 0x10000);
                p->slidva = FPMultDiv(ca >> 4, p->slidspeed, (1 << 22));
                break;  // Collided, don't check more collisions.
            }
        }

            // Then, with other things
        if (!GL_NoCollide && THN_CalcCollide(p->thn, NULL, NULL, &t)) {
            sint16 da;
            sint32 k;

            if (p->flags & PLYF_ISCONSOLE)
                GAME_EFF_Start(SH_Vtal, 3, 256,  120, 0, RACE_EffCrash);
            THN_AddThing(p->thn->x/2 + t->x/2, p->thn->y/2 + t->y/2, 0, THNT_SPARKS, 0, 0);

            THN_MoveThing(p->thn,
                          p->thn->x + (sint32)(p->thn->x - t->x)/6,
                          p->thn->y + (sint32)(p->thn->y - t->y)/6);
              // Second try, to better get rid of deep collisions.
              // Don't use a while, 'cause it will hang due to Murphy's law.
            if (THN_CalcCollide(p->thn, NULL, NULL, &t))
                THN_MoveThing(p->thn,
                              p->thn->x + (sint32)(p->thn->x - t->x)/6,
                              p->thn->y + (sint32)(p->thn->y - t->y)/6);
            k = FPMultDiv(p->revo, 7, 8);
            if (k != 0) p->revo = k;
            da = GetAngle(p->thn->x - t->x, t->y - p->thn->y) - p->thn->angle;
            p->ma += da/25;
        }
    }

        // Keep track of position in the race, number of laps, etc.
    if (p->status == PLYST_RACING) {
        sint32 dx, dy;
        sint16 d;

        dx = (((Map.path.points[p->racer->npoint].x >> 1) + (1 << 30)) >> 18) - (p->thn->x >> 18);
        dy = (((Map.path.points[p->racer->npoint].y >> 1) + (1 << 30)) >> 18) - (p->thn->y >> 18);
        if (Abs32((sint16)(GetAngle(dx, -dy) - Map.path.points[p->racer->npoint].dir)) > 0x4000) {
            Map.path.points[p->racer->npoint].ncars--;
            p->racer->npoint++;
            if (p->racer->npoint >= Map.path.numpoints) {
                int nl = p->racer->nlap;
                if (nl >= SIZEARRAY(p->racer->laptime))
                    nl = SIZEARRAY(p->racer->laptime)-1;
                if (p->racer->bestlaptime == 0
                 || p->racer->bestlaptime > p->racer->laptime[nl]) {
                    if (p->flags & PLYF_ISCONSOLE)
                        GAME_EFF_Start(SH_Vtal, 4, 256, 400, 0, RACE_EffLapRecord);
                    p->racer->bestlaptime = p->racer->laptime[nl];
                }
                if (nl >= SIZEARRAY(p->racer->laptime)-1) {
                    int i;
                    for (i = 0; i < nl; i++)
                        p->racer->laptime[i] = p->racer->laptime[i+1];
                    p->racer->laptime[nl] = 0;
                }
                if (p->status == PLYST_RACING && GL_TimedRace) {
                    nl = p->racer->nlap+1;
                    if (nl >= SIZEARRAY(GL_RaceTime))
                        nl = SIZEARRAY(GL_RaceTime)-1;
                    p->racer->laptimeleft += GL_RaceTime[nl];
                    if (p->racer->laptimeleft > GL_RaceTime[0])
                        p->racer->laptimeleft = GL_RaceTime[0];
                        // Extended time!
                    p->racer->clexttime = 3*70;
                    if (p->flags & PLYF_ISCONSOLE)
                        GAME_EFF_Start(SH_Vtal, 4, 256, 400, 0, RACE_EffExtended);
                }
                p->racer->npoint = 0;
                if (p->status == PLYST_RACING && (GL_TimedRace || p->racer->nlap < GL_RaceLaps)) {
                    p->racer->nlap++;
                    if (p->racer->nlap == GL_RaceLaps-1) {
//                        GAME_EFF_Start(SH_Vtal, 4, 256, 400, 0, RACE_EffFinalLap);
                        p->racer->clfinallap = 3*70;
                        p->racer->clexttime = 0;
                    }
                    if (p->racer->nlap == GL_RaceLaps) {
                        p->status = PLYST_WON;
                        p->racer->clexttime = 0;
                        p->racer->clendrace = 5*70;
                        p->racer->finished  = TRUE;
                        p->racer->nlap--;
                        if (Map.racers[0]->thn == p->thn && (p->flags & PLYF_ISCONSOLE)) {
                            GAME_EFF_Start(SH_Vtal, 4, 256, 500, 0, RACE_EffYouWin);
                            GAME_EFF_LockChannel(SH_Vtal, 4);
                        }
                    }
                }
            }
            Map.path.points[p->racer->npoint].ncars++;
        }
    }

        // Ground detection.
    if (p->thn->flags & THNF_SOLID) {
        byte c;
        uint32 x, y;
        byte *tile;

        x = p->thn->x;
        y = p->thn->y;
        assert(((y >> 25)*128 + (x >> 25)) < 16384);
        tile = Map.map[(y >> 25)*128 + (x >> 25)];
        assert((((y & 0x1FFFFFF) >> 19)*64 + ((x & 0x1FFFFFF) >> 19)) < 4096);
        c = tile[((y & 0x1FFFFFF) >> 19)*64 + ((x & 0x1FFFFFF) >> 19)];
//        tile[((y & 0x1FFFFFF) >> 19)*64 + ((x & 0x1FFFFFF) >> 19)] = 160;
        if (c < 160 || c >= 192) {
            int base;

            if (c >= 48 && c < 63)              // Amarillo
                base = 1;
            else if (c >= 64 && c <= 79)        // Verde
                base = 2;
            else                                // Marron
                base = 0;
            p->revo -= p->revo >> 7; //FPMultDiv(p->revo, 98, 100);
            if (p->v > (1 << 3)) {
                p->thn->z = 0x20000 + ((RND_GetNum() % (1 << 5)) << 14);
                if (!(RND_GetNum() & 7)) {
                    THN_PThing t;
                    t = THN_AddThing(x + ((RND_GetNum()&0x7FFF) << 7) - (1 << 21),
                                     y + ((RND_GetNum()&0x7FFF) << 7) - (1 << 21),
                                     0, THNT_SMOKE + base, 0, 0);
                    if (t != NULL) {
                        ((THN_PSmoke)t->data)->dx = FPMult(p->v, Cos(p->thn->angle))/2;
                        ((THN_PSmoke)t->data)->dy = FPMult(p->v, Sin(p->thn->angle))/2;
                    }
                }
            } else
                p->thn->z = 0x20000;
        } else {
/*
            if (p->v >= (1 << 7))
                p->thn->z = 0x20000 - ((RND_GetNum() % (p->v >> 7)) << 4);
            else
*/
                p->thn->z = 0x20000;
        }
    }

}

// ------------------------------ USERCTL.C ----------------------------

