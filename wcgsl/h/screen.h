// ----------------------------- SCREEN.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#ifndef _SCREEN_H_
#define _SCREEN_H_

#ifndef _BASE_H_
#include <base.h>
#endif

enum {
    SCRC_NORMAL_FIRST  =   0,
    SCRC_NORMAL_LAST   =  15,
    SCRC_CUSTOM_FIRST  =  16,
    SCRC_CUSTOM_LAST   = 199,
    SCRC_SPECIAL_FIRST = 200,
    SCRC_SPECIAL_LAST  = 254,
    SCRC_RESERVED      = 255,
};



#endif

// ----------------------------- SCREEN.H -------------------------------

