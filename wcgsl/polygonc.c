// ------------------------------ POLYGONC.C --------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "polygon.h"
#include <llscreen.h>
#include <dpmi.h>

POLY_TFullEdge POLY_FullEdgeBuf[768];

POLY_TFullVertex POLY_ScrapPoly[30];

PUBLIC word POLY_ShadeTableSel = 0;
PUBLIC const byte *POLY_ShadeTable = NULL;

// ------------------------------------------

/*
PRIVATE uint32 *DivTableBase = NULL;

PUBLIC bool POLY_InitDivTable(void) {
    uint32 *p;
    uint32 i, j;

    if (POLY_DivTable != NULL)
        return TRUE;
    assert(DivTableBase == NULL);
    DivTableBase = NEW(sizeof(*DivTableBase)*64*256 + 1023);
    if (DivTableBase == NULL)
        return FALSE;
    POLY_DivTable = (uint32*)((((dword)DivTableBase) + 1023UL) & ~1023UL);
    p = POLY_DivTable;
    for (i = 0; i < 64; i++)
        for (j = 0; j < 256; j++)
            if (i == 0)
                *p++ = 0;
            else
                *p++ = (j << 24)/i;
    POLY_DivTable = (uint32*)(((dword)POLY_DivTable) >> 2);
    return TRUE;
}

PUBLIC void POLY_EndDivTable(void) {
    if (POLY_DivTable == NULL)
        return;
    assert(DivTableBase != NULL);
    DISPOSE(DivTableBase);
    POLY_DivTable = NULL;
}
*/

PUBLIC bool POLY_InitDivTable(void) {
    sint32 *p;
    sint32 i, j;

    if (POLY_DivTable != NULL)
        return TRUE;
    POLY_DivTable = NEW(sizeof(*POLY_DivTable)*64*128);
    if (POLY_DivTable == NULL)
        return FALSE;
    p = (sint32*) POLY_DivTable;
    for (j = -64; j < 64; j++)
        for (i = 0; i < 64; i++)
            if (i == 0)
                *p++ = 0;
            else
                *p++ = (j << 16)/i;
    return TRUE;
}

PUBLIC void POLY_EndDivTable(void) {
    if (POLY_DivTable == NULL)
        return;
    DISPOSE(POLY_DivTable);
    POLY_DivTable = NULL;
}

// ------------------------------------------

PUBLIC bool POLY_InitShadeTable(const byte *tbl) {
    if (POLY_ShadeTableSel == 0) {
        POLY_ShadeTableSel = DPMI_NewSelector();
        if (POLY_ShadeTableSel == 0)
            return FALSE;
        
        DPMI_SetLimit(POLY_ShadeTableSel, 0xFFFF);
    }
    POLY_ShadeTable = tbl;
    DPMI_SetBaseAddress(POLY_ShadeTableSel, (dword)tbl);
    return TRUE;
}

PUBLIC void POLY_EndShadeTable(void) {
    if (POLY_ShadeTableSel != 0) {
        DPMI_FreeSelector(POLY_ShadeTableSel);
        POLY_ShadeTableSel = 0;
        POLY_ShadeTable = NULL;
    }
}

// ------------------------------------------


PUBLIC int  POLY_TracePoly(POLY_TFullEdge *edge, const POLY_TFullVertex *verts,
                           int start, int add, int size,
                           dword what) {
    dword   i, j, roll;
    sint32  dy;
    int     nscans, clipend;

    if (add < 0)
        roll = size - 1;
    else
        roll = 0;

    nscans = 0;
    for (i = start; /* break conditions inside loop */; i = j) {
        const POLY_TFullVertex *vi, *vj;
        int incscans, viy, vjy;
        vi = verts + i;
        viy = vi->y;
        if (viy >= POLY_MaxY)
            break;
        j = i + add;
        if (j >= size)
            j = roll;
        vj = verts + j;
        vjy = vj->y;
        dy = vjy - viy;
        if (dy == 0)
            continue;
        if (dy < 0)
            break;
        if (j == start)
            return 0;
        if (vjy <= POLY_MinY)
            continue;
        clipend = vjy - POLY_MaxY;
        if (clipend <= 0)
            clipend = 0;

        if (what & PTRF_EDGE)
            POLY_TraceEdge(edge, vi, vj, clipend);
        if (what & PTRF_TEXMASK)
            POLY_TraceTexture(edge, vi, vj, clipend);
        if (what & PTRF_SHADE)
            POLY_TraceShade(edge, vi, vj, clipend);
        if (what & PTRF_Z)
            POLY_TraceZ(edge, vi, vj, clipend);
        if (viy >= POLY_MinY)
            incscans = dy - clipend;
        else
            incscans = dy - (POLY_MinY-viy) - clipend;
        nscans += incscans;
        edge += incscans;
    }

    return nscans;
}

