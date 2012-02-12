// ------------------------------ IS2CODE.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

// Drawing code is to be made generic someday....

#include <is2code.h>
#include <vertdraw.h>
#include <hordraw.h>

#include <string.h>
#include <jclib.h>
#include <llscreen.h>

PUBLIC IS2_PSprite IS2_Load(const char *fname) {
    IS2_PSprite sc;
    struct {
        char  sign[4];
        word  w, h;
//        dword flags;
        sint16 dx, dy;
        word  xratio, yratio;
        dword flags;
        dword len;
    } h;
    FILE *f;
    int n;

    f = JCLIB_Open(fname);
    if (f == NULL)
        return NULL;
    if (fread(&h, sizeof(h), 1, f) != 1 || memcmp(h.sign, ".IS2", 4) != 0) {
        JCLIB_Close(f);
        return NULL;
    }
    if (h.flags & IS2F_HORIZONTAL)
        n = h.h;
    else
        n = h.w;
    sc = NEW(sizeof(IS2_TSprite) + (n-1)*sizeof(sc->offsets[0]) + h.len);
    if (sc == NULL) {
        JCLIB_Close(f);
        return NULL;
    }

    sc->w = h.w;
    sc->h = h.h;
    sc->dx = h.dx;
    sc->dy = h.dy;
    sc->xRatio = h.xratio;
    sc->yRatio = h.yratio;
    sc->flags = h.flags;
    fread(sc->offsets + n, h.len, 1, f);
    JCLIB_Close(f);

        // Fill in the offsets[] array.
    {
        int i;
        byte *p;

        p = (byte*)(sc->offsets + n);
        for (i = 0; i < n; i++) {
            sc->offsets[i] = (word)(p - (byte*)sc);
            p++;    // Skip first transparent run (will always exist).
            while (*p != 0) {
                p += *p + 1; // Skip data run.
if (p - (byte*)(sc->offsets + n) >= h.len) {
    breakpoint();
    break;
}
                if (*p == 0)
                    break;
                p++;
            }
            p++;        // Skip end-of-run marker.
        }
    }
    return sc;
}

static void DrawColumns(IS2_PSprite sp, int x, int y, int h, int od, int n)
{
    if (DRW_SetVerticalSpan(x, y, h, sp->h)) {
        if (LLS_SizeX == 640) {
            byte *p = LLS_Screen[0] + (y+DRW_Skip)*LLS_SizeX + x;
            while (n-- > 0) {
                DRW_DoVerticalDraw640(p, ((byte*)sp) + sp->offsets[od]);
                p++;
            }
            return;
        }
        while (n > 3) {
            DRW_DoVerticalDraw4(LLS_Screen[y+DRW_Skip]+x,
                               ((byte*)sp) + sp->offsets[od]);
            n -= 4;
            x += 4;
        }
        switch (n) {
            case 0:
                break;
            case 1:
                DRW_DoVerticalDraw1(LLS_Screen[y+DRW_Skip]+x,
                                   ((byte*)sp) + sp->offsets[od]);
                break;
            case 2:
                DRW_DoVerticalDraw2(LLS_Screen[y+DRW_Skip]+x,
                                   ((byte*)sp) + sp->offsets[od]);
                break;
            case 3:
                DRW_DoVerticalDraw3(LLS_Screen[y+DRW_Skip]+x,
                                   ((byte*)sp) + sp->offsets[od]);
                break;
        }
    }
}

PUBLIC void IS2_DrawVertical(IS2_PSprite sp, int x, int y, int w, int h) {
    int i, n, sw;
    int d, od;
    dword ddapos, ddainc;

    REQUIRE(sp != NULL);

    if (w == 0 || h == 0)
        return;

    if (w < 0) {
        w = -w;
        ddainc = -((((dword)sp->w) << 16) / w);
        ddapos = ((sp->w) << 16) + ((sint32)ddainc / 2) - 1;    // -1 upper bound.
    } else {
        ddainc = (((dword)sp->w) << 16) / w;
        ddapos = ddainc >> 1;
    }
    x -= FPMultDiv(sp->dx, w, sp->w);
    y -= FPMultDiv(sp->dy, h, sp->h);

    DRW_Tile = FALSE;

    if (x < DRW_MinX) {
        i = DRW_MinX - x;
        ddapos += i*ddainc;
    } else
        i = 0;
    if (x + w >= DRW_MaxX)
        sw = DRW_MaxX - x;
    else
        sw = w;

    od = ddapos>>16;
    n  = 0;
    for (; i < sw; i++) {
        d = ddapos>>16;
        if (od != d || (DRW_VSpans != NULL
         && *(dword *)(DRW_VSpans[x+i]) != *(dword *)(DRW_VSpans[x+i-1]))) {
            DrawColumns(sp, x+i-n, y, h, od, n);
            od = d;
            n  = 1;
        } else {
            n++;
        }

        ddapos += ddainc;
    }
    if (n != 0)
        DrawColumns(sp, x+i-n, y, h, od, n);
}

