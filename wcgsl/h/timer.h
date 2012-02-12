// ------------------------------ TIMER.H ----------------------------
// For use with Watcom C 9.5 and DOS4GW
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _TIMER_H_
#define _TIMER_H_

#ifndef _BASE_H_
#include <base.h>
#endif

    // --------------------------

PUBLIC volatile dword TIMER_Clock;
PUBLIC void (*TIMER_HookFunction)(void);

    // --------------------------

#define TIMER_70HZ ((dword)(65536.0*18.2/70.0))
#define TIMER_35HZ ((dword)(65536.0*18.2/35.0))

    // --------------------------

PUBLIC void TIMER_Init(dword speed);

PUBLIC void TIMER_SetSpeed(dword speed);

PUBLIC void TIMER_End(void);

#endif

// ------------------------------ TIMER.H ----------------------------