PUBLIC void POLY_SolidDraw(const POLY_TFullVertex *verts, int nverts, byte color) {
    int i;
    sint32 minx, miny, maxx, maxy;
    int minyindex;
    int nscans;
    byte c;
    const POLY_TFullVertex *v;

    assert(verts != NULL);
    assert(nverts > 0);

    minyindex = 0;
    v = verts;
    minx = maxx = v->x;
    miny = maxy = v->y;
    v++;
    for (i = 1; i < nverts; i++, v++) {
        if (minx > v->x)
            minx = v->x;
        if (maxx < v->x)
            maxx = v->x;
        if (miny > v->y) {
            miny = v->y;
            minyindex = i;
        }
        if (maxy < v->y)
            maxy = v->y;
    }
    if (maxx <= POLY_MinX || maxy <= POLY_MinY || minx >= POLY_MaxX || miny >= POLY_MaxY
        || maxx <= minx || maxy <= miny)
        return;

    nscans = POLY_TracePoly(POLY_FullEdgeBuf, verts,
                            minyindex, -1, nverts,
                            PTRF_EDGE);
    if (nscans <= 0)
        return;
    assert(nscans <= SIZEARRAY(POLY_FullEdgeBuf));
    if (nscans != POLY_TracePoly(POLY_FullEdgeBufRight, verts,
                                 minyindex, 1, nverts,
                                 PTRF_EDGE))
        return;
    if (POLY_ShadeTable != NULL)
        c = POLY_ShadeTable[256*(byte)(verts->l >> 16) + color];
    else
        c = color;
    if (miny < POLY_MinY)
        miny = POLY_MinY;
    POLY_SolidDump(((byte*)LLS_Screen) + LLS_SizeX*miny,
                   POLY_FullEdgeBuf, nscans, LLS_SizeX, c);
}

PUBLIC void POLY_GouraudDraw(const POLY_TFullVertex *verts, int nverts, byte color) {
    int i;
    sint32 minx, miny, maxx, maxy;
    int minyindex;
    int nscans;
    POLY_TFullVertex *v;

    if (POLY_ShadeTableSel == 0) {
        POLY_SolidDraw(verts, nverts, color);
        return;
    }

    assert(verts != NULL);
    assert(nverts > 0);

    minyindex = 0;
    v = (POLY_TFullVertex*)verts;
    minx = maxx = v->x;
    miny = maxy = v->y;
//    pl = ((byte*)&v->l);
//    pl[2] = POLY_ShadeTable[256*(byte)(v->l >> 16) + color];
    v->l = (POLY_ShadeTable[256*(byte)((v->l >> 16) + (1 << 15)) + color] << 16) + (v->l & 0xFFFF) - (1 << 15);
    v++;
    for (i = 1; i < nverts; i++, v++) {
        if (minx > v->x)
            minx = v->x;
        if (maxx < v->x)
            maxx = v->x;
        if (miny > v->y) {
            miny = v->y;
            minyindex = i;
        }
        if (maxy < v->y)
            maxy = v->y;
//        pl = ((byte*)&v->l);
//        pl[2] = POLY_ShadeTable[256*(byte)(v->l >> 16) + color];
        v->l = (POLY_ShadeTable[256*(byte)((v->l >> 16) + (1 << 15)) + color] << 16) + (v->l & 0xFFFF) - (1 << 15);
    }
    if (maxx <= POLY_MinX || maxy <= POLY_MinY || minx >= POLY_MaxX || miny >= POLY_MaxY
        || maxx <= minx || maxy <= miny)
        return;

    nscans = POLY_TracePoly(POLY_FullEdgeBuf, verts,
                            minyindex, -1, nverts,
                            PTRF_EDGE | PTRF_SHADE);
    if (nscans <= 0)
        return;
    assert(nscans <= SIZEARRAY(POLY_FullEdgeBuf));
    if (nscans != POLY_TracePoly(POLY_FullEdgeBufRight, verts,
                                 minyindex, 1, nverts,
                                 PTRF_EDGE | PTRF_SHADE))
        return;

    if (miny < POLY_MinY)
        miny = POLY_MinY;
    POLY_GouraudDump(((byte*)LLS_Screen) + LLS_SizeX*miny,
                     POLY_FullEdgeBuf, nscans, LLS_SizeX);
}

