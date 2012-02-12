// ------------------------------ CARS.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "cars.h"
#include "globals.h"
#include <sincos.h>
#include <atan.h>
#include <jclib.h>
#include <stdio.h>

#include "race.h"

// ########################################################

PRIVATE THN_TBounds CarBounds = {
/*
    3 << 21,
    0,
*/
    0xB00000,
    4, {
        {  0xB00000, 0x380000 },
        {  0xB00000, -0x380000 },
        { -0x800000, -0x380000 },
        { -0x800000, 0x380000 },
    }
};

PRIVATE void SetCarTarget(CAR_PCar car) {
    sint32 s;
    car->tx = (Map.path.points[car->NextPoint].x >> 1) + (1 << 30);
    car->ty = (Map.path.points[car->NextPoint].y >> 1) + (1 << 30);
    Map.path.points[car->NextPoint].ncars++;

    if (Map.path.points[car->NextPoint].speed > 8000*256
     && Map.path.points[car->NextPoint].ncars > 1)
        s = FPMultDiv(Map.path.points[car->NextPoint].speed-8000*256, 1 << 24, 5000*256);
    else
        return;

    car->tx += (((uint32)(RND_GetNum() << 16))%s)
              - s/2;
    car->ty += (((uint32)(RND_GetNum() << 16))%s)
              - s/2;
}

PUBLIC bool CAR_New(THN_PThing t, int type, word flags) {
    CAR_PCar car;
    static char carname[] = "carrn0";

    car = NEW(sizeof(*car));
    if (car == NULL)
        return FALSE;
    t->data    = car;
    t->routine = &CAR_RunIt;
    t->flags   = THNF_SOLID;
    carname[3] = '0' + GL_CarType;
    carname[5] = '0' + (((byte)type) >> 4);     // Up to 16 different cars.
    t->spr     = FS3_New(carname);
    t->bounds  = &CarBounds;
/*
    t->nTable  = type & 0xF;
    if (t->nTable >= 11)
        BASE_Abort("Unkown color %d", t->nTable);
*/
    car->thn   = t;
    car->flags = flags;
    car->toAngle = t->angle;
    car->NextPoint = 0;
    SetCarTarget(car);
    REQUIRE(RCS_NRacers < RCS_MaxRacers);
    car->racer = RCS_Racers + RCS_NRacers;
    car->racer->thn = car->thn;
    RCS_NRacers++;

/*
    if (GL_CarType == 2) CarBounds.radius = 4 << 21;
    else                 CarBounds.radius = 3 << 21;
*/
    return TRUE;
}

PRIVATE void GearChange(CAR_PCar car) {
    THN_PThing t;
    uint32 x, y;

    if (RND_GetNum() % 2048)
        return;
    x = car->thn->x - FPMult(1 << 23, Cos(car->thn->angle));
    y = car->thn->y - FPMult(1 << 23, Sin(car->thn->angle));
    if (GL_CarType == 0) {
        t = THN_AddThing(x, y, 0, THNT_SPARKS, 0, 0);
        if (t != NULL) {
            ((THN_PSparks)t->data)->dx = FPMult(car->v, Cos(car->thn->angle))/4;
            ((THN_PSparks)t->data)->dy = FPMult(car->v, Sin(car->thn->angle))/4;
        }
    }
}

