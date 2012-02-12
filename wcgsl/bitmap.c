// ----------------------------- BITMAP.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#include <bitmap.h>
#include <llscreen.h>
#include <string.h>
#include <jclib.h>

//#undef LLS_Screen
//#define LLS_Screen VGA200x320

BM_TBitmap *BM_Make(int w, int h, dword flags, const char *fname) {
    BM_TBitmap *sc;

    if (w <= 0 || h <= 0)
        return NULL;
    sc = NEW(sizeof(BM_TBitmapHeader) + w*h);
    if (sc == NULL)
        return NULL;
    sc->Header.Width  = w;
    sc->Header.Height = h;
    sc->Header.Flags  = flags;
    if (fname != NULL)
        if (JCLIB_Load(fname, sc->Data, w*h) != w*h)
            DISPOSE(sc);
    return sc;
}

BM_TBitmap *BM_Load(dword flags, const char *fname) {
    BM_TBitmap *sc;
    struct {
        char  sign[4];
        word  w, h;
        dword flags;
        dword len;
    } h;
    FILE *f;

    f = JCLIB_Open(fname);
    if (f == NULL)
        return NULL;
    if (fread(&h, sizeof(h), 1, f) != 1 || memcmp(h.sign, ".ISP", 4) != 0) {
        JCLIB_Close(f);
        return NULL;
    }
    sc = NEW(sizeof(BM_TBitmapHeader) + h.len);
    if (sc == NULL) {
        JCLIB_Close(f);
        return NULL;
    }

    sc->Header.Width  = h.w;
    sc->Header.Height = h.h;
    sc->Header.Flags  = flags;
    fread(sc->Data, h.len, 1, f);
    JCLIB_Close(f);
    return sc;
}

void BM_Draw(int x, int y, BM_TBitmap *bm) {
    int i, j;
    byte *p, *g;
    if (bm == NULL)
        return;
    if (x >= LLS_SizeX || (x+bm->Header.Width)  <= 0 ||
        y >= LLS_SizeY || (y+bm->Header.Height) <= 0)
        return;

    if (x < 0) x = 0;
    if (x+bm->Header.Width > LLS_SizeX) x = LLS_SizeX-bm->Header.Width;
    if (y < 0) y = 0;
    if (y+bm->Header.Height > LLS_SizeY) y = LLS_SizeY-bm->Header.Height;

    if (LLS_VMode == LLSVM_MODE13) {
        p = &(LLS_Screen[y][x]);
        g = bm->Data;
        if (bm->Header.Flags & BMF_SPRITE) {
            for (i = 0; i < bm->Header.Height; i++) {
                for (j = 0; j < bm->Header.Width; j++) {
                    if (*g != 0)
                        *p = *g;
                    g++;
                    p++;
                }
                p += 320-bm->Header.Width;
            }
        } else {
            for (i = 0; i < bm->Header.Height; i++) {
                memcpy(p, g, bm->Header.Width);
                p += 320;
                g += bm->Header.Width;
            }
        }
    } else {        // Planar mode.
        int k, pl;
        byte *gp, *pp;
        pp = (byte *)LLS_Screen + (LLS_SizeX/4)*y + x/4;
        gp = bm->Data;
        if (LLS_Mode != LLSM_DIRECT)
            pl = (LLS_Size/4)*(x%4);
        else
            pl = 1 << (x%4);
        for (k = 0; k < 4; k++) {
            p = pp;
            g = gp;
            if (LLS_Mode != LLSM_DIRECT)
                p += pl;
            else
                VGA_SetPlanes(pl);
            if (bm->Header.Flags & BMF_SPRITE) {
                for (i = 0; i < bm->Header.Height; i++) {
                    for (j = 0; j < bm->Header.Width; j += 4) {
                        if (*g != 0)
                            p[j/4] = *g;
                        g += 4;
                    }
                    g = (g-4) + ((bm->Header.Width-1) & 3) + 1;
                    p += LLS_SizeX/4;
                }
            } else {
                for (i = 0; i < bm->Header.Height; i++) {
                    for (j = 0; j < bm->Header.Width; j += 4) {
                        p[j/4] = *g;
                        g += 4;
                    }
                    g = (g-4) + ((bm->Header.Width-1) & 3) + 1;
                    p += LLS_SizeX/4;
                }
            }
            if (LLS_Mode != LLSM_DIRECT) {
                if ( (pl = (pl + LLS_Size/4)) == LLS_Size) {
                    pl = 0;
                    pp++;
                }
            } else
                if ( (pl <<= 1) == 0x10) {
                    pl = 1;
                    pp++;
                }
            gp++;
        }
    }
}

void BM_Get(int x, int y, BM_TBitmap *bm) {
    int i, j;
    byte *p, *g;
    if (bm == NULL)
        return;

    bm->Header.Flags &= ~BMF_SPRITE;
    if (   x >= LLS_SizeX || (x+bm->Header.Width)  <= 0
        || y >= LLS_SizeY || (y+bm->Header.Height) <= 0) {
        memset(bm->Data, 0, bm->Header.Width*bm->Header.Height);
        return;
    }

    if (x < 0) x = 0;
    if (x+bm->Header.Width > LLS_SizeX) x = LLS_SizeX-bm->Header.Width;
    if (y < 0) y = 0;
    if (y+bm->Header.Height > LLS_SizeY) y = LLS_SizeY-bm->Header.Height;

    p = &(LLS_Screen[y][x]);
    g = bm->Data;
    if (LLS_VMode == LLSVM_MODE13) {
        p = &(LLS_Screen[y][x]);
        g = bm->Data;
        for (i = 0; i < bm->Header.Height; i++) {
            memcpy(g, p, bm->Header.Width);
            p += 320;
            g += bm->Header.Width;
        }
    } else {        // Planar mode.
        int k, pl;
        byte *gp, *pp;
        pp = (byte *)LLS_Screen + (LLS_SizeX/4)*y + x/4;
        gp = bm->Data;
        if (LLS_Mode != LLSM_DIRECT)
            pl = (LLS_Size/4)*(x%4);
        else
            pl = x%4;
        for (k = 0; k < 4; k++) {
            p = pp;
            g = gp;
            if (LLS_Mode != LLSM_DIRECT)
                p += pl;
            else
                VGA_SetReadPlane(pl);
            if (bm->Header.Flags & BMF_SPRITE) {
                for (i = 0; i < bm->Header.Height; i++) {
                    for (j = 0; j < bm->Header.Width; j += 4) {
                        if (*g != 0)
                            *g = p[j/4];
                        g += 4;
                    }
                    g = (g-4) + ((bm->Header.Width-1) & 3) + 1;
                    p += LLS_SizeX/4;
                }
            } else {
                for (i = 0; i < bm->Header.Height; i++) {
                    for (j = 0; j < bm->Header.Width; j += 4) {
                        *g = p[j/4];
                        g += 4;
                    }
                    g = (g-4) + ((bm->Header.Width-1) & 3) + 1;
                    p += LLS_SizeX/4;
                }
            }
            if (LLS_Mode != LLSM_DIRECT) {
                if ( (pl = (pl + LLS_Size/4)) == LLS_Size) {
                    pl = 0;
                    pp++;
                }
            } else
                if ( (++pl) == 4) {
                    pl = 0;
                    pp++;
                }
            gp++;
        }
    }
}

// ----------------------------- BITMAP.C -------------------------------

