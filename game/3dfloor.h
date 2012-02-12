// ------------------------------ 3DFLOOR.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _3DFLOOR_H_
#define _3DFLOOR_H_

#ifndef _BASE_H_
#include <base.h>
#endif

struct SEC_SSector;

typedef struct {
    sint32 radius;
    dword  h, focus;
    word   horizon;
    word   flags;
    bool   hiDetail;
} F3D_TCameraData, *F3D_PCameraData;

enum {
    F3DCF_STATIC = 0x0001
};

typedef struct {
    F3D_TCameraData data;
    uint32 x, y;
    word   angle;
    sint16 tilt;
    uint32 dx, dy;
    struct SEC_SSector *sec;
} F3D_TCamera, *F3D_PCamera;

enum {
    STDCAM_NORMAL,
    STDCAM_LOW,
    STDCAM_HIGH,
    STDCAM_STATIC,
};

PUBLIC const F3D_TCameraData F3D_StdCams1Player[];
PUBLIC const F3D_TCameraData F3D_StdCams2Player[];

PUBLIC F3D_PCameraData F3D_StdCams; // Points to one of the above.

 // ------------------------------------

PUBLIC void FL_SetMap(const byte *map[], const byte trans[]);
#pragma aux FL_SetMap parm [EAX] [EBX]

PUBLIC void FL_DrawRaster(byte *dest, int width,
                          uint32 x, uint32 y, uint32 dx, uint32 dy);
#pragma aux FL_DrawRaster parm [EDI] [EAX] \
                               [EBX] [ECX] [EDX] [ESI]

PUBLIC void FL_DrawRasterLo(byte *dest, int width,
                            uint32 x, uint32 y, uint32 dx, uint32 dy);
#pragma aux FL_DrawRasterLo parm [EDI] [EAX] \
                                 [EBX] [ECX] [EDX] [ESI]

PUBLIC void FL_DrawRasterTrans(byte *dest, int width,
                               uint32 x, uint32 y, uint32 dx, uint32 dy);
#pragma aux FL_DrawRasterTrans parm [EDI] [EAX] \
                                    [EBX] [ECX] [EDX] [ESI]

PUBLIC void FL_DrawRasterNoTile(byte *dest, int width,
                                uint32 x, uint32 y, uint32 dx, uint32 dy);
#pragma aux FL_DrawRasterNoTile parm [EDI] [EAX] \
                                     [EBX] [ECX] [EDX] [ESI]

 // ------------------------------------

PUBLIC void F3D_Draw3D(byte *dest, const byte *map[], int w, int h,
                       const F3D_PCamera cam, const byte (*trans)[256]);

PUBLIC void F3D_Draw2D(byte *dest, const byte *map[], int w, int h,
                       const F3D_PCamera cam, const byte trans[256]);

PUBLIC void F3D_Draw3DNoTile(byte *dest, const byte *map[], int w, int h,
                             const F3D_PCamera cam);


#endif

// ------------------------------ 3DFLOOR.H ----------------------------

