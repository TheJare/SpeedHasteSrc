// ------------------------------ CARS.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _CARS_H_
#define _CARS_H_

#include "things.h"
#include "racers.h"

typedef struct {
/*
    sint32 x, y;
    word angle;
    word nFrames;
    FS3_PSprite frames;
*/
    THN_PThing thn;
    sint32 v, av;
    word   state;
    word   flags;
    word   toAngle;
    word   gear;
    sint32 toSpeed;
    sint32 carSpeed;
    dword  NextPoint;
    uint32 tx, ty;

    RCS_PRacer racer;
    word   aCounter;
    sint32           cartype, maxspeed, damage;
    sint32           fill[4];
} CAR_TCar, *CAR_PCar;

enum {
    CARST_TURNING = 0x0001,
    CARST_STOPPED = 0x0002,
};

PUBLIC bool CAR_New(THN_PThing t, int type, word flags);

PUBLIC void CAR_RunIt(CAR_PCar car);

#endif

// ------------------------------ CARS.H ----------------------------