PUBLIC void POLY_ShadeDraw(const POLY_TFullVertex *verts, int nverts, byte color) {
    int i;
    sint32 minx, miny, maxx, maxy;
    int minyindex;
    int nscans;
    const POLY_TFullVertex *v;

    if (POLY_ShadeTableSel == 0) {
        POLY_SolidDraw(verts, nverts, color);
        return;
    }

    assert(verts != NULL);
    assert(nverts > 0);

    minyindex = 0;
    v = verts;
    minx = maxx = v->x;
    miny = maxy = v->y;
    v++;
    for (i = 1; i < nverts; i++, v++) {
        if (minx > v->x)
            minx = v->x;
        if (maxx < v->x)
            maxx = v->x;
        if (miny > v->y) {
            miny = v->y;
            minyindex = i;
        }
        if (maxy < v->y)
            maxy = v->y;
    }
    if (maxx <= POLY_MinX || maxy <= POLY_MinY || minx >= POLY_MaxX || miny >= POLY_MaxY
        || maxx <= minx || maxy <= miny)
        return;

    nscans = POLY_TracePoly(POLY_FullEdgeBuf, verts,
                            minyindex, -1, nverts,
                            PTRF_EDGE | PTRF_SHADE);
    if (nscans <= 0)
        return;
    assert(nscans <= SIZEARRAY(POLY_FullEdgeBuf));
    if (nscans != POLY_TracePoly(POLY_FullEdgeBufRight, verts,
                                 minyindex, 1, nverts,
                                 PTRF_EDGE | PTRF_SHADE))
        return;

    if (miny < POLY_MinY)
        miny = POLY_MinY;
    POLY_ShadeDump(((byte*)LLS_Screen) + LLS_SizeX*miny,
                   POLY_FullEdgeBuf, nscans, LLS_SizeX, color, POLY_ShadeTable);
}

PUBLIC void POLY_TextureDraw(const POLY_TFullVertex *verts, int nverts, const byte *texture) {
    int i;
    sint32 minx, miny, maxx, maxy;
    int minyindex;
    int nscans;
    const POLY_TFullVertex *v;

    assert(verts != NULL);
    assert(nverts > 0);

    if (texture == NULL) {
        POLY_SolidDraw(verts, nverts, 30);
        return;
    }

    minyindex = 0;
    v = verts;
    minx = maxx = v->x;
    miny = maxy = v->y;
    v++;
    for (i = 1; i < nverts; i++, v++) {
        if (minx > v->x)
            minx = v->x;
        if (maxx < v->x)
            maxx = v->x;
        if (miny > v->y) {
            miny = v->y;
            minyindex = i;
        }
        if (maxy < v->y)
            maxy = v->y;
    }
    if (maxx <= POLY_MinX || maxy <= POLY_MinY || minx >= POLY_MaxX || miny >= POLY_MaxY
        || maxx <= minx || maxy <= miny)
        return;

    nscans = POLY_TracePoly(POLY_FullEdgeBuf, verts,
                            minyindex, -1, nverts,
                            PTRF_EDGE | PTRF_TEXTURE);
    if (nscans <= 0)
        return;
    assert(nscans <= SIZEARRAY(POLY_FullEdgeBuf));
    if (nscans != POLY_TracePoly(POLY_FullEdgeBufRight, verts,
                                 minyindex, 1, nverts,
                                 PTRF_EDGE | PTRF_TEXTURE))
        return;

    if (miny < POLY_MinY)
        miny = POLY_MinY;
    POLY_TextureDump(((byte*)LLS_Screen) + LLS_SizeX*miny,
                   POLY_FullEdgeBuf, nscans, LLS_SizeX, texture);
}

