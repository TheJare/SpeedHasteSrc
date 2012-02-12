// ----------------------------- TEXT.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#include <text.h>
#include <jclib.h>
#include <llscreen.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

  // Font handling.

bool FONT_Load(FONT_TFont *font, const char *fname) {
    FILE *f;
    struct {
        word flags;
        byte height,
             width;
        dword len;
    } fHdr;

    assert(font != NULL);
    font->data = NULL;
    font->offs = NULL;
    f = JCLIB_Open(fname);
    if (f == NULL)
        return FALSE;
    fread(&fHdr, sizeof(fHdr), 1, f);
    font->flags  = fHdr.flags;
    font->width  = fHdr.width;
    font->height = fHdr.height;
    font->data = NEW(fHdr.len);
    if (font->data == NULL)
        return FALSE;
    fread(font->data, fHdr.len, 1, f);
    JCLIB_Close(f);
    if (font->width == 0) {
        word off, *poff;
        int nchars;

        if (font->flags & FONT_FALPHA)
            nchars = ('z'-'a'+1)*2 + 10;
        else if (font->flags & FONT_F128)
            nchars = 128-' ';
        else if (font->flags & FONT_CHRMAP) {
            int k;
            nchars = 0;
            for (k = 0; k < 256; k++)
                if (font->data[k] != 255)
                    nchars++;
        } else
            nchars = 256-' ';

        font->offs = NEW(sizeof(*font->offs)*nchars);
        if (font->offs == NULL) {
            DISPOSE(font->data);
            return FALSE;
        }
        poff = font->offs;
        if (font->flags & FONT_CHRMAP)
            off = 256;  // Skip char map.
        else
            off = 0;
        while (nchars-- > 0) {
            *poff++ = off;
/*
            if (off >= fHdr.len) {
                char buf[300];
                sprintf(buf, "font: %s, off: %d, nchars: %d, len: %d\n",
                        fname, off, nchars, fHdr.len);
                BASE_Require(buf, __FILE__, __LINE__);
            }
*/
            off = off + (font->height*font->data[off] + 1);
        }
    } else
        font->offs = NULL;
    return TRUE;
}

PUBLIC void FONT_End(FONT_TFont *font) {
    if (font == NULL)
        return;
    DISPOSE(font->data);
    DISPOSE(font->offs);
}

FONT_TFont FONT_System;
FONT_TFont FONT_Big;
FONT_TFont FONT_Mini;
FONT_TFont FONT_Border;

PRIVATE int getNum(const char **str) {
    int v = 0;
    byte c;

    while ( (c = *((*str)++)) != '\0')
        v = v*10 + c - '0';
    return v;
}

PUBLIC int FONT_Index(FONT_TFont *font, int c, byte **pc) {
    if (font->flags & FONT_CHRMAP) {
        if (font->data[(byte)c] == 255)
            c = toupper(c);
        c = font->data[(byte)c];
        if (c == 255)
            c = -1;
    } else if (font->flags & FONT_FALPHA) {
        if (isupper(c))
            c = c - 'A';
        else if (islower(c))
            c = c - 'a' + ('Z' - 'A' + 1);
        else if (isdigit(c))
            c = c - '0' + 2*('Z' - 'A' + 1);
        else
            c = -1;
    } else if (font->flags & FONT_F128) {
        if (c == ((byte)'¤') || c == ((byte)'¥'))
            c = 128;
        else if (c >= 128)
            c = ' ';
        c = c - ' ' - 1;
    } else {
/*
        if (c == ' ')
            c = -1;
        else
*/
            c = c - ' ';
    }

    if (c < 0) {            // Is it the space?
        *pc = NULL;
        return font->height / 2;
    }
    if (font->width == 0) {
        *pc = font->data + font->offs[c] + 1;
        return font->data[font->offs[c]];
    } else {
        *pc = font->data + font->width*font->height*c;
        return font->width;
    }
}

