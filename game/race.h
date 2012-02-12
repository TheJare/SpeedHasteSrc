// ------------------------------ RACE.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _RACE_H_
#define _RACE_H_

#include <base.h>

#include <gamevtal.h>
/*

typedef struct {

    GAME_PEffect EffMotor;

} RACE_TRace, *RACE_PRace;
*/

#include "net.h"

typedef struct {
    int   nlaps;
    dword time;
    dword best;
    dword flags;
    int   pos;
} RACE_TResult, *RACE_PResult;

enum {
  RACERF_ISCONSOLE  = 0x0001,
  RACERF_ABANDONED  = 0x0002,
  RACERF_NOTPRESENT = 0x8000,
};

PUBLIC int RACE_DoRace(int mode, RACE_TResult result[NET_MAXNODES]);

    // TRUE if user quitted manually.
PUBLIC bool RACE_DoDemo(void);

    // Snoop wires.
PUBLIC void RACE_HandleComms(bool ints);

PUBLIC GAME_PEffect RACE_EffCrash;

PUBLIC  GAME_PEffect RACE_EffFinalLap ,
                     RACE_EffLapRecord,
                     RACE_EffTimeout  ,
                     RACE_EffExtended ,
                     RACE_EffYouWin   ,
                     RACE_EffGameOver ;

#endif

// ------------------------------ RACE.H ----------------------------
