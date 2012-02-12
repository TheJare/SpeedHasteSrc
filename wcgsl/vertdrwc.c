// ------------------------------ VERTDRWC.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include <vertdraw.h>

bool  DRW_Tile = FALSE;      // Shall we tile the texture?
const byte * DRW_TranslatePtr;  // Pointer to palette translation table.

    // Variables that communicate with the ASM code; should be set by
    // the module routines, not assigned by the user.

int   DRW_Skip,      // Pixels on screen to skip before starting to draw.
      DRW_Height;    // Height of data to actually draw (after clipping).
dword DRW_DDAStart,  // DDA starting value, 16.16 after skipping.
      DRW_DDAInc,    // DDA increment per pixel on screen, 16.16.
      DRW_DDAIncInv; // 1/DDAInc.

        // Clipping info. If VSpans == NULL, use MinY and MaxY always.
int   DRW_MinX,
      DRW_MaxX,
      DRW_MinY,
      DRW_MaxY;
sint16 (*DRW_VSpans)[2];

// ==========================================================

PUBLIC void DRW_SetClipZone(int minx, int miny, int maxx, int maxy,
                            sint16 vspans[][2]) {
    DRW_MinX   = minx;
    DRW_MaxX   = maxx;
    DRW_MinY   = miny;
    DRW_MaxY   = maxy;
    DRW_VSpans = vspans;
}

PUBLIC void DRW_SetDDAData(int skip, int height,
                           dword ddastart, dword ddainc) {    // 16.16
    DRW_Skip      = skip;
    DRW_Height    = height;
    DRW_DDAStart  = ddastart;
    DRW_DDAInc    = ddainc;
    DRW_DDAIncInv = FP16Inverse(ddainc);
}

PUBLIC bool DRW_SetVerticalSpan(int x, int y, int hscr, int hdata) {
    int skip, h;
    dword dda;

    if (hscr <= 0 || x < DRW_MinX || x >= DRW_MaxX)
        return FALSE;

    x -= DRW_MinX;
    if (DRW_VSpans == NULL) {
        skip = DRW_MinY - y;
        if (skip < 0) {
            skip = 0;
        } else if (skip >= hscr)
            return FALSE;
        if (y + hscr <= DRW_MinY || y + skip >= DRW_MaxY)
            return FALSE;
        if (y + hscr >= DRW_MaxY)
            h = DRW_MaxY - y - skip;
        else
            h = hscr - skip;
    } else {
        skip = DRW_VSpans[x][0] - y;
        if (skip < 0) {
            skip = 0;
        } else if (skip >= hscr)
            return FALSE;
        if (y + hscr <= DRW_VSpans[x][0] || y + skip >= DRW_VSpans[x][1])
            return FALSE;
        if (y + hscr >= DRW_VSpans[x][1])
            h = DRW_VSpans[x][1] - y - skip;
        else
            h = hscr - skip;
    }

        // Now we know there's something to draw.
    dda = (((dword)hdata) << 16) / hscr;
    if (dda < 4)
        return FALSE;
    DRW_SetDDAData(skip, h, (dda>>1) + dda*skip, dda);
    return TRUE;
}

PUBLIC bool DRW_SetVerticalSpanFP(int x, int y, int hscr, int hdata) {
    int skip, h, hs;
    dword dda, ddas;

    if (hscr <= 0 || x < DRW_MinX || x >= DRW_MaxX)
        return FALSE;

    hs = ((y+hscr)>>16) - (y>>16);

    x -= DRW_MinX;
    if (DRW_VSpans == NULL) {
        skip = DRW_MinY - (y>>16);
        if (skip < 0) {
            skip = 0;
        } else if (skip >= hs)
            return FALSE;
        if ((y>>16) + hs <= DRW_MinY || (y>>16) + skip >= DRW_MaxY)
            return FALSE;
        if ((y>>16) + hs >= DRW_MaxY)
            h = DRW_MaxY - (y>>16) - skip;
        else
            h = hs - skip;
    } else {
        skip = DRW_VSpans[x][0] - (y>>16);
        if (skip < 0) {
            skip = 0;
        } else if (skip >= hs)
            return FALSE;
        if ((y>>16) + hs <= DRW_VSpans[x][0] || (y>>16) + skip >= DRW_VSpans[x][1])
            return FALSE;
        if ((y>>16) + hs >= DRW_VSpans[x][1])
            h = DRW_VSpans[x][1] - (y>>16) - skip;
        else
            h = hs - skip;
    }

        // Now we know there's something to draw.
    if ((hdata>>16) >= hscr || (skip > 0 && FP16Mult(hdata, skip) >= hscr)) {
        dda  = 0x10000;
        ddas = 0x00000;
    } else {
        dda  = FP16Div(hdata, hscr);
        ddas = FP16Mult(0xFFFF -(y&0x0000FFFF), dda) + FPMultDiv(hdata, skip<<16, hscr);
    }

    DRW_SetDDAData(skip, h, ddas, dda);

    return TRUE;
}

// ------------------------------ VERTDRWC.C ----------------------------

