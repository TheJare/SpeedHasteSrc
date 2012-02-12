// ----------------------------- LLSCREEN.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Screen access in Mode 13h.

#include <llscreen.h>
#include <stdlib.h>
#include <string.h>

#include <llvesa.h>

TVGA200x320 *LLS_Virtual1 = NULL,
            *LLS_Virtual2 = NULL;

TVGA200x320 *LLS_Screen_ = NULL;

int LLS_SizeX, LLS_SizeY, LLS_Size;
int LLS_Mode  = -1;
int LLS_VMode = -1;
int LLS_UpdateMinY =   0;
int LLS_UpdateMaxY = 200;

    // Initializes the module, in the given mode.
bool LLS_Init(int mode, int vmode) {
    assert(mode >= LLSM_DIRECT && mode <= LLSM_ADDITIVE);
//    if (LLS_Mode < 0) {
        switch (vmode) {
            case LLSVM_MODE13:
                LLS_SizeX = 320;
                LLS_SizeY = 200;
                VGA_SetMode(0x13);
                break;
            case LLSVM_MODEX:
                LLS_SizeX = 320;
                LLS_SizeY = 240;
                VGA_SetMode(0x13);
                VGA_ClearPage(0, 65536);
                VGA_Set240();
                VGA_ClearPage(0, 65536);
                break;
            case LLSVM_MODEY:
                LLS_SizeX = 320;
                LLS_SizeY = 200;
                VGA_SetMode(0x13);
                VGA_ClearPage(0, 65536);
                VGA_Tweak();
                VGA_ClearPage(0, 65536);
                break;
            case LLSVM_MODEZ:
                LLS_SizeX = 360;
                LLS_SizeY = 480;
                VGA_SetMode(0x13);
                VGA_ClearPage(0, 65536);
                VGA_Set360();
                VGA_ClearPage(0, 65536);
                break;
            case LLSVM_640x400x256:
//                if (!LLV_Init() || !LLV_SetMode(LLVM_640x400x256))
                if (!LLV_Init() || !LLV_SetModeRez(640, 400, 8))
                    return FALSE;
                LLS_SizeX = 640;
                LLS_SizeY = 400;
                break;
            case LLSVM_640x480x256:
//                if (!LLV_Init() || !LLV_SetMode(LLVM_640x480x256))
                if (!LLV_Init() || !LLV_SetModeRez(640, 480, 8))
                    return FALSE;
                LLS_SizeX = 640;
                LLS_SizeY = 480;
                break;
            case LLSVM_800x600x256:
//                if (!LLV_Init() || !LLV_SetMode(LLVM_800x600x256))
                if (!LLV_Init() || !LLV_SetModeRez(800, 600, 8))
                    return FALSE;
                LLS_SizeX = 800;
                LLS_SizeY = 600;
                break;
            case LLSVM_1024x768x256:
//                if (!LLV_Init() || !LLV_SetMode(LLVM_1024x768x256))
                if (!LLV_Init() || !LLV_SetModeRez(1024, 768, 8))
                    return FALSE;
                LLS_SizeX = 1024;
                LLS_SizeY = 768;
                break;
        }
        LLS_UpdateMinY = 0;
        LLS_UpdateMaxY = LLS_SizeY;
        LLS_Size = LLS_SizeX*LLS_SizeY;
        LLS_VMode = vmode;
        DISPOSE(LLS_Virtual1);
        DISPOSE(LLS_Virtual2);
        switch(mode) {
            case LLSM_DIRECT:
                LLS_Screen_ = VGA200x320;
                break;
            case LLSM_VIRTUAL:
                REQUIRE((LLS_Virtual1 = NEW(LLS_Size)) != NULL);
                memset(LLS_Virtual1, 0, LLS_Size);
                LLS_Screen_ = LLS_Virtual1;
                break;
            case LLSM_ADDITIVE:
                REQUIRE((LLS_Virtual1 = NEW(LLS_Size)) != NULL);
                REQUIRE((LLS_Virtual2 = NEW(LLS_Size)) != NULL);
                memset(LLS_Virtual1, 0, LLS_Size);
                memset(LLS_Virtual2, 0, LLS_Size);
//LLS_Virtual2 = 0xA0000;
                LLS_Screen_ = LLS_Virtual1;
                break;
        }
        LLS_Mode = mode;
//    }
    return TRUE;
}

    // Sets the given mode, and returns the old mode.
