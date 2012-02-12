// --------------------------- POLYGON.H ------------------------------
// For use with WATCOM 9.5 + DOS4GW
// (C) Copyright 1993/4 by Jare & JCAB of Iguana.

// Polygon stuff. Coordinates go from (POLY_MinX,MinY) to (POLY_MaxX,MaxY).
// MinX and MinY are not 100% tested.

#ifndef _POLYGON_H_
#define _POLYGON_H_

#ifndef _BASE_H_
#include <base.h>
#endif

    // Default to 0, 0, 320, 200
PUBLIC sint32 POLY_MinX, POLY_MinY, POLY_MaxX, POLY_MaxY;

// ----------------------

#define POLY_TSB    14                 // # of integer bits in texture coordinates.
#define POLY_TSMULT (1<<(32-POLY_TSB)) // Texture coordinate multiplier.

typedef struct {
    sint32 x, y;        // Screen x coordinates of vertex.
    sint32 tx, ty;      // Texture coordinates.
    sint32 l;           // Light value.
    sint32 rx, ry, rz;  // Vertex's unprojected coordinates.
} POLY_TFullVertex, *POLY_PFullVertex;

typedef struct {
        // All these are left, right values:
    sint32 x0, x1;      // Screen x coordinates of scan.
    sint32 tx0, tx1;    // Texture x coordinates.
    sint32 ty0, ty1;    // Texture y coordinates.
    sint32 l0, l1;      // Light values.
    sint32 z0, z1;      // Object's z coordinates.
} POLY_TFullEdge, *POLY_PFullEdge;
    // Notes about the values.
        // Texture coordinates are in TSB.(32-TSB) format.
        // Light values are in 8.24 format.
        // Light values are in ?.?  format.

    // Scrap edgebuf.
PUBLIC  POLY_TFullEdge        POLY_FullEdgeBuf[768];
#define POLY_FullEdgeBufRight ((POLY_PFullEdge)(&POLY_FullEdgeBuf[0].x1))

    // Handy scrap polygon structure to use (but you can use your own!).
PUBLIC POLY_TFullVertex POLY_ScrapPoly[30];

// ------------------------------------------

PUBLIC bool POLY_InitShadeTable(const byte *tbl);
PUBLIC void POLY_EndShadeTable(void);

PUBLIC word POLY_ShadeTableSel;
PUBLIC const byte *POLY_ShadeTable;

/*
PUBLIC const uint32 *POLY_DivTable;   // Divide table, aligned to 1024.
                                      // [64][256] in 24.8 format.
*/
PUBLIC const sint32 *POLY_DivTable;   // Divide table. x/y = table[y][x]
                                      // [64][-64..63] in 16.16 format.

PUBLIC bool POLY_InitDivTable(void);
PUBLIC void POLY_EndDivTable(void);

// ------------------------------------------

    // Flags for the 'what' parameter below.
    // Also serve for the GenDraw() function.
enum {
    PTRF_EDGE       = 0x0001,
    PTRF_TEXTURE    = 0x0002,
    PTRF_SHADE      = 0x0004,
    PTRF_Z          = 0x0008,
    PTRF_TEX256     = 0x0010,

        // These only apply to GenDraw().
    PTRF_HOLES      = 0x0100,       // Texture has holes
    PTRF_TRANS      = 0x0200,       // Make transparency.
    PTRF_LIGHT      = 0x0400,       // Apply light (gouraud if also _SHADE)
    PTRF_ZTEXTURE   = 0x0800,       // Texture coords are to be z-correct.
    PTRF_TEXMASK    = (PTRF_ZTEXTURE|PTRF_TEXTURE|PTRF_TEX256),

};

PUBLIC int  POLY_TracePoly(POLY_TFullEdge *edge, const POLY_TFullVertex *verts,
                           int start, int add, int size,
                           dword what);

// Note that this routine fills one edge, so it will store edge[0], edge[2],
// etc... You will call it twice, with edge for left edge and &edge.x1 for
// right edge.

// ------------------------------------------

PUBLIC void POLY_TraceEdge(POLY_TFullEdge *edge,
                           const POLY_TFullVertex *v0,
                           const POLY_TFullVertex *v1,
                           int clipend);
#pragma aux POLY_TraceEdge parm [EDI] [ESI] [EBX] [EDX]

PUBLIC void POLY_TraceTexture(POLY_TFullEdge *edge,
                              const POLY_TFullVertex *v0,
                              const POLY_TFullVertex *v1,
                              int clipend);