PUBLIC void POLY_HoleTexDraw(const POLY_TFullVertex *verts, int nverts, const byte *texture) {
    int i;
    sint32 minx, miny, maxx, maxy;
    int minyindex;
    int nscans;
    const POLY_TFullVertex *v;

    assert(verts != NULL);
    assert(nverts > 0);

    if (texture == NULL) {
        POLY_SolidDraw(verts, nverts, 30);
        return;
    }

    minyindex = 0;
    v = verts;
    minx = maxx = v->x;
    miny = maxy = v->y;
    v++;
    for (i = 1; i < nverts; i++, v++) {
        if (minx > v->x)
            minx = v->x;
        if (maxx < v->x)
            maxx = v->x;
        if (miny > v->y) {
            miny = v->y;
            minyindex = i;
        }
        if (maxy < v->y)
            maxy = v->y;
    }
    if (maxx <= POLY_MinX || maxy <= POLY_MinY || minx >= POLY_MaxX || miny >= POLY_MaxY
        || maxx <= minx || maxy <= miny)
        return;

    nscans = POLY_TracePoly(POLY_FullEdgeBuf, verts,
                            minyindex, -1, nverts,
                            PTRF_EDGE | PTRF_TEXTURE);
    if (nscans <= 0)
        return;
    assert(nscans <= SIZEARRAY(POLY_FullEdgeBuf));
    if (nscans != POLY_TracePoly(POLY_FullEdgeBufRight, verts,
                                 minyindex, 1, nverts,
                                 PTRF_EDGE | PTRF_TEXTURE))
        return;

    if (miny < POLY_MinY)
        miny = POLY_MinY;
    POLY_HoleTexDump(((byte*)LLS_Screen) + LLS_SizeX*miny,
                     POLY_FullEdgeBuf, nscans, LLS_SizeX, texture);
}

PUBLIC void POLY_LightTexDraw(const POLY_TFullVertex *verts, int nverts, const byte *texture) {
    int i;
    sint32 minx, miny, maxx, maxy;
    int minyindex;
    int nscans;
    const POLY_TFullVertex *v;

    if (texture == NULL) {
        POLY_SolidDraw(verts, nverts, 30);
        return;
    } else if (POLY_ShadeTable == NULL) {
        POLY_TextureDraw(verts, nverts, texture);
        return;
    }
    assert(verts != NULL);
    assert(nverts > 0);

    minyindex = 0;
    v = verts;
    minx = maxx = v->x;
    miny = maxy = v->y;
    v++;
    for (i = 1; i < nverts; i++, v++) {
        if (minx > v->x)
            minx = v->x;
        if (maxx < v->x)
            maxx = v->x;
        if (miny > v->y) {
            miny = v->y;
            minyindex = i;
        }
        if (maxy < v->y)
            maxy = v->y;
    }
    if (maxx <= POLY_MinX || maxy <= POLY_MinY || minx >= POLY_MaxX || miny >= POLY_MaxY
        || maxx <= minx || maxy <= miny)
        return;

    nscans = POLY_TracePoly(POLY_FullEdgeBuf, verts,
                            minyindex, -1, nverts,
                            PTRF_EDGE | PTRF_TEXTURE);
    if (nscans <= 0)
        return;
    assert(nscans <= SIZEARRAY(POLY_FullEdgeBuf));
    if (nscans != POLY_TracePoly(POLY_FullEdgeBufRight, verts,
                                 minyindex, 1, nverts,
                                 PTRF_EDGE | PTRF_TEXTURE))
        return;

    if (miny < POLY_MinY)
        miny = POLY_MinY;
    POLY_LightTexDump(((byte*)LLS_Screen) + LLS_SizeX*miny,
                      POLY_FullEdgeBuf, nscans, LLS_SizeX, texture,
                      POLY_ShadeTable + 256*(byte)(verts->l >> 16));
}

