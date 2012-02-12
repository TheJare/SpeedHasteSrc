// ----------------------------- CURSOR.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#ifndef _CURSOR_H_
#define _CURSOR_H_

#ifndef _BASE_H_
#include <base.h>
#endif

PUBLIC bool CURS_ThereIsMouse, CURS_ThereIsJoy;

PUBLIC bool CURS_Init(bool mouse, bool joy);
    // bools specify if that device is to be used or no (even if exists).
    // Returns FALSE if no pointing device.

PUBLIC bool CURS_Calibrate(void);
    // Calibrates the pointer if necessary (if there's a joystick).

PUBLIC word CURS_GetPosition(int *x, int *y);
    // Returns the pointer states, and stores the pointer coordinates.

PUBLIC void CURS_SetPosition(int x, int y);
    // Sets the pointer coordinates.

PUBLIC void CURS_End(void);
    // Ends it all.

#endif

// ----------------------------- CURSOR.H -------------------------------

