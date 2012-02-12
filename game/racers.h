// ------------------------------ RACERS.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _RACERS_H_
#define _RACERS_H_

#include "things.h"

typedef struct {
    THN_PThing thn;         // Point back to racer's thing.
    sint32 npoint, nlap;
    dword  laptime[5];
    sint32 laptimeleft;
    dword  bestlaptime;
    dword  totaltime;
    dword  cltimeout;
    dword  clexttime;
    dword  clfinallap;
    dword  clendrace;
    dword  finished;
} RCS_TRacer, *RCS_PRacer;

PUBLIC int         RCS_NRacers;
PUBLIC int         RCS_MaxRacers;
PUBLIC RCS_TRacer *RCS_Racers;

// ---------------------------

PUBLIC bool RCS_Init(int nracers);

PUBLIC void RCS_End(void);

#endif

// ------------------------------ RACERS.H ----------------------------
