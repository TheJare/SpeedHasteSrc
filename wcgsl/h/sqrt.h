// --------------------------- SQRT.H ------------------------------
// For use with WATCOM 9.5 + DOS4GW
// (C) Copyright 1995 by Jare & JCAB of Iguana.
// Original code by Jose Juan Garcia Quesada

#ifndef _SQRT_H_
#define _SQRT_H_

#ifndef _BASE_H_
#include <base.h>
#endif

PUBLIC uint32 SQR_Sqrt(uint32 v);
#pragma aux   SQR_Sqrt modify nomemory parm [EAX] value [EAX]

    // This returns the srqt with 16 extra bits of precision.
    // i.e SQR_Sqrt16(1) will be 0x00010000
PUBLIC uint32 SQR_Sqrt16(uint32 v);
#pragma aux   SQR_Sqrt16 modify nomemory parm [EAX] value [EAX]

#endif

// --------------------------- SQRT.H ------------------------------

