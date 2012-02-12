// ------------------------------ THINGS.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _THINGS_H_
#define _THINGS_H_

#include "flsprs.h"

typedef struct THN_SBoundsPoint THN_TBoundsPoint, *THN_PBoundsPoint;

typedef struct {
    uint32 radius;          // Bounding circle for the object.
    int nPoints;            // Size of following array of points.
    struct THN_SBoundsPoint { // Proper points of the object's shape.
        sint32 dx, dy;      // Displacement from object's center.
    } points[];            // Not 1, but 'n' of these.
} THN_TBounds, *THN_PBounds;

struct THN_SThing;
struct SEC_SSector;

typedef struct {
    struct THN_SThing *prev, *next; // Linked list.
} THN_TLink, *THN_PLink;

typedef struct THN_SThing {
    uint32 type;                    // Type of thing as it was created.
    uint32 magic;                   // Magic to avoid repeating calculus.
    uint32 x, y, z;                 // Map position.
    word angle;                     // Angle it's looking at.
    byte flags;                     // ....
    FS3_PSprite spr;                // Graphics data.
    THN_PBounds bounds;             // Collision tracking.
    void (*routine)(void *data);    // Action routine.
    void *data;                     // Pointer passed to routine();
    THN_TLink block;                // Linked list for each map tile.
    THN_TLink global;               // Global linked list for all things.
    THN_TLink sector;               // Linked list for the sector.
    struct SEC_SSector *sec;        // Sector this thing is in.
    word tirerot;                   // Angle of tires (if a car).
    word tiredir;                   // Direction of tires (if a car).
    sint32 fill[4];
} THN_TThing, *THN_PThing;

enum {
    THNF_SOLID = 0x01,          // Things with this flag collide each other.
    THNF_VECTOR = 0x80,         // Things with this flag have a vector visual.

    THNT_PLAYER   = 0x0100,
    THNT_CAR      = 0x0500,
    THNT_OBSTACLE = 0x0600,
    THNT_SPARKS   = 0x1000,
    THNT_SMOKE    = 0x1100,
    THNT_RACEPOS  = 0x1200,
    THNT_HUMANPOS = 0x1300,
};

    // Things tile map, a 64*64 array.
typedef THN_PThing *THN_PThingMap;

// ------------------------------

    // Global var for the actual things block map.
PUBLIC THN_PThingMap THN_ThingMap;
    // And a global list of all things.
PUBLIC THN_PThing    THN_ThingList;
    // Things that think (their func != NULL).
PUBLIC THN_PThing    THN_ThinkerList;

// ------------------------------

    // Clear the given map and assign it to THN_ThingMap.
PUBLIC void THN_InitMap(THN_PThingMap map);

    // Create a new thing of a type and place it where specified.
PUBLIC THN_PThing THN_AddThing(uint32 x, uint32 y, uint32 z, int type, word angle, word flags);

    // Remove a thing from the map and delete it.
PUBLIC void THN_DeleteThing(THN_PThing t);

    // Exec the control routine of all things.
PUBLIC void THN_RunThings(void);

    // Move things around the block map.
PUBLIC void THN_MoveThing(THN_PThing t, uint32 nx, uint32 ny);


    // -------------------

    // Calculate the possible collision of a thing with the others.
PUBLIC bool THN_CalcCollide(THN_PThing t, int *s1, int *s2, THN_PThing *s);

    // Check two specific things for collision.
PUBLIC bool THN_CollideThing(THN_PThing t1, THN_PThing t2, int *s1, int *s2);

    // -------------------

typedef struct {
    THN_PThing thn;
    dword      clock;
    sint32     dx, dy;
} THN_TSparks, *THN_PSparks;

typedef struct {
    THN_PThing thn;
    dword      clock;
    sint32     dx, dy, dz;
} THN_TSmoke, *THN_PSmoke;


#endif

// ------------------------------ THINGS.H ----------------------------

