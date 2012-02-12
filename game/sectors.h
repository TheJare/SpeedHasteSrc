// ------------------------------ SECTORS.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _SECTORS_H_
#define _SECTORS_H_

#include "things.h"
#include "3dfloor.h"
#include <is2code.h>
#include <namelist.h>

typedef struct {
    dword magic;
    uint32 x, y;
    sint32 rx, rz;
    sint32 pz, px, py;      // Rotated & projected (py is depth).
} SEC_TVertex, *SEC_PVertex;

struct SEC_SSector;

typedef struct {
    SEC_PVertex  v0, v1;
    IS2_PSprite  tex;
    struct SEC_SSector *otherside;
    word         angle;
} SEC_TSide, *SEC_PSide;

typedef struct SEC_SSector {
    dword       flags;
    int         nv;
    SEC_TSide  *v;
    dword       magic;
    THN_PThing  list;
} SEC_TSector, *SEC_PSector;

typedef struct {
    int nv;
    int ns;

    SEC_PVertex verts;
    SEC_PSector secs;

    NL_TNameTree  tnames;
    SEC_PSector  *slist;        // Size is ns.
} SEC_TSectorMap, *SEC_PSectorMap;

// ----------------------------

#define SEC_TOSEC(c) ((uint32)((c) - 0x40000000) >> 15)
#define SEC_TOMAP(c) ((sint32)((c) << 15) + 0x40000000)

PUBLIC bool SEC_LoadMap(SEC_PSectorMap sec, const char *name);

PUBLIC bool SEC_IsInSector(SEC_PSectorMap sec, SEC_PSector s, uint32 x, uint32 y);

PUBLIC SEC_PSector SEC_FindSector(SEC_PSectorMap sec, SEC_PSector s, uint32 x, uint32 y);

PUBLIC void SEC_EndMap(SEC_PSectorMap sec);


PUBLIC void SEC_RenderSector(SEC_PSector s, int cx, int cy, F3D_PCamera cam);

PUBLIC void SEC_Render(SEC_PSectorMap sec, SEC_PSector s, int cx, int cy, F3D_PCamera cam);


#endif

// ------------------------------ SECTORS.H ----------------------------
