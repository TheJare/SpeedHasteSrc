// ----------------------------- GRAPHICS.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Useful graphics routines.

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#ifndef _BASE_H_
#include <base.h>
#endif

    // Draws a rectangle from (x0, y0), size (w,h).
    // 'interior' and 'border' are the
    // corresonding colors, or -1 for no color to be used.
    // The rectangle is clipped against LLS_SizeX and LLS_SizeY.
PUBLIC void GFX_Rectangle(int x0, int y0, int w, int h,
                          int border, int interior);

#endif

// ----------------------------- GRAPHICS.H -------------------------------

