// ----------------------------- BITMAP.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#ifndef _BITMAP_H_
#define _BITMAP_H_

#ifndef _BASE_H_
#include <base.h>
#endif

enum {
    BM_TRANSPARENT = 0,         // Transparent color.

        // Bitmap flags.
    BMF_SPRITE = 0x0001,        // Bitmap has transparent pixels.
};

// ---------------------------------------------

typedef struct {
    word  Width, Height;
    dword Flags;            // Combination of BMF_xxx constants.
} BM_TBitmapHeader;
 /****************/

    // Skeleton for bitmaps. When you know the size of the bitmap,
    // use dynamic memory and use something like :
    // MyBitmap = (BM_TBitmap)malloc(TBitmapHeader + width*height);
typedef struct {
    BM_TBitmapHeader Header;
    byte             Data[1];   // May be any size, no static TBitmaps.
} BM_TBitmap, *BM_PBitmap;
 /**********/

    // malloc()'ates a bitmap of the given params, and if fname != NULL,
    // loads the raw data in from a file (usually a .PIX).
PUBLIC BM_TBitmap *BM_Make(int w, int h, dword flags, const char *fname);

    // Loads a full .ISP file, malloc()'ing the bitmap and sets the given
    // flags.
PUBLIC BM_TBitmap *BM_Load(dword flags, const char *fname);

    // Draws the bitmap to the screen.
PUBLIC void BM_Draw(int x, int y, BM_TBitmap *bm);

    // Captures the bitmap from screen.
PUBLIC void BM_Get(int x, int y, BM_TBitmap *bm);

#endif

// ----------------------------- BITMAP.H -------------------------------

