// ------------------------------ VERTDRAW.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _VERTDRAW_H_
#define _VERTDRAW_H_

#ifndef _BASE_H_
#include <base.h>
#endif

    // Variables that the user can set to fit specific needs.
PUBLIC bool        DRW_Tile;            // Shall we tile the texture?
PUBLIC const byte *DRW_TranslatePtr;    // Pointer to palette translation table.

    // Variables that communicate with the ASM code; should be set by
    // the module routines, not assigned by the user.

PUBLIC int   DRW_Skip,      // Pixels on screen to skip before starting to draw.
             DRW_Height;    // Height of data to actually draw (after clipping).
PUBLIC dword DRW_DDAStart,  // DDA starting value, 16.16.
             DRW_DDAInc,    // DDA increment per pixel on screen, 16.16.
             DRW_DDAIncInv; // 1/DDAInc.

        // Clipping info. If VSpans == NULL, use MinY and MaxY always.
PUBLIC int DRW_MinX,
           DRW_MaxX,
           DRW_MinY,
           DRW_MaxY;
PUBLIC sint16 (*DRW_VSpans)[2];

    // Various routines to setup drawing parameters.

PUBLIC void DRW_SetClipZone(int minx, int miny, int maxx, int maxy,
                            sint16 vspans[][2]);
    // vspans holds (maxx-minx) pairs of (miny, maxy) for irregular clip.

PUBLIC bool DRW_SetVerticalSpan(int x, int y, int hscr, int hdata);
    // For easy drawing into the set clipzone. Returns TRUE if there
    // should happen any drawing, else DO NOT call DRW_DoVerticalDraw().

PUBLIC bool DRW_SetVerticalSpanFP(int x, int y, int hscr, int hdata);
    // Like SetVerticalSpan, but 'y' and 'hdata' are 16.16 fixed point.
    // This is used when accuracy in the DDA is needed.

PUBLIC void DRW_SetDDAData(int skip, int height,
                           dword ddastart, dword ddainc);     // 16.16
    // Lower level info for drawing without taking clipzone into account.

    // Actual drawing routine.
extern void DRW_DoVerticalDraw(byte *dest, const byte *data);
#pragma aux DRW_DoVerticalDraw parm [EDI] [ESI]
extern void DRW_DoVerticalDraw1(byte *dest, const byte *data);
#pragma aux DRW_DoVerticalDraw1 parm [EDI] [ESI]
extern void DRW_DoVerticalDraw2(byte *dest, const byte *data);
#pragma aux DRW_DoVerticalDraw2 parm [EDI] [ESI]
extern void DRW_DoVerticalDraw3(byte *dest, const byte *data);
#pragma aux DRW_DoVerticalDraw3 parm [EDI] [ESI]
extern void DRW_DoVerticalDraw4(byte *dest, const byte *data);
#pragma aux DRW_DoVerticalDraw4 parm [EDI] [ESI]

extern void DRW_DoVerticalDrawMX(byte *dest, const byte *data);
#pragma aux DRW_DoVerticalDrawMX parm [EDI] [ESI]

extern void DRW_DoVerticalDraw640(byte *dest, const byte *data);
#pragma aux DRW_DoVerticalDraw640 parm [EDI] [ESI]

extern void DRW_DoTransparentDraw(byte *dest, const byte *data);
#pragma aux DRW_DoTransparentDraw parm [EDI] [ESI]

extern void DRW_DoTransparentDraw640(byte *dest, const byte *data);
#pragma aux DRW_DoTransparentDraw640 parm [EDI] [ESI]


#endif

// ------------------------------ VERTDRAW.H ----------------------------

