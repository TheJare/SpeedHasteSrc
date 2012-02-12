// ------------------------------ FLSPRS.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _FLSPRS_H_
#define _FLSPRS_H_

#include "3dfloor.h"
#include <object3d.h>
#include <is2code.h>

typedef struct {
    uint32 w, h;    // World size of sprites.
    IS2_PSprite sp;
} FS2_TSprite, *FS2_PSprite;

enum {
        // Sprite flags.
    FLSF_VECTOR  = 0x0001,
    FLSF_NOSCALE = 0x0002,

        // Mesh flags (in O3DM_Object).
    FLOF_HASTIRES = 0x0001,
};

typedef struct {
    word  flags;            // FLSF_xxx combo.
    word nSprs;
    union {
            // 3D object (different views for distances).
        struct {                // If there is only 1 nSprs.
            int tirev;
            sint32 tirecenter[4][3];
            O3DM_PObject obj;
        };
        struct {
            int tirev;
            sint32 tirecenter[4][3];
            O3DM_PObject obj;
        } *objs;

            // 2D bitmap (different views for angles and heights).
        struct {                // For each horizontal angle...
            word nSprs;         // 'n' vertical sprites.
            FS2_PSprite sprs;
        } *sprs;
    };
} FS3_TSprite, *FS3_PSprite;

    // Load and dispose single sprites.
PUBLIC bool FS3_Load(FS3_PSprite s, const char *fname);
PUBLIC void FS3_Delete(FS3_PSprite s);

    // Load a sprite into the GL_NameTree namelist. If it was already
    // loaded, it won't be reloaded.
PUBLIC FS3_PSprite FS3_New(const char *fname);

    // Dispose of all sprites in the GL_NameTree namelist.
PUBLIC void FS3_End(void);

// --------------------------

PUBLIC void FSP_ClearObjs(int scrx, int scry, F3D_PCamera cam);

PUBLIC void FSP_AddObj(sint32 x, sint32 y, sint32 z, word angle, int ntable,
                       FS3_PSprite sp, word tirerot, word tiredir);

//PUBLIC void FSP_DumpObjs(const byte (*trans)[256], int scrx, int scry);
PUBLIC void FSP_DumpObjs(F3D_PCamera cam, const byte (*trans)[256], int scrx, int scry);

#endif

// ------------------------------ FLSPRS.H ----------------------------