#pragma aux POLY_TraceTexture parm [EDI] [ESI] [EBX] [EDX]

PUBLIC void POLY_TraceShade(POLY_TFullEdge *edge,
                            const POLY_TFullVertex *v0,
                            const POLY_TFullVertex *v1,
                            int clipend);
#pragma aux POLY_TraceShade parm [EDI] [ESI] [EBX] [EDX]

PUBLIC void POLY_TraceZ(POLY_TFullEdge *edge,
                        const POLY_TFullVertex *v0,
                        const POLY_TFullVertex *v1,
                        int clipend);
#pragma aux POLY_TraceZ parm [EDI] [ESI] [EBX] [EDX]

// ------------------------------------------

PUBLIC void POLY_SolidDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, int color);
#pragma aux POLY_SolidDump parm [EDI] [ESI] [ECX] [EDX] [EAX]

PUBLIC void POLY_ShadeDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, int color, const byte *ltable);
#pragma aux POLY_ShadeDump parm [EDI] [ESI] [ECX] [EDX] [EBX] [EAX]

PUBLIC void POLY_GouraudDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width);
#pragma aux POLY_GouraudDump parm [EDI] [ESI] [ECX] [EDX]

PUBLIC void POLY_TextureDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture);
#pragma aux POLY_TextureDump parm [EDI] [ESI] [ECX] [EDX] [EBX]

PUBLIC void POLY_TextureDump256(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture);
#pragma aux POLY_TextureDump256 parm [EDI] [ESI] [ECX] [EDX] [EBX]

PUBLIC void POLY_HoleTexDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture);
#pragma aux POLY_HoleTexDump parm [EDI] [ESI] [ECX] [EDX] [EBX]

PUBLIC void POLY_LightTexDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture, const byte *ltable);
#pragma aux POLY_LightTexDump parm [EDI] [ESI] [ECX] [EDX] [EBX] [EAX]

PUBLIC void POLY_ShadeTexDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture, word ShadeTableSel);
#pragma aux POLY_ShadeTexDump parm [EDI] [ESI] [ECX] [EDX] [EBX] [EAX]

PUBLIC void POLY_ShadeTexDump256(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture, word ShadeTableSel);
#pragma aux POLY_ShadeTexDump256 parm [EDI] [ESI] [ECX] [EDX] [EBX] [EAX]

PUBLIC void POLY_TransDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *ltable);
#pragma aux POLY_TransDump parm [EDI] [ESI] [ECX] [EDX] [EBX]

PUBLIC void POLY_ZTextureDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture);
#pragma aux POLY_ZTextureDump parm [EDI] [ESI] [ECX] [EDX] [EBX]



PUBLIC void POLY_SolidDraw(const POLY_TFullVertex *verts, int nverts, byte color);

PUBLIC void POLY_GouraudDraw(const POLY_TFullVertex *verts, int nverts, byte color);

PUBLIC void POLY_ShadeDraw(const POLY_TFullVertex *verts, int nverts, byte color);

PUBLIC void POLY_TextureDraw(const POLY_TFullVertex *verts, int nverts, const byte *texture);

PUBLIC void POLY_HoleTexDraw(const POLY_TFullVertex *verts, int nverts, const byte *texture);

PUBLIC void POLY_LightTexDraw(const POLY_TFullVertex *verts, int nverts, const byte *texture);

PUBLIC void POLY_ShadeTexDraw(const POLY_TFullVertex *verts, int nverts, const byte *texture);

PUBLIC void POLY_TransDraw(const POLY_TFullVertex *verts, int nverts);


/*
#define POLY_SolidDump(a,b,c,d,e)
#define POLY_ShadeDump(a,b,c,d,e,f)
#define POLY_TextureDump(a,b,c,d,e)
#define POLY_LightTexDump(a,b,c,d,e,f)
#define POLY_ShadeTexDump(a,b,c,d,e,f)
*/

PUBLIC void POLY_Line(sint32 x0, sint32 y0, sint32 x1, sint32 y1, int c);

// --------------------

PUBLIC void POLY_ZTextureDumpC(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture);

    // Value for drawing a texture without persp. correction.
    // The greater, the further away for the frontier. Default is 0x2000
PUBLIC int POLY_MinZProjTexture;

    // TRUE if polygon has most probably drawn something to screen.
PUBLIC bool POLY_GenDraw(POLY_TFullVertex *verts, int nverts, int color,
                         const byte *texture, dword what);



#endif

// --------------------------- POLYGON.H ------------------------------