int TEXT_Write(const FONT_TFont *font, int x, int y, const char *str, int ink) {
    int lmar = x, rmar = LLS_SizeX;
    int i, j, width, border = 0;
    byte c;
    byte *pc, *sc;

    assert(font != NULL);
    if (str == NULL || font->data == NULL)
        return x;

    while ( (c = (byte)*str++) != '\0') {
        if (c == TXC_PREFIXC) {
            c = (byte)*str++;
            switch(c) {
                case TXC_CRC:
                    x = lmar;
                    continue;
                case TXC_LFC:
                    y += font->height;
                    continue;
                case TXC_F8C:
                    continue;
                case TXC_F6C:
                    continue;
                case TXC_F4C:
                    continue;
                case TXC_ATC:
                    x = getNum(&str);
                    y = getNum(&str);
                    rmar = LLS_SizeX;
                    lmar = x;
                    continue;
                case TXC_INKC:
                    ink = getNum(&str);
                    continue;
                case TXC_BORDERC:
                    border = getNum(&str);
                    continue;
                case TXC_RMC:
                    rmar = getNum(&str);
                    continue;
                case TXC_LMC:
                    lmar = getNum(&str);
                    continue;
                default:
                    ;
            }
        }
            // Find char dimensions (and, conveniently, addr).
        width = FONT_Index(font, c, &pc);
        if (x > rmar - width) {     // Would go out of area? Next line.
            x = lmar;
            y += font->height;
        }
        if (pc != NULL && x <= (LLS_SizeX - width) && y <= (LLS_SizeY - font->height)) {
            sc = ((byte*)LLS_Screen) + LLS_SizeX*y + x;

            if (font->width == 0) {
                for (i = 0; i < width; i++) {
                    byte k;
                    for (j = 0; j < font->height; j++) {
                        if ( (k = *pc++) == 255)
                            *sc = ink;
                        else if (k == 254)
                            *sc = border;
                        else if (k != 0)
                            *sc = k;
                        sc += LLS_SizeX;
                    }
                    sc += 1 - font->height*LLS_SizeX;
                }
                if ((font->flags & (FONT_FTIGHT|FONT_CHRMAP)) == 0)
                    x++;
            } else {
                for (i = 0; i < font->height; i++) {
                    byte k;
                    for (j = 0; j < font->width; j++) {
                        if ( (k = *pc++) == 255)
                            *sc++ = ink;
                        else if (k == 254)
                            *sc++ = border;
                        else if (k == 0)
                            sc++;
                        else
                            *sc++ = k;
                    }
                    sc += LLS_SizeX - font->width;
                }
            }
        }
        x += width;
    }
    return x;
}


int  TEXT_GetExtent(const FONT_TFont *font, int x, int y, const char *str, int *w, int *h) {
    int lmar = x, rmar = LLS_SizeX;
    int width, border = 0;
    byte c;
    byte *pc;
    int sx, sy;
    sx = x;
    sy = y;

    assert(font != NULL);
    if (w != NULL)
        *w = 0;
    if (h != NULL)
        *h = 0;
    if (str == NULL || font->data == NULL)
        return x;

    while ( (c = (byte)*str++) != '\0') {
        if (c == TXC_PREFIXC) {
            c = (byte)*str++;
            switch(c) {
                case TXC_CRC:
                    x = lmar;
                    continue;
                case TXC_LFC:
                    y += font->height;
                    continue;
                case TXC_F8C:
                    continue;
                case TXC_F6C:
                    continue;
                case TXC_F4C:
                    continue;
                case TXC_ATC:
                    x = getNum(&str);
                    y = getNum(&str);
                    rmar = LLS_SizeX;
                    lmar = x;
                    continue;
                case TXC_INKC:
                    getNum(&str);
                    continue;
                case TXC_BORDERC:
                    border = getNum(&str);
                    continue;
                case TXC_RMC:
                    rmar = getNum(&str);
                    continue;
                case TXC_LMC:
                    lmar = getNum(&str);
                    continue;
                default:
                    ;
            }
        }
            // Find char dimensions (and, conveniently, addr).
        width = FONT_Index(font, c, &pc);
        if (x > rmar - width) {     // Would go out of area? Next line.
            x = lmar;
            y += font->height;
        }
        if (pc != NULL && x <= (LLS_SizeX - width) && y <= (LLS_SizeY - font->height)) {
            if ((font->flags & (FONT_FTIGHT|FONT_CHRMAP)) == 0)
                x++;
        }
        x += width;
    }
    if (w != NULL)
        *w = x - sx;
    if (h != NULL)
        *h = y - sy;
    return x;
}


int TEXT_Printf(const FONT_TFont *font, int x, int y, int ink, const char *fmt, ...) {
    char buf[300];
    va_list arg;

    va_start(arg, fmt);
    vsprintf(buf, fmt, arg);
    va_end(arg);
    return TEXT_Write(font, x, y, buf, ink);
}

// ----------------------------- TEXT.C -------------------------------