PUBLIC void IS2_DrawHorizontal(IS2_PSprite sp, int x, int y) {
    IS2_DrawHorizontalTile(sp, x, y, sp->w, sp->h);
}

PUBLIC void IS2_DrawHorizontalTile(IS2_PSprite sp, int x, int y, int w, int h) {
    byte *p, *s;
    word *o;
    int   i, skip;

    x -= sp->dx;
    y -= sp->dy;
    if (x >= DRW_MaxX || y >= DRW_MaxY || (x+w) <= DRW_MinX || (y+h) <= DRW_MinY)
        return;
    if (y+h > DRW_MaxY)
        h = DRW_MaxY - y;
    if (y < DRW_MinY) {
        o = sp->offsets + (DRW_MinY-y);
        h -= (DRW_MinY - y);
        y = DRW_MinY;
    } else {
        o = sp->offsets;
    }
    skip = DRW_MinX - x;
    if (skip > 0) {
        s = LLS_Screen[0]+LLS_SizeX*y + DRW_MinX;
    } else {
        skip = 0;
        s = LLS_Screen[0]+LLS_SizeX*y + x;
    }
    if (x+w > DRW_MaxX) w = (DRW_MaxX - x);
    for (i = 0; i < h; i++) {
        p = ((byte*)sp) + *o;
        DRW_DoHorizontalDraw(s, p, skip, w);
        s += LLS_SizeX;
        o++;
    }
}

PUBLIC void IS2_Draw(IS2_PSprite sp, int x, int y, int w, int h) {
    if (sp == NULL)
        return;
    if (sp->flags & IS2F_HORIZONTAL)
        IS2_DrawHorizontalTile(sp, x, y, w, h);
    else
        IS2_DrawVertical(sp, x, y, w, h);
}


PUBLIC void IS2_DrawVTransparent(IS2_PSprite sp, int x, int y, int w, int h) {
    int i, n, sw;
    int d;
    dword ddapos, ddainc;
    byte *p;

    REQUIRE(sp != NULL);

    if (w == 0 || h == 0)
        return;

    if (w < 0) {
        w = -w;
        ddainc = -((((dword)sp->w) << 16) / w);
        ddapos = ((sp->w) << 16) + ((sint32)ddainc / 2) - 1;    // -1 upper bound.
    } else {
        ddainc = (((dword)sp->w) << 16) / w;
        ddapos = ddainc >> 1;
    }
    x -= FPMultDiv(sp->dx, w, sp->w);
    y -= FPMultDiv(sp->dy, h, sp->h);

    DRW_Tile = FALSE;

    if (x < DRW_MinX) {
        i = DRW_MinX - x;
        ddapos += i*ddainc;
        x = DRW_MinX;
    } else
        i = 0;
    if (x + w >= DRW_MaxX)
        sw = DRW_MaxX - x;
    else
        sw = w;

    n  = 0;
    for (; i < sw; i++) {
        if (DRW_SetVerticalSpan(x, y, h, sp->h)) {
            p = LLS_Screen[0] + (y+DRW_Skip)*LLS_SizeX + x;
            d = ddapos >> 16;
            if (LLS_SizeX == 640) {
                DRW_DoTransparentDraw640(p, ((byte*)sp) + sp->offsets[d]);
            } else {
                DRW_DoTransparentDraw(p, ((byte*)sp) + sp->offsets[d]);
            }
        }
        x++; //p++;
        ddapos += ddainc;
    }
}


PUBLIC void IS2_DrawTransparent(IS2_PSprite sp, int x, int y, int w, int h) {
    if (sp == NULL)
        return;
    if (sp->flags & IS2F_HORIZONTAL)
        IS2_DrawHorizontalTile(sp, x, y, w, h);
    else
        IS2_DrawVTransparent(sp, x, y, w, h);
}

// ------------------------------ IS2CODE.C ----------------------------