PUBLIC void POLY_ShadeTexDraw(const POLY_TFullVertex *verts, int nverts, const byte *texture) {
    int i;
    sint32 minx, miny, maxx, maxy;
    int minyindex;
    int nscans;
    const POLY_TFullVertex *v;

    if (texture == NULL) {
        POLY_SolidDraw(verts, nverts, 30);
        return;
    } else if (POLY_ShadeTableSel == 0) {
        POLY_TextureDraw(verts, nverts, texture);
        return;
    }

    assert(verts != NULL);
    assert(nverts > 0);

    minyindex = 0;
    v = verts;
    minx = maxx = v->x;
    miny = maxy = v->y;
    v++;
    for (i = 1; i < nverts; i++, v++) {
        if (minx > v->x)
            minx = v->x;
        if (maxx < v->x)
            maxx = v->x;
        if (miny > v->y) {
            miny = v->y;
            minyindex = i;
        }
        if (maxy < v->y)
            maxy = v->y;
    }
    if (maxx <= POLY_MinX || maxy <= POLY_MinY || minx >= POLY_MaxX || miny >= POLY_MaxY
        || maxx <= minx || maxy <= miny)
        return;

    nscans = POLY_TracePoly(POLY_FullEdgeBuf, verts,
                            minyindex, -1, nverts,
                            PTRF_EDGE | PTRF_TEXTURE | PTRF_SHADE);
    if (nscans <= 0)
        return;
    assert(nscans <= SIZEARRAY(POLY_FullEdgeBuf));
    if (nscans != POLY_TracePoly(POLY_FullEdgeBufRight, verts,
                                 minyindex, 1, nverts,
                                 PTRF_EDGE | PTRF_TEXTURE | PTRF_SHADE))
        return;

    if (miny < POLY_MinY)
        miny = POLY_MinY;
    POLY_ShadeTexDump(((byte*)LLS_Screen) + LLS_SizeX*miny,
                      POLY_FullEdgeBuf, nscans, LLS_SizeX, texture, POLY_ShadeTableSel);
}

PUBLIC void POLY_TransDraw(const POLY_TFullVertex *verts, int nverts) {
    int i;
    sint32 minx, miny, maxx, maxy;
    int minyindex;
    int nscans;
    const POLY_TFullVertex *v;

    if (POLY_ShadeTable == NULL) {
        POLY_SolidDraw(verts, nverts, 127);
        return;
    }

    assert(verts != NULL);
    assert(nverts > 0);

    minyindex = 0;
    v = verts;
    minx = maxx = v->x;
    miny = maxy = v->y;
    v++;
    for (i = 1; i < nverts; i++, v++) {
        if (minx > v->x)
            minx = v->x;
        if (maxx < v->x)
            maxx = v->x;
        if (miny > v->y) {
            miny = v->y;
            minyindex = i;
        }
        if (maxy < v->y)
            maxy = v->y;
    }
    if (maxx <= POLY_MinX || maxy <= POLY_MinY || minx >= POLY_MaxX || miny >= POLY_MaxY
        || maxx <= minx || maxy <= miny)
        return;

    nscans = POLY_TracePoly(POLY_FullEdgeBuf, verts,
                            minyindex, -1, nverts,
                            PTRF_EDGE);
    if (nscans <= 0)
        return;
    assert(nscans <= SIZEARRAY(POLY_FullEdgeBuf));
    if (nscans != POLY_TracePoly(POLY_FullEdgeBufRight, verts,
                                 minyindex, 1, nverts,
                                 PTRF_EDGE))
        return;

    if (miny < POLY_MinY)
        miny = POLY_MinY;
    POLY_TransDump(((byte*)LLS_Screen) + LLS_SizeX*miny,
                   POLY_FullEdgeBuf, nscans, LLS_SizeX,
                   POLY_ShadeTable + 256*(byte)(verts->l >> 16));
}


