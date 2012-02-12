// ----------------------------- GRAPHICS.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Useful graphics routines.

#include <graphics.h>
#include <llscreen.h>
#include <string.h>
#include <stdlib.h>

void GFX_Rectangle(int x0, int y0, int w, int h,
                   int border, int interior) {
    byte *p;
    w += x0;
    h += y0;
    if (x0 > w)
        swap(x0,w);
    if (y0 > h)
        swap(y0,h);
    if (x0 >= LLS_SizeX || w <= 0 || y0 >= LLS_SizeY || h <= 0
        || x0 == w || y0 == h)
        return;
    if (x0 < 0)
        x0 = 0;
    if (y0 < 0)
        y0 = 0;
    w -= x0;
    h -= y0;
    if (w < 3 || h < 3)
        interior = -1;
    if (interior < 0 && border < 0)
        return;

    p = LLS_Screen[0] + LLS_SizeX*y0 + x0;
    if (border >= 0)
        memset(p, border, w);
    else
        memset(p, interior, w);
    p += LLS_SizeX;
    if (border >= 0) {
        while (h-- > 2) {
            p[0] = p[w-1] = border;
            if (interior >= 0)
                memset (p + 1, interior, w-2);
            p += LLS_SizeX;
        }
    } else {
        while (h-- > 2) {
            memset(p, interior, w);
            p += LLS_SizeX;
        }
    }
    if (border >= 0)
        memset(p, border, w);
    else if (interior >= 0)
        memset(p, interior, w);
}

// ----------------------------- GRAPHICS.C -------------------------------

