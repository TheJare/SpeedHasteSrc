// --------------------------- JOYSTICK.H ------------------------------
// For use with WATCOM 9.5 + DOS4GW
// (C) Copyright 1993/4 by Jare & JCAB of Iguana-VangeliSTeam.

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#ifndef _BASE_H_
#include <base.h>
#endif


bool JOY_Read(int *xa, int *ya, int *xb, int *yb, uint *ba, uint *bb);
   /*       --------
      Stores the joystick position in the given places. When any pointer
      is NULL, it is not stored (or even calculated). If there's no
      joystick it returns FALSE, else TRUE.
   */

bool JOY_BIOSRead(int *xa, int *ya, int *xb, int *yb, uint *ba, uint *bb);
    // Reads it through the BIOS. ALWAYS returns TRUE, doesn't detect.

// -----------------------------
// Med-level functions.
// These perform averaging of values to avoid multitaskers' interference.

    // bios = TRUE indicates that joystick reading should be done thru
    // the BIOS INT 15h. Returns bits 0 & 1 showing joystick presence.
    // nvalues helps remove errors while in a multitasker by averaging
    // previous stick values. BUT it kinda slows down the response.
PUBLIC byte JOY_Init(bool bios, int nvalues);

PUBLIC void JOY_End(void);

PUBLIC bool JOY_Get(int *xa, int *ya, int *xb, int *yb, uint *ba, uint *bb);

// -----------------------
// Calibrated reading.

    // Values are left, left-center, right-center, right.
PUBLIC word JOY_CalAX[4];
PUBLIC word JOY_CalAY[4];
PUBLIC word JOY_CalBX[4];
PUBLIC word JOY_CalBY[4];

PUBLIC bool JOY_CalGet(int wx, int wy, int *xa, int *ya, int *xb, int *yb, uint *ba, uint *bb);

#endif

// --------------------------- JOYSTICK.H ------------------------------

