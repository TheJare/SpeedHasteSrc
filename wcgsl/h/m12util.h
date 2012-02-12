// ------------------ M12UTILS.H ------------------
// For use with Watcom C.
// (C) Copyright 1995 by Jare & JCAB of Iguana.
// Mode 12h routines

#ifndef _M12UTILS_H_
#define _M12UTILS_H_

#ifndef _BASE_H_
#include <base.h>
#endif

typedef byte TTexture[8*4], *PTexture;

PUBLIC TTexture *TEX_Std;       // TEX_STDMAX

enum {
    TEX_STDMAX = 256,       // All 16x16 mixtures. Yeah, a waste. O:-)
};

PUBLIC void M12_CreateTexture(PTexture tex, int c1, int c2);
PUBLIC bool M12_InitStdTextures(void);

#define TEX(a,b) (TEX_Std + (((a)&15)<<4) + ((b)&15))

// -------------------------------------------

PUBLIC byte (*M12_Font)[8];

PUBLIC bool M12_InitFont(void);

// -------------------------------------------

// extern "C" {

PUBLIC void M12_RectangleWidth(int x0, int y0, int x1, int y1,
                               int width, const TTexture t);

#define M12_Rectangle(x0,y0,x1,y1,t) M12_RectangleWidth(x0,y0,x1,y1,1,t)

PUBLIC void M12_FillRectangle(int x0, int y0, int x1, int y1, const TTexture t);
#pragma aux M12_FillRectangle parm [EAX] [EDX] [EBX] [ECX] [ESI]

#define M12_ClearScreen(t) M12_FillRectangle(0, 0, 640, 480, (t))

PUBLIC void M12_BoxRelief(int x0, int y0, int x1, int y1, int w,
                          PTexture t1, PTexture t2);

PUBLIC void M12_PrepareTexture(int y, const TTexture t);
#pragma aux M12_PrepareTexture parm [EAX] [ESI]

//PUBLIC void M12_DrawLetter(int x, int y, int w, int h, const byte *c);
PUBLIC void M12_DrawLetter(int x, int y, const byte *c);
#pragma aux M12_DrawLetter parm [ECX] [EAX] [EBX]

PUBLIC int M12_Printf(int x, int y, const char *fmt, ...);

#define M12_AreaSize(x0,y0,x1,y1) (4*((y1)-(y0))*((((x1)+7)>>3)-((x0)>>3)))

PUBLIC void M12_SaveArea(int x0, int y0, int x1, int y1, byte *dest);
#pragma aux M12_SaveArea parm [ESI] [EAX] [ECX] [EBX] [EDI]

PUBLIC void M12_DumpArea(int x0, int y0, int x1, int y1, const byte *from);
#pragma aux M12_DumpArea parm [EDI] [EAX] [ECX] [EBX] [ESI]

/*

PUBLIC void M12_DrawBitmap(int x, int y, int w, int h, const byte *bmp);

PUBLIC void M12_DrawSprite(int x, int y, int w, int h, const byte *spr);
*/

PUBLIC void M12_DrawCursor(int x, int y, int w, int h, const byte *cur);
#pragma aux M12_DrawCursor parm [ECX] [EDI] [EDX] [EAX] [EBX]

//}

#endif

/* ------------------ M12UTILS.H ------------------ */