PUBLIC void CAR_RunIt(CAR_PCar car) {
    THN_PThing thn;
    int x, y;

    if (Paused || !RaceStarted)
        return;

    assert(car != NULL);
    thn = car->thn;
    assert(thn != NULL);


    // Turn car. Then, if almost aligned with target, align it.

//    thn->angle += car->av;
    thn->angle += FPMultDiv(car->av,  car->v, (1 << 22));
    if ((thn->angle >> 6) == (car->toAngle >> 6)) {
        thn->angle = car->toAngle;
        car->state &= ~CARST_TURNING;
        car->av = 0;
    }


    // Advance the car.

    x = thn->x + FPMult(car->v, Cos(thn->angle));
    y = thn->y + FPMult(car->v, Sin(thn->angle));
    THN_MoveThing(thn, x, y);


    // Adjust the speed and angle parameters for path.

    {
        sint32 dx, dy;
        dword d2;
        sint16 d;

        dx = (((Map.path.points[car->racer->npoint].x >> 1) + (1 << 30)) >> 18) - (thn->x >> 18);
        dy = (((Map.path.points[car->racer->npoint].y >> 1) + (1 << 30)) >> 18) - (thn->y >> 18);
        if (Abs32((sint16)(GetAngle(dx, -dy) - Map.path.points[car->racer->npoint].dir)) > 0x4000) {
            car->racer->npoint++;
            if (car->racer->npoint >= Map.path.numpoints) {
                car->racer->npoint = 0;
                car->racer->nlap++;
            }
        }

        dx = (car->tx >> 18) - (thn->x >> 18);
        dy = (car->ty >> 18) - (thn->y >> 18);
        d2 = dx*dx + dy*dy;
        if ((d2 < (1 << (15))) ||
            (d2 < (1 << (19))
             && Abs32((sint32)(sint16)(GetAngle(dx, -dy) - thn->angle)) > 0x4000)) {
            Map.path.points[car->NextPoint].ncars--;
            car->NextPoint++;
            if (car->NextPoint >= Map.path.numpoints)
                car->NextPoint = 0;
            SetCarTarget(car);
            dx = (car->tx >> 18) - (thn->x >> 18);
            dy = (car->ty >> 18) - (thn->y >> 18);
        }

        car->toAngle = GetAngle(dx, -dy);
//        thn->angle = car->toAngle;      // úú

        d = (sint16)(car->toAngle - thn->angle + car->av);

        {
            sint32 s, a, maxa, maxva;
            s = -Sin(car->v >> (22 - 14));
            maxva = 40 - FPMult(30, s);
            maxa  = 120 - FPMult(105, s);
            a = maxva;
            if (d < 0) {
                if (car->av > 0)
                    car->av -= 2*a;
                else
                    car->av -= a;
            } else if (d > 0) {
                if (car->av < 0)
                    car->av += 2*a;
                else
                    car->av += a;
            }
            if (car->av < -maxa*8)
                car->av = -maxa*8;
            if (car->av > maxa*8)
                car->av = maxa*8;
        }
            // Advance tires.
        {
            sint32 tdir;
            thn->tirerot += Abs32(car->v / (32*(1 << 4)));
            if (GL_CarType == 0)
                tdir = -4*car->av - FPMultDiv(16*car->av, (car->v), 1 << 22);
            else
                tdir = -8*car->av - FPMultDiv(32*car->av, (car->v), 1 << 22);
            thn->tiredir += (sint16)(tdir-thn->tiredir)/32;
        }

//        if (Map.path.points[car->NextPoint].speed > 8000*256)
            car->toSpeed = 0 //2*(car->carSpeed) * ((Map.path.points[car->NextPoint].speed-8000*256) / 256)// / (64*2))
                         + Map.path.points[car->NextPoint].speed;
//        if (GL_CarType == 1)
            car->toSpeed -= 1800*256;
//        else
//            car->toSpeed = Map.path.points[car->NextPoint].speed;

        if (car->toSpeed < car->v) {
            car->v -= (GL_CarType == 1)?0x8000*3/4:0x8000;
            GearChange(car);
        } else if (car->toSpeed > car->v) {
            sint32 acc;
            acc = (GL_CarType == 1)?0x1000:0x1100 - FPMultDiv(0xC00, car->v, 1 << 22);
//            car->v += ((GL_CarType == 1)?(0x3A00):0x1500)*(car->carSpeed+20)/64;
            car->v += acc*(car->carSpeed+3)/16;
            GearChange(car);
        }
    }

//    if (BASE_CheckArg("nocollide") <= 0)
    {
        THN_PThing t;
        if (THN_CalcCollide(thn, NULL, NULL, &t)) {
            sint16 da;

            if (((word)t->type & 0xFF00) == THNT_PLAYER && (((PLY_PPlayer)t->data)->flags & PLYF_ISCONSOLE))
                GAME_EFF_Start(SH_Vtal, 3, 256,  120, 0, RACE_EffCrash);

            THN_AddThing(thn->x/2 + t->x/2, thn->y/2 + t->y/2, 0, THNT_SPARKS, 0, 0);

            THN_MoveThing(thn,
                          thn->x + (sint32)(thn->x - t->x)/6,
                          thn->y + (sint32)(thn->y - t->y)/6);
              // Second try, to better get rid of deep collisions.
              // Don't use a while, 'cause it will hang due to Murphy's law.
            if (THN_CalcCollide(thn, NULL, NULL, &t))
                THN_MoveThing(thn,
                              thn->x + (sint32)(thn->x - t->x)/6,
                              thn->y + (sint32)(thn->y - t->y)/6);
            car->v = FPMultDiv(car->v, 7, 8);
            da = GetAngle(thn->x - t->x, t->y - thn->y) - thn->angle;
            thn->angle += da/25;
        }
    }
        // Ground detection.
    {
        byte c;
        uint32 x, y;
        byte *tile;

        x = thn->x;
        y = thn->y;
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
            car->v -= car->v >> 7; //FPMultDiv(car->v, 98, 100);
            if (car->v > (1 << 3)) {
                thn->z = 0x20000 + ((RND_GetNum() % (1 << 5)) << 14);
                if (!(RND_GetNum() & 7)) {
                    THN_PThing t;
                    t = THN_AddThing(x + ((RND_GetNum()&0x7FFF) << 7) - (1 << 21),
                                     y + ((RND_GetNum()&0x7FFF) << 7) - (1 << 21),
                                     0, THNT_SMOKE + base, 0, 0);
                    if (t != NULL) {
                        ((THN_PSmoke)t->data)->dx = FPMult(car->v, Cos(thn->angle))/2;
                        ((THN_PSmoke)t->data)->dy = FPMult(car->v, Sin(thn->angle))/2;
                    }
                }
            } else
                thn->z = 0x20000;
        } else {
            if (car->v > (1 << 7))
                thn->z = 0x20000 - ((RND_GetNum() % (car->v >> 7)) << 4);
            else
                thn->z = 0x20000;
        }
    }
}

// ------------------------------ CARS.C ----------------------------

