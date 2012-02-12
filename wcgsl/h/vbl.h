// -------------------------- VBL.H -------------------------
// For use with Watcom C 10.0 and DOS4GW
// Vertical blanking interrupt.
// (C) Copyright 1993-95 by Jare & JCAB of Iguana.

#ifndef _VBL_H_
#define _VBL_H_

#ifndef _BASE_H_
#include <base.h>
#endif


    // Set this prior to using the module. By default it is set to
    // 0, meaning the VBL will have unrestricted access to the
    // retrace signal. If TRUE, the VBL will have to asume that it
    // can not sync correctly, and therefor revert to a flick-prone
    // pure timer mode and not wait for the retrace. The timer freq
    // will be at a rate of VBL_CompatibleMode frames, so set to 70
    // for 320x200-ratio modes and to 60 for 320x240-ratio modes.
    // BTW, remember that the timer cannot deliver less than 18.2 fps.
PUBLIC int VBL_CompatibleMode;

PUBLIC bool VBL_Init (int half);
PUBLIC void VBL_Done (void);
PUBLIC int  VBL_VSync(int val);

PUBLIC void VBL_ChangePage      (int val);
PUBLIC void VBL_DumpPalette     (byte *src, int first, int size);
PUBLIC void VBL_ZeroPalette     (void);
PUBLIC int  VBL_SyncTripleBuffer(int offs, int size, int nvbls);
PUBLIC void VBL_FadeHandler     (void);


PUBLIC void VBL_RestoreSystemTime (void);

#pragma aux VBL_VSync       parm [EAX] value [EAX]
#pragma aux VBL_ChangePage  parm [EAX]
#pragma aux VBL_DumpPalette parm [ESI] [EDI] [ECX]


// -------------------------------------------------

extern void (* volatile VBL_HalfHandler)(void);
extern void (* volatile VBL_FullHandler)(void);

extern volatile byte *VBL_Palette;
extern volatile byte *VBL_OldPal;
extern volatile word  VBL_FirstColor;
extern volatile word  VBL_LastColor;
extern volatile dword VBL_PageOff;
extern volatile bool  VBL_Active;

    // Fade stuff. Set these variables to use FadeHandler.
enum {
    VBL_FADEFAST,
    VBL_FADEFULL
};
    // if VBL_DestPal == NULL fade to rgb.
extern volatile byte *VBL_DestPal;
extern volatile byte  VBL_DestRed, VBL_DestGreen, VBL_DestBlue;
extern volatile int   VBL_FadeSpeed;
extern volatile int   VBL_FadeStartColor, VBL_FadeNColors;
extern volatile int   VBL_FadeMode;
    // When set to 1 gets active.
extern volatile int   VBL_FadePos;



#endif