void LLS_SetMode(int mode, int vmode) {
    assert(mode >= LLSM_DIRECT && mode <= LLSM_ADDITIVE);
    if (vmode != LLS_VMode) {
        switch (vmode) {
            case LLSVM_MODE13:
                LLS_SizeX = 320;
                LLS_SizeY = 200;
                VGA_SetMode(0x13);
                break;
            case LLSVM_MODEX:
                LLS_SizeX = 320;
                LLS_SizeY = 200;
                VGA_SetMode(0x13);
                VGA_Set240();
                break;
            case LLSVM_MODEY:
                LLS_SizeX = 320;
                LLS_SizeY = 240;
                VGA_SetMode(0x13);
                VGA_Tweak();
                break;
            case LLSVM_MODEZ:
                LLS_SizeX = 360;
                LLS_SizeY = 480;
                VGA_SetMode(0x13);
                VGA_Set360();
                break;
            case LLSVM_640x400x256:
                if (!LLV_Init() || !LLV_SetModeRez(640, 400, 8))
                    return;
                LLS_SizeX = 640;
                LLS_SizeY = 400;
                break;
            case LLSVM_640x480x256:
                if (!LLV_Init() || !LLV_SetModeRez(640, 480, 8))
                    return;
                LLS_SizeX = 640;
                LLS_SizeY = 480;
                break;
            case LLSVM_800x600x256:
                if (!LLV_Init() || !LLV_SetModeRez(800, 600, 8))
                    return;
                LLS_SizeX = 800;
                LLS_SizeY = 600;
                break;
            case LLSVM_1024x768x256:
                if (!LLV_Init() || !LLV_SetModeRez(1024, 768, 8))
                    return;
                LLS_SizeX = 1024;
                LLS_SizeY = 768;
                break;
        }
        LLS_UpdateMinY = 0;
        LLS_UpdateMaxY = LLS_SizeY;
        LLS_Size = LLS_SizeX*LLS_SizeY;
        LLS_VMode = vmode;
    }
    if (mode != LLS_Mode) {
        switch (mode) {
            case LLSM_DIRECT:
                DISPOSE(LLS_Virtual1);
                DISPOSE(LLS_Virtual2);
                LLS_Screen_ = VGA200x320;
                break;
            case LLSM_VIRTUAL:
                DISPOSE(LLS_Virtual1);
                DISPOSE(LLS_Virtual2);
                REQUIRE((LLS_Virtual1 = NEW(LLS_Size)) != NULL);
                memset(LLS_Virtual1, 0, LLS_Size);
                LLS_Screen_ = LLS_Virtual1;
                break;
            case LLSM_ADDITIVE:
                DISPOSE(LLS_Virtual1);
                DISPOSE(LLS_Virtual2);
                REQUIRE((LLS_Virtual1 = NEW(LLS_Size)) != NULL);
                REQUIRE((LLS_Virtual2 = NEW(LLS_Size)) != NULL);
                memset(LLS_Virtual1, 0, LLS_Size);
                memset(LLS_Virtual2, 0, LLS_Size);
//LLS_Virtual2 = 0xA0000;
                LLS_Screen_ = LLS_Virtual1;
                break;
        }
        LLS_Mode = mode;
    }
}

    // Updates the physical screen with the contents of the virtual
    // screens (if applicable).
PUBLIC void LLS_Update(void) {
    extern void LLS_UpdateVGA(void);

    switch (LLS_VMode) {
        case LLSVM_640x400x256:
        case LLSVM_640x480x256:
        case LLSVM_800x600x256:
        case LLSVM_1024x768x256:
            if (LLS_Mode == LLSM_VIRTUAL)
//                LLV_PutRectangle(0, 0, LLS_SizeX, LLS_SizeY, (const byte*)LLS_Virtual1, LLS_SizeX);
                if (LLS_UpdateMinY < LLS_UpdateMaxY)
                    LLV_PutBuffer((const byte*)LLS_Virtual1 + LLS_UpdateMinY*LLS_SizeX,
                                  LLS_UpdateMinY, LLS_UpdateMaxY);
            break;
        default:
            LLS_UpdateVGA();
    }
}

    // De-initializes the module, frees all resources, etc.
void LLS_End(void) {
    if (LLS_Mode >= 0) {
        DISPOSE(LLS_Virtual1);
        DISPOSE(LLS_Virtual2);
        LLS_Mode = -1;
        VGA_SetMode(3);
    }
}

// ----------------------------- LLSCREEN.C -------------------------------