PUBLIC void POLY_Line(sint32 x0, sint32 y0, sint32 x1, sint32 y1, int c) {
    sint32 dx, dy, x, y, py, ix, iy, ipy, ie, d, ine;

    if ((   (x0 <= POLY_MinX && x1 < POLY_MinX)
         ||(x0 >= POLY_MaxX && x1 >= POLY_MaxX))
     || (   (y0 <= POLY_MinY && y1 < POLY_MinY)
         ||(y0 >= POLY_MaxY && y1 >= POLY_MaxY)))
         return;

        // Calc global deltas.
    dx = x1 - x0;
    dy = y1 - y0;
    if (dx > 0) ix = 1;
    else        ix = -1;
    if (dy > 0) iy = 1;
    else        iy = -1;
    x = x0;
    y = y0;
    if (dx < 0)
        dx = -dx;
    if (dy < 0)
        dy = -dy;
    py  = y*LLS_SizeX;
    ipy = iy*LLS_SizeX;
    if (x0 < POLY_MinX || x1 >= POLY_MaxX ||
        x1 < POLY_MinX || x0 >= POLY_MaxX ||
        y0 < POLY_MinY || y1 >= POLY_MaxY ||
        y1 < POLY_MinY || y0 >= POLY_MaxY
        ) {
            // Slow version, with clipping.
        if (abs(dx) > abs(dy)) {
            ie  = 2*dy;
            d   = ie - dx;
            ine = d - dx;
            while (x != x1) {
                if (x < POLY_MinX || y < POLY_MinY || x >= POLY_MaxX || y >= POLY_MaxY)
                ;else LLS_Screen[0][py+x] = c;
                if (d <= 0) {
                    d += ie;
                } else {
                    d += ine;
                    y += iy;
                    py += ipy;
                }
                x += ix;
            }
        } else {
            ie  = 2*dx;
            d   = ie - dy;
            ine = d - dy;
            while (y != y1) {
                if (x < POLY_MinX || y < POLY_MinY || x >= POLY_MaxX || y >= POLY_MaxY)
                ;else
                    LLS_Screen[0][py+x] = c;
                if (d <= 0) {
                    d += ie;
                } else {
                    d += ine;
                    x += ix;
                }
                y += iy;
                py += ipy;
            }
        }
    } else {
            // No need to clip line.
        if (abs(dx) > abs(dy)) {
            ie  = 2*dy;
            d   = ie - dx;
            ine = d - dx;
            while (x != x1) {
                assert(x >= POLY_MinX);
                assert(y >= POLY_MinY);
                assert(x < POLY_MaxX);
                assert(y < POLY_MaxY);
                LLS_Screen[0][py+x] = c;
                if (d <= 0) {
                    d += ie;
                } else {
                    d += ine;
#ifndef NDEBUG
                    y += iy;
#endif
                    py += ipy;
                }
                x += ix;
            }
        } else {
            ie  = 2*dx;
            d   = ie - dy;
            ine = d - dy;
            while (y != y1) {
                assert(x >= POLY_MinX);
                assert(y >= POLY_MinY);
                assert(x < POLY_MaxX);
                assert(y < POLY_MaxY);
                LLS_Screen[0][py+x] = c;
                if (d <= 0) {
                    d += ie;
                } else {
                    d += ine;
                    x += ix;
                }
                y += iy;
                py += ipy;
            }
        }
    }
}

// --------------------

#define ZBITS 5
#define NPTBITS (18+ZBITS)

byte *PutTexByte(byte *dest, const byte *tex, sint32 tx, sint32 ty);
#pragma aux PutTexByte parm [EDI] [ESI] [EDX] [EAX] value [EDI] = \
"   SHR EAX,23   " \
"   SHL EDX,32-6-23  " \
"   AND EAX,63            " \
"   SHLD EAX,EDX,6        " \
"   MOV  AL,[ESI+EAX]     " \
"   MOV  [EDI],AL         " \
"   INC  EDI              "

