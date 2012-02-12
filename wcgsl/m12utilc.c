// ------------------ M12UTILS.C ------------------
// For use with Watcom C.
// (C) Copyright 1995 by Jare & JCAB of Iguana.
// Mode 12h routines

#include "m12util.h"
#include <dpmi.h>
#include <stdarg.h>
#include <stdio.h>

PUBLIC TTexture *TEX_Std = NULL;

enum {
    XMIN = 0,
    XMAX = 640,
    YMIN = 0,
    YMAX = 819   // 480, why restrict?
};

PUBLIC void M12_CreateTexture(PTexture tex, int c1, int c2) {
    byte j, k1, k2;

    assert(tex != NULL);
    k1 = c1;
    k2 = c2;
    for (j = 1; j < 16; j <<= 1, tex++) {
        byte b1, b2;

        b1 = ((k1 & 1) << 1) | (k2 & 1);
        b1 = b1 | (b1 << 2) | (b1 << 4) | (b1 << 6);
        b2 = ((k2 & 1) << 1) | (k1 & 1);
        b2 = b2 | (b2 << 2) | (b2 << 4) | (b2 << 6);
        tex[4*0] = b1; tex[4*2] = b1; tex[4*4] = b1; tex[4*6] = b1;
        tex[4*1] = b2; tex[4*3] = b2; tex[4*5] = b2; tex[4*7] = b2;
        k1 >>= 1;
        k2 >>= 1;
    }
}

PUBLIC bool M12_InitStdTextures(void) {
    int i;

    if (TEX_Std != NULL)
        return TRUE;
    TEX_Std = NEW(TEX_STDMAX*sizeof(TEX_Std[0]));
    if (TEX_Std == NULL)
        return FALSE;
    for (i = 0; i < TEX_STDMAX; i++) {
        M12_CreateTexture(TEX_Std + i, i & 15, i >> 4);
    }
    return TRUE;
}


// -------------------------------------------

PUBLIC byte (*M12_Font)[8] = NULL;

PUBLIC bool M12_InitFont(void) {
    if (M12_Font != NULL)
        return TRUE;
    DPMI_rmi.w.AX = 0x1130;
    DPMI_rmi.b.BH = 3;
    DPMI_RealModeInt(0x10);
    M12_Font = (void*)DPMI_MK_PTR(DPMI_rmi.w.ES, DPMI_rmi.w.BP);
    return TRUE;
}


PUBLIC void M12_RectangleWidth(int x0, int y0, int x1, int y1,
                               int width, const TTexture t) {
    int i;

    if (x0 > x1) {
        i=x0;x0=x1;x1=i;
    }
    if (y0 > y1) {
        i=y0;y0=y1;y1=i;
    }
    M12_FillRectangle(x0, y0, x0+width, y1,       t);
    M12_FillRectangle(x0, y1, x1,       y1-width, t);
    M12_FillRectangle(x1, y1, x1-width, y0,       t);
    M12_FillRectangle(x1, y0, x0,       y0+width, t);
}

PUBLIC void M12_BoxRelief(int x0, int y0, int x1, int y1, int w,
                          PTexture t1, PTexture t2) {
    int i;
    if (w > 0) {
        for (i = 0; i < w; i++) {
            M12_FillRectangle(x0, y0+i, x1-i, y0+1+i, t1);
            M12_FillRectangle(x0+i, y0, x0+i+1, y1-i, t1);
            M12_FillRectangle(x0+i+1, y1-i, x1, y1-i-1, t2);
            M12_FillRectangle(x1-i, y0+i+1, x1-i-1, y1, t2);
        }
    } else {
        w = -w;
        for (i = 0; i < w; i++) {
            M12_FillRectangle(x0, y0+i, x1-i, y0+1+i, t2);
            M12_FillRectangle(x0+i, y0, x0+i+1, y1-i, t2);
            M12_FillRectangle(x0+i+1, y1-i, x1, y1-i-1, t1);
            M12_FillRectangle(x1-i, y0+i+1, x1-i-1, y1, t1);
        }
    }
}



PUBLIC int M12_Printf(int x, int y, const char *fmt, ...) {
    unsigned char buf[300];
    va_list list;
    int i;

    va_start(list, fmt);
    buf[0] = '\0';
    vsprintf(buf, fmt, list);
    for (i = 0; buf[i] != '\0'; i++) {
        if (buf[i] != ' ')
            M12_DrawLetter(x, y, M12_Font[buf[i]]);
        x += 8;
    }
    return x;
}

// ------------------ M12UTILS.C ------------------

