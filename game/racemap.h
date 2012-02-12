// ------------------------------ RACEMAP.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _RACEMAP_H_
#define _RACEMAP_H_

#ifndef _BASE_H_
#include <base.h>
#endif

#include "things.h"
#include "racers.h"
#include "sectors.h"

typedef struct {
    uint32 x, y;
    uint32 dir; //word dir;
    int speed;
    int ncars;
} PATH_TPoint, *PATH_PPoint;

typedef struct {
    int         numpoints;
    PATH_PPoint points;
} PATH_TPath, *PATH_PPath;

typedef struct {
    uint32 x, y, z;
} MAP_TStaticCamera, *MAP_PStaticCamera;

typedef struct {
    uint32 x, y;
    word angle;
    bool used;
} MAP_TStartPos, *MAP_PStartPos;

typedef struct {
    byte         *map[128*128];
    byte         *trans;
    word          nGrafs;
    byte         *grafs;
    THN_PThingMap thnMap;
    byte         *backg;        // Two levels of background bitmap.
    byte         *backg2;       // Front level (mountains, etc.).
    int           hbackg, hbackg2;
    MAP_PStaticCamera staticCameras;
    int               nStaticCameras;
    MAP_PStartPos     startPos;
    int               nStartPos;
    PATH_TPath        path;
    int               nracers;
    RCS_PRacer       *racers;
    SEC_TSectorMap    sec;
} MAP_TMap, *MAP_PMap;

    // Load graphics data from map.
PUBLIC bool MAP_Load(MAP_PMap map, const char *fname);

    // Load things, cameras, etc. from map.
PUBLIC bool MAP_LoadThings(MAP_PMap map, const char *fname);

PUBLIC void MAP_Free(MAP_PMap map);

 // ------------------------------------
 // Add on utilities.

#define MAP_MAX_STATIC_CAMERAS 40

PUBLIC bool              MAP_AddStaticCamera(MAP_PMap map, uint32 x, uint32 y, uint32 z);
PUBLIC MAP_PStaticCamera MAP_FindStaticCamera(MAP_PMap map, uint32 x, uint32 y, dword *d);
PUBLIC void              MAP_EndStaticCameras(MAP_PMap map);

 // ------------------------------------

#define MAP_MAX_START_POS 40

PUBLIC bool MAP_AddStartPos(MAP_PMap map, uint32 x, uint32 y, word angle);
PUBLIC int  MAP_UseStartPos(MAP_PMap map);
PUBLIC void MAP_EndStartPos(MAP_PMap map);

 // ------------------------------------

PUBLIC void PATH_Init(PATH_PPath path, const char *fname);
PUBLIC void PATH_Done(PATH_PPath path);

#endif

// ------------------------------ RACEMAP.H ----------------------------