PUBLIC void POLY_ZTextureDumpC(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture) {
    int i;
    for (;nscans > 0; nscans--, edge++, dest += width) {
        sint32 tx, ty, z;
        sint32 dx, dtx, dty, dz;
        sint32 ptx, pty, nptx, npty, dptx, dpty;
        byte *p;

        if (edge->x0 >= edge->x1)
            continue;

        dx = edge->x1 - edge->x0;
        dtx = (edge->tx1 - edge->tx0)/dx;
        dty = (edge->ty1 - edge->ty0)/dx;
        dz = (edge->z1 - edge->z0)/dx;
        if (edge->x0 < POLY_MinX) {
            edge->tx0 += dtx*(POLY_MinX-edge->x0);
            edge->ty0 += dty*(POLY_MinX-edge->x0);
            edge->z0  += dz*(POLY_MinX-edge->x0);
            edge->x0 = POLY_MinX;
        }
        if (edge->x1 > POLY_MaxX)
            edge->x1 = POLY_MaxX;
        dx = edge->x1 - edge->x0;
        if (dx <= 0)
            continue;
        p = dest + edge->x0;
        tx = edge->tx0;
        ty = edge->ty0;
        z = edge->z0;


        ptx = FP32Div(tx, z);    // (tx/z*(1 << 32))/((1 << 32)/z) == tx*z/z*(1 << 32)/(1 << 32) == tx
        pty = FP32Div(ty, z);    // (tx/z*(1 << 32))/((1 << 32)/z) == tx*z/z*(1 << 32)/(1 << 32) == tx

        while (dx >= 16) {
            tx = tx+dtx*16;
            ty = ty+dty*16;
            z  = z+dz*16;
            nptx = FP32Div(tx, z);
            npty = FP32Div(ty, z);
            dptx = (nptx-ptx)/16;
            dpty = (npty-pty)/16;
                    // 16 times.
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
                p = PutTexByte(p, texture, ptx, pty); ptx += dptx; pty += dpty;
            ptx = nptx;
            pty = npty;
            dx -= 16;
        }

        if (dx > 0) {
            tx = tx+dtx*dx;
            ty = ty+dty*dx;
            z  = z+dz*dx;
            nptx = FP32Div(tx, z);
            npty = FP32Div(ty, z);
            dptx = (nptx-ptx)/dx;
            dpty = (npty-pty)/dx;
            for (i = 0; i < dx; i++) {
                *p++ = texture[((ptx >> NPTBITS)&0x3F) + (((pty >> NPTBITS)&0x3F) << 6)];
                ptx += dptx; pty += dpty;
            }
        }
    }
}


    // Value for drawing a texture without persp. correction.
    // The greater, the further away for the frontier. Default is 0x2000
PUBLIC int POLY_MinZProjTexture = 0x2000;

