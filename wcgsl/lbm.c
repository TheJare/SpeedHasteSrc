// ------------------------------ LBM.C --------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "lbm.h"

#include <string.h>
#include <stdio.h>

typedef struct {
    word w;                     // 320
    word h;                     // 200
    word x;                     // 0
    word y;                     // 0
    byte nplanes;               // 8
    byte masking;               // 0
    byte compression;           // 0
    byte padding;               // 0
    word transparentColor;      // 0
    byte xAspect;               // 5
    byte yAspect;               // 6
    sint16 pageWidth;           // 320
    sint16 pageHeight;          // 200
} LBM_THeader;

PRIVATE void WriteDword(dword a, FILE *f) {
    a = BSwapDword(a);
    fwrite(&a, sizeof(a), 1, f);
}

PRIVATE void WriteName(const char *n, FILE *f) {
    fputc(*n++, f);
    fputc(*n++, f);
    fputc(*n++, f);
    fputc(*n++, f);
}

PUBLIC bool LBM_Write(const char *fname, int width, int height, const byte *pal, const byte *scr) {
    LBM_THeader h;
    dword len, dlen;
    FILE *f;
    int   i;

    memset(&h, 0, sizeof(h));
    f = fopen(fname, "wb");
    if (f == NULL)
        return FALSE;
    len  = width*height;
    dlen = len;
    if ((len & 1) == 1)
        len++;

    WriteName("FORM", f);
    WriteDword(4 + 4 + 4 + sizeof(h) + 4 + 4 + 768 + 4 + 4 + len, f);

    WriteName("PBM ", f);

    h.pageWidth  = BSwapWord(320);
    h.pageHeight = BSwapWord(200);
    h.yAspect = BSwapWord(6);
    h.xAspect = BSwapWord(5);
    h.nplanes = BSwapWord(8);
    h.w = BSwapWord(width);
    h.h = BSwapWord(height);
    WriteName("BMHD", f);
    WriteDword(sizeof(h), f);
    fwrite(&h, sizeof(h), 1, f);

    WriteName("CMAP", f);
    WriteDword(768, f);
    for (i = 0; i < 768; i++)
        fputc(pal[i] << 2, f);

    WriteName("BODY", f);
    WriteDword(len, f);
    fwrite(scr, dlen, 1, f);
    if (len > dlen)
        fputc(0, f);
    fclose(f);
    return TRUE;
}

// --------------------------------

typedef struct {
	byte    manufacturer;
	byte    version;
	byte    encoding;
	byte    bits_per_pixel;
	word    xmin,ymin;
	word    xmax,ymax;
	word    hres;
	word    vres;
	byte    palette[48];
	byte    reserved;
	byte    colour_planes;
	word    bytes_per_line;
	word    palette_type;
	byte    filler[58];
} PCX_THeader;

PUBLIC bool PCX_Write(const char *fname, int width, int height, const byte *pal, const byte *scr) {
    PCX_THeader h;
    FILE *f;
    int   i;

    memset(&h, 0, sizeof(h));
    f = fopen(fname, "wb");
    if (f == NULL)
        return FALSE;

    h.manufacturer = 0x0a;
    h.version      = 5;
    h.encoding     = 1;
    h.bits_per_pixel = 8;
    h.xmin = 0;
    h.ymin = 0;
    h.xmax = width - 1;
    h.ymax = height - 1;
    h.colour_planes  = 1;
    h.bytes_per_line = width;
    h.palette_type   = 1;
    fwrite(&h, sizeof(h), 1, f);

    for (i = 0; i < height; i++) {
        const byte *p;

        p = scr;
        scr += width;
        while (p < scr) {
            int n;

            n = 1;
            while (n < 63 && (p+1) < scr && p[0] == p[1]) {
                n++; p++;
            }
            if (n > 1) {
                fputc(n | 0xC0, f);
                fputc(*p, f);
            } else {
                if ((*p & 0xC0) == 0xC0)
                    fputc(0xC1, f);
                fputc(*p, f);
            }
            p++;
        }
    }
    fputc (0x0C, f);
    for (i = 0; i < 768; i++)
        fputc(pal[i] << 2, f);
    fclose(f);
    return TRUE;
}
