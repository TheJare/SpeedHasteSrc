
#include "vbl.h"
#include <hard.h>
#include <vga.h>
#include <string.h>

    // Set this prior to using the module. By default it is set to
    // 0, meaning the VBL will have unrestricted access to the
    // retrace signal. If TRUE, the VBL will have to asume that it
    // can not sync correctly, and therefor revert to a flick-prone
    // pure timer mode and not wait for the retrace. The timer freq
    // will be at a rate of VBL_CompatibleMode frames, so set to 70
    // for 320x200-ratio modes and to 60 for 320x240-ratio modes.
    // BTW, remember that the timer cannot deliver less than 18.2 fps.
PUBLIC int VBL_CompatibleMode = 0;

extern void VBL_InitializeA(int half);
#pragma aux VBL_InitializeA parm [ESI]

volatile dword VBL_PageOff = 0;

PUBLIC void __interrupt __far VBL_Handler(void);
PUBLIC void __interrupt __far VBL_TestHandler(void);

extern HARD_TIRQHandler VBL_OldHandler;

PUBLIC bool VBL_Init(int nlines)
{
    int oldirqmask;

    if (VBL_OldHandler != NULL)
        return TRUE;

    if (VBL_Palette == NULL) {
        VBL_Palette = NEW(768*2);
        if (VBL_Palette == NULL)
            return FALSE;
        memset(VBL_Palette, 0, 2*768);
        VBL_OldPal = VBL_Palette + 768;
    }

    oldirqmask = inp(0x21);
    outp(0x21, oldirqmask | 1);
    _disable();
    outp(0x43, 0x36);
    outp(0x40,    0);
    outp(0x40,    0);

    VBL_OldHandler = HARD_GetIRQVector(0);
    HARD_SetIRQVector(0, &VBL_TestHandler);

    VBL_InitializeA(nlines);

    HARD_SetIRQVector(0, &VBL_Handler);

    outp(0x21, oldirqmask & 0xFE);
    _enable();
    VBL_Active = TRUE;

    VBL_FadePos = 0;

    return TRUE;
}

PUBLIC void VBL_Done(void)
{
    if (VBL_OldHandler != NULL) {
        _disable();
        HARD_SetIRQVector(0, VBL_OldHandler);
        VBL_OldHandler = NULL;
        outp(0x43, 0x36);
        outp(0x40,    0);
        outp(0x40,    0);
        _enable();
    }
}

#pragma off (check_stack)

PUBLIC int VBL_SyncTripleBuffer(int offs, int size, int nvbls)
{
    int n;

    n = VBL_VSync(nvbls);
    VBL_ChangePage(VBL_PageOff);
    VBL_PageOff = VBL_PageOff+size;
    if (VBL_PageOff >= offs+3*size)
        VBL_PageOff -= offs+3*size;
    return n;
}


volatile byte *VBL_DestPal = NULL;
volatile byte  VBL_DestRed = 0, VBL_DestGreen = 0, VBL_DestBlue = 0;
    // < 0 -> to rgb, == 0 -> nofade, > 0 -> to destpal.
volatile int   VBL_FadeSpeed = 0;
volatile int   VBL_FadeStartColor = 0, VBL_FadeNColors = 256;
volatile int   VBL_FadeMode = VBL_FADEFULL;
volatile int   VBL_FadePos  = 0;

PUBLIC void VBL_FadeHandler(void) {
    int i;

    if (VBL_FadePos > 0 && VBL_FadeSpeed > 0) {
        for (i = 0; i < VBL_FadeSpeed; i++) {
            if (VBL_FadePos > 64)
                VBL_FadePos = 64;
            if (VBL_FadeMode == VBL_FADEFAST) {
                if (VBL_DestPal != NULL)
                    VGA_FadePalette((byte*)VBL_Palette+3*VBL_FadeStartColor,
                                    (byte*)VBL_Palette+3*VBL_FadeStartColor,
                                    VBL_FadeNColors,
                                    (byte*)VBL_DestPal);
                else
                    VGA_FadeOutPalette((byte*)VBL_Palette+3*VBL_FadeStartColor,
                                       (byte*)VBL_Palette+3*VBL_FadeStartColor,
                                       VBL_FadeNColors,
                                       VBL_DestRed, VBL_DestGreen, VBL_DestBlue);
            } else {
                if (VBL_DestPal != NULL)
                    VGA_FullFadePalette((byte*)VBL_Palette+3*VBL_FadeStartColor,
                                        VBL_FadeNColors,
                                        (byte*)VBL_DestPal,
                                        VBL_FadePos);
                else
                    VGA_FullFadeOutPalette((byte*)VBL_Palette+3*VBL_FadeStartColor,
                                           VBL_FadeNColors,
                                           VBL_DestRed, VBL_DestGreen, VBL_DestBlue,
                                           VBL_FadePos);
            }
            VBL_FadePos++;
            if (VBL_FadePos > 64) {
                VBL_FadePos = 0;
                break;
            }
        }
        VBL_FirstColor = VBL_FadeStartColor;
        VBL_LastColor  = VBL_FadeStartColor + VBL_FadeNColors - 1;
    }
}