PUBLIC bool POLY_GenDraw(POLY_TFullVertex *verts, int nverts, int color,
                         const byte *texture, dword what) {
    int i;
    sint32 minx, miny, maxx, maxy, minz, maxz;
    int minyindex;
    int nscans;
    POLY_TFullVertex *v;
    byte *dest;

    if (verts == NULL || nverts < 2)
        return FALSE;

        // First clean up flags.
    what &= ~PTRF_Z;
    if (texture == NULL)
        what &= ~(PTRF_TEXMASK|PTRF_HOLES);
    if (what & PTRF_TRANS)
        what = 0;
    if (what & PTRF_HOLES)
        what &= ~(PTRF_SHADE|PTRF_ZTEXTURE);
    if (POLY_ShadeTable == NULL)
        what &= ~(PTRF_LIGHT|PTRF_SHADE);
    if (!(what & PTRF_SHADE))
;//        color = POLY_ShadeTable[256*(byte)(verts->l >> 16) + color];

    if (nverts == 2) {
        POLY_Line(verts[0].x, verts[0].y, verts[1].x, verts[1].y, color);
        return TRUE;
    }

    what |= PTRF_EDGE;

    minyindex = 0;
    v = verts;
    minx = maxx = v->x;
    miny = maxy = v->y;
    minz = maxz = v->rz;
    v++;
    for (i = 1; i < nverts; i++, v++) {
        if (minx > v->x)
            minx = v->x;
        if (maxx < v->x)
            maxx = v->x;
        if (miny > v->y) {
            miny = v->y;
            minyindex = i;
        }
        if (maxy < v->y)
            maxy = v->y;
        if (minz > v->rz)
            minz = v->rz;
        if (maxz < v->rz)
            maxz = v->rz;
    }
    if (maxx <= POLY_MinX || maxy <= POLY_MinY || minx >= POLY_MaxX || miny >= POLY_MaxY
        || maxx <= minx || maxy <= miny)
        return FALSE;

    if (minz > POLY_MinZProjTexture || minz < 32)
        what &= ~PTRF_ZTEXTURE;
    if (what & PTRF_ZTEXTURE) {
            // Apply Z correction to U,V and invert Z for interpolation.
        for (i = 0, v = verts; i < nverts; i++, v++) {
            v->tx = FPnDiv(v->tx, v->rz, ZBITS);
            v->ty = FPnDiv(v->ty, v->rz, ZBITS);
            v->rz = FPnDiv((1 << 30), v->rz, 2); // Effectively, (1 << 32)/z
        }
        what |= PTRF_Z;
    }
/*
    if ((what & ~(PTRF_EDGE|PTRF_Z)) == PTRF_SHADE) {
        v = verts;
        for (i = 0; i < nverts; i++, v++) {
            v->l = (POLY_ShadeTable[256*(byte)((v->l >> 16) + (1 << 15)) + color] << 16) + (v->l & 0xFFFF) - (1 << 15);
        }
    }
*/
    nscans = POLY_TracePoly(POLY_FullEdgeBuf, verts,
                            minyindex, -1, nverts,
                            what);
    if (nscans <= 0)
        return FALSE;
    assert(nscans <= SIZEARRAY(POLY_FullEdgeBuf));
    if (nscans != POLY_TracePoly(POLY_FullEdgeBufRight, verts,
                                 minyindex, 1, nverts,
                                 what))
        return FALSE;

    if (miny < POLY_MinY)
        miny = POLY_MinY;

        // Clean up flags.
    what &= ~(PTRF_EDGE|PTRF_Z);
    dest = ((byte*)LLS_Screen) + LLS_SizeX*miny;
    switch (what) {
        case 0:
        case PTRF_LIGHT:
            color = POLY_ShadeTable[256*(byte)(verts->l >> 16) + color];
            POLY_SolidDump(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX, color);
            break;
        case PTRF_HOLES|PTRF_TEXTURE:
            POLY_HoleTexDump(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX, texture);
            break;
        case PTRF_TRANS:
            POLY_TransDump(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX,
                           POLY_ShadeTable + 256*(byte)(verts->l >> 16));
            break;
        case PTRF_SHADE:
//            POLY_GouraudDump(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX);
            POLY_ShadeDump(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX, color, POLY_ShadeTable);
            break;
        case PTRF_TEXTURE:
            POLY_TextureDump(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX, texture);
            break;
        case PTRF_TEXTURE|PTRF_TEX256:
        case PTRF_TEXTURE|PTRF_LIGHT|PTRF_TEX256:
        case PTRF_TEXTURE|PTRF_ZTEXTURE|PTRF_TEX256:
        case PTRF_TEXTURE|PTRF_ZTEXTURE|PTRF_LIGHT|PTRF_TEX256:
            POLY_TextureDump256(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX, texture);
            break;
        case PTRF_TEXTURE|PTRF_SHADE|PTRF_TEX256:
        case PTRF_TEXTURE|PTRF_ZTEXTURE|PTRF_SHADE|PTRF_TEX256:
            POLY_ShadeTexDump256(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX, texture,
                                 POLY_ShadeTableSel);
            break;
        case PTRF_TEXTURE|PTRF_LIGHT:
            POLY_LightTexDump(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX, texture,
                              POLY_ShadeTable + 256*(byte)(verts->l >> 16));
            break;
        case PTRF_TEXTURE|PTRF_SHADE:
            POLY_ShadeTexDump(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX, texture,
                              POLY_ShadeTableSel);
            break;
        case PTRF_TEXTURE|PTRF_ZTEXTURE:
        case PTRF_TEXTURE|PTRF_ZTEXTURE|PTRF_LIGHT:
        case PTRF_TEXTURE|PTRF_ZTEXTURE|PTRF_SHADE:
            POLY_ZTextureDump(dest, POLY_FullEdgeBuf, nscans, LLS_SizeX, texture);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}


// ------------------------------ POLYGONC.C --------------------

