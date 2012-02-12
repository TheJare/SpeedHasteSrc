// ------------------------------ RECORDS.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _RECORDS_H_
#define _RECORDS_H_

#ifndef _BASE_H_
#include <base.h>
#endif

    // Best runners.
typedef struct {
    char  name[2][8];
    int   ncirc[2];
    int   nlaps[2];
    dword time[2];
} REC_TBestRecord, *REC_PBestRecord;

    // Lap records for each circuit. As many as there are circuits.
typedef struct {
    char  name[2][8];
    int   ncirc[2];
    dword time[2];
} REC_TCircRecord, *REC_PCircRecord;

// ---------------------------

    // --------- Tablas de records.

PUBLIC REC_PBestRecord REC_Best;
PUBLIC int             REC_NBest;

PUBLIC REC_PCircRecord REC_Circ;
PUBLIC int             REC_NCirc;

    // --------- Funciones

PUBLIC bool REC_Init(void);
PUBLIC void REC_Save(void);
PUBLIC void REC_End(void);

PUBLIC int REC_AddBest(REC_PBestRecord list, int n, REC_PBestRecord rec, int ncar);
PUBLIC int REC_AddCirc(REC_PCircRecord list, int n, REC_PCircRecord rec, int ncar);

    // -1 if ESC
PUBLIC int REC_ShowBest(REC_PBestRecord list, int n, int ncar);
PUBLIC int REC_ShowCirc(REC_PCircRecord list, int n, int ncar);

PUBLIC int REC_InputBest(REC_PBestRecord list, int n, int nplayer, int ncirc, int nlaps, dword time, int ncar);
PUBLIC int REC_InputCirc(REC_PCircRecord list, int n, int nplayer, int ncirc, dword time, int ncar);

#endif

// ------------------------------ RECORDS.H ----------------------------
