// ------------------------------ SKIDS.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _SKIDS_H_
#define _SKIDS_H_

#ifndef _BASE_H_
#include <base.h>
#endif


typedef struct SKID_SSkidPoint {
    sint32 x0, y0, x1, y1;
    bool isfirst;
    struct SKID_SSkidPoint next;
} SKID_TSkidPoint, *SKID_PSkidPoint;


PUBLIC bool SKID_Init(int max);
PUBLIC void SKID_End(void);



PUBLIC void SKID_Render(int cx, int cy, F3D_PCamera cam);


#endif

// ------------------------------ SKIDS.H ----------------------------
