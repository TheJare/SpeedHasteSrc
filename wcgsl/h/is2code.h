// ------------------------------ IS2CODE.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

// Drawing code is to be made generic someday....

#ifndef _IS2CODE_H_
#define _IS2CODE_H_

#ifndef _BASE_H_
#include <base.h>
#endif

typedef struct {
    word w, h;
    sint16 dx, dy;          // Offset to apply to x, y coords, like a hot spot.
                            // Given from top-left (0,0) corner.
    word  xRatio,
          yRatio;            // Ratio specified when created. 8.8.
    dword flags;
    word offsets[1];        // More than one. Add them to (byte*)&sprite.
} IS2_TSprite, *IS2_PSprite;

enum {
    IS2F_HORIZONTAL = 0x0001,
};

PUBLIC IS2_PSprite IS2_Load(const char *fname);

    // (x,y) are the center coordinates of the sprite. (w,h) are the
    // size. The upper left corner will be at x-w/2, y-h/2.
    // A negative w value will show the sprite horizontally mirrored.
PUBLIC void IS2_DrawVertical(IS2_PSprite sp, int x, int y, int w, int h);

    // (x,y) are the center coordinates of the sprite. Doesn't scale.
PUBLIC void IS2_DrawHorizontal(IS2_PSprite sp, int x, int y);

    // Can tile the sprite data. To actually tile, also set DRW_Tile=TRUE.
PUBLIC void IS2_DrawHorizontalTile(IS2_PSprite sp, int x, int y, int w, int h);

    // Calls either DrawHorizontalTile() or DrawVertical().
PUBLIC void IS2_Draw(IS2_PSprite sp, int x, int y, int w, int h);


PUBLIC void IS2_DrawVTransparent(IS2_PSprite sp, int x, int y, int w, int h);

PUBLIC void IS2_DrawTransparent(IS2_PSprite sp, int x, int y, int w, int h);


#endif

// ------------------------------ IS2CODE.H ----------------------------

