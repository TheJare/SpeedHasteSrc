// ------------------------------ 3DFLOOR.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "3dfloor.h"
#include <sincos.h>
#include <llscreen.h>

#include "globals.h"

const F3D_TCameraData F3D_StdCams1Player[] = {
{
// ***** General view: not very high, not very low
    20165824,           // radius
    1848,               // h
    3136,              // focus
    1,                  // horizon
    0,                  // flags;
    TRUE,               // hiDetail
}, {
// ****** Low view: from the car
    0 << 20,            // radius
    1100,               // h
//    0x10E0,             // focus
    0x0BE0,             // focus
    1,                  // horizon
    0,                  // flags;
    TRUE,               // hiDetail
}, {
// ****** High view: like an helicopter
    40137344,          // radius
    2976,              // h
    3200,              // focus
    1,                  // horizon
    0,                  // flags;
    TRUE,               // hiDetail
}, {
// ****** Circuit Cameras
    0,                  // radius
    13920,              // h
    0xA80,              // focus
    8,                  // horizon
    F3DCF_STATIC,       // flags;
    TRUE,               // hiDetail
}
};

const F3D_TCameraData F3D_StdCams2Player[] = {
{
// ***** General view: not very high, not very low
    28000000,          // radius
    1720,               // h
    0xA00,              // focus
    1,                  // horizon
    0,                  // flags;
    TRUE,               // hiDetail
}, {
// ****** Low view: from the car
    0 << 20,            // radius
     1050,               // h
//    0x10E0,             // focus
    0x0B00,             // focus
    1,                  // horizon
    0,                  // flags;
    TRUE,               // hiDetail
}, {
// ****** High view: like an helicopter
    45000000,          // radius
     2770,              // h
    0xA00,              // focus
    1,                  // horizon
    0,                  // flags;
    TRUE,               // hiDetail
}, {
// ****** Circuit Cameras
    0,                  // radius
     5568,              // h
    0xA00,              // focus
    4,                  // horizon
    F3DCF_STATIC,       // flags;
    TRUE,               // hiDetail
}
};



F3D_PCameraData F3D_StdCams = F3D_StdCams1Player;

    // -------------------------------------------

#define PAL_LEVELS 16

PUBLIC void F3D_Draw3D(byte *dest, const byte *map[], int w, int h,
                       const F3D_PCamera cam, const byte (*trans)[256]) {
    byte *p;
    int   Y;

    assert(dest != NULL);
    assert(map != NULL);
    assert(cam != NULL);
    assert(w > 0);
    assert(h > 0);

    Y = cam->data.horizon;
    p = dest;
    while (h--) {
        sint32 vecX, vecY, posX, posY;
        dword rad;
        byte t;

        rad  = FPMultDiv(cam->data.h, cam->data.focus,Y);     // 1DE800 ª= 1 << 20
        if (rad < (4 << 16))
            t = PAL_LEVELS-1;
        else if (rad >= (36 << 16))
            t = 0;
        else
            t = PAL_LEVELS-2-(((PAL_LEVELS-2)*(rad-(4 << 16))/(36-4)) >> 16);
        FL_SetMap(map, trans[31-t]);
        vecX = FPMult(FPMultDiv(cam->data.h,0x1000,Y), Cos(cam->angle-16384));
        vecY = FPMult(FPMultDiv(cam->data.h,0x1000,Y), Sin(cam->angle-16384));
        posX = cam->x + FPnMult(rad/*0x100*/, (sint32)Cos(cam->angle), 22) - FPnMult(vecX,w,1);
        posY = cam->y + FPnMult(rad/*0x100*/, (sint32)Sin(cam->angle), 22) - FPnMult(vecY,w,1);
//        if (cam->data.hiDetail)
        if (FloorDetail)
            FL_DrawRaster(p, w, posX, posY, vecX, vecY);
//            FL_DrawRasterNoTile(p, w, posX, posY, vecX, vecY);
        else
            FL_DrawRasterLo(p, w, posX, posY, vecX, vecY);
        p += LLS_SizeX;
        Y++;
    }
}

PUBLIC void F3D_Draw3DNoTile(byte *dest, const byte *map[], int w, int h,
                             const F3D_PCamera cam) {
    byte *p;
    int   Y;

    assert(dest != NULL);
    assert(map != NULL);
    assert(cam != NULL);
    assert(w > 0);
    assert(h > 0);

    Y = cam->data.horizon;
    p = dest;
    FL_SetMap(map, NULL);
    while (h--) {
        sint32 vecX, vecY, posX, posY;
        dword rad;
        byte t;

        rad  = FPMultDiv(cam->data.h, cam->data.focus,Y);     // 1DE800 ª= 1 << 20
        vecX = FPMult(FPMultDiv(cam->data.h,0x1000,Y), Cos(cam->angle-16384));
        vecY = FPMult(FPMultDiv(cam->data.h,0x1000,Y), Sin(cam->angle-16384));
        posX = cam->x + FPnMult(rad/*0x100*/, (sint32)Cos(cam->angle), 22) - FPnMult(vecX,w,1);
        posY = cam->y + FPnMult(rad/*0x100*/, (sint32)Sin(cam->angle), 22) - FPnMult(vecY,w,1);
            FL_DrawRasterNoTile(p, w, posX, posY, vecX, vecY);
        p += LLS_SizeX;
        Y++;
    }
}

PUBLIC void F3D_Draw2D(byte *dest, const byte *map[], int w, int h,
                       const F3D_PCamera cam, const byte trans[256]) {
    byte *p;
    sint32 x, y, dx, dy;

    assert(dest != NULL);
    assert(map != NULL);
    assert(cam != NULL);
    assert(w > 0);
    assert(h > 0);

    FL_SetMap(map, trans);
    dx = FPMult(Cos(cam->angle-16384), cam->data.focus);
    dy = FPMult(Sin(cam->angle-16384), cam->data.focus);
//    x  = cam->x - FPnMult(dx, w, 1) + FPnMult(dy, 3*h, 2);
//    y  = cam->y - FPnMult(dy, h, 1) - FPnMult(dx, 3*w, 2);
    x  = cam->x - FPnMult(dx, w, 1) + FPnMult(dy, h, 1);
    y  = cam->y - FPnMult(dy, h, 1) - FPnMult(dx, w, 1);

    p = dest;
    while (h--) {
/*
        if (cam->data.hiDetail)
            FL_DrawRaster(p, w, x, y, dx, dy);
        else
            FL_DrawRasterLo(p, w, x, y, dx, dy);
*/
        FL_DrawRasterTrans(p, w, x, y, dx, dy);
        p += LLS_SizeX;
        x += -dy;
        y +=  dx;
    }
}

// ------------------------------ 3DFLOOR.C ----------------------------

