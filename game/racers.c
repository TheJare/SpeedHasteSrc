// ------------------------------ RACERS.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "racers.h"

#include <string.h>

int         RCS_NRacers = 0;
int         RCS_MaxRacers = 0;
RCS_TRacer *RCS_Racers = NULL;

// ---------------------------

PUBLIC bool RCS_Init(int nracers) {
    if (nracers <= 0 || RCS_Racers != NULL)
        return FALSE;
    RCS_Racers = NEW(nracers*sizeof(*RCS_Racers));
    if (RCS_Racers == NULL)
        return FALSE;
    memset(RCS_Racers, 0, nracers*sizeof(*RCS_Racers));
    RCS_NRacers = 0;
    RCS_MaxRacers = nracers;
    return TRUE;
}

PUBLIC void RCS_End(void) {
    if (RCS_Racers != NULL)
        DISPOSE(RCS_Racers);
    RCS_NRacers = 0;
    RCS_MaxRacers = 0;
}

// ------------------------------ RACERS.C ----------------------------