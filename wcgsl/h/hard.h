// ----------------------------- HARD.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

// Low level stuff.

#ifndef _HARD_H_
#define _HARD_H_

#ifndef _BASE_H_
#include <base.h>
#endif

// ------------ IRQ handling.
// IRQ # is from 0 to 15. IRQs 8-15 are redirected automatically (if the
// extender supports it).

typedef void (__interrupt __far *HARD_TIRQHandler)(void);

PUBLIC HARD_TIRQHandler HARD_GetIRQVector(int n);
PUBLIC void             HARD_SetIRQVector(int n, HARD_TIRQHandler vec);

#endif

// ----------------------------- HARD.H -------------------------------

