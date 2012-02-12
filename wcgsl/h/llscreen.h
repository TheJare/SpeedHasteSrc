// ----------------------------- LLSCREEN.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Screen access in Mode 13h.

#ifndef _LLSCREEN_H_
#define _LLSCREEN_H_

#include <vga.h>

    // Screen access modes.
enum {
    LLSM_DIRECT,        // Direct, the pointer points to 0xA0000.
    LLSM_VIRTUAL,       // There's a single RAM virtual screen.
    LLSM_ADDITIVE       // There's TWO virtual screens, and the
                        // actualization dumps just the differences.
};

    // Screen video modes.
enum {
    LLSVM_MODE13,       // 320x200 flat.
    LLSVM_MODEY,        // 320x200 planar.
    LLSVM_MODEX,        // 320x240 planar.
    LLSVM_MODEZ,        // 360x480 planar.
    LLSVM_640x400x256,  // Vesa mode.
    LLSVM_640x480x256,  // Vesa mode.
    LLSVM_800x600x256,  // Vesa mode.
    LLSVM_1024x768x256, // Vesa mode.
};

PUBLIC TVGA200x320 *LLS_Screen_;

PUBLIC int LLS_Mode;
PUBLIC int LLS_VMode;

PUBLIC int LLS_SizeX, LLS_SizeY, LLS_Size;

#define LLS_Screen (*LLS_Screen_)

PUBLIC int LLS_UpdateMinY;
PUBLIC int LLS_UpdateMaxY;


PUBLIC bool LLS_Init(int mode, int vmode);
    //      ********
    // Initializes the module, in the given mode.

PUBLIC void LLS_SetMode(int mode, int vmode);
    //     ***********
    // Sets the given mode.

PUBLIC void LLS_Update(void);
    //     **********
    // Updates the physical screen with the contents of the virtual
    // screens (if applicable).

PUBLIC void LLS_End(void);
    //      ********
    // De-initializes the module, frees all resources, etc.

#endif

// ----------------------------- LLSCREEN.H -------------------------------

