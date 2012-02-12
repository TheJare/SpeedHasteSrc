// --------------------------- VGA.H ------------------------------
// For use with WATCOM 9.5 + DOS4GW
// (C) Copyright 1993/4 by Jare & JCAB of Iguana.

// All-around nice screen stuff.
// Remember setting the Bitmask and PlaneMask when dealing with planar
// (tweaked or 16-color) modes. But also take into account that ET4000,
// for example, will not be able to write to the screen, even in plain
// Mode13h, if the PlaneMask is zeroed. So try to be careful, will you?
// Do not assume anything about the registers, and try to keep them stable
// EVEN in Mode13h. Some functions set them to all-1s or all-0s.

#ifndef _VGA_H_
#define _VGA_H_

#ifndef _BASE_H_
#include <base.h>
#endif

enum {
    VGA_BASEADDR   = 0xA0000,
    VGA_WIDTH      = 320,
    VGA_HEIGHT     = 200,
    VGA_SCREENSIZE = VGA_WIDTH*VGA_HEIGHT,
};

    // Regular mode 13h structure (320 bytes per line).
typedef byte  TVGA200x320[200][320];   // Access by (Y, X).

    // Tweaked mode structure (80 bytes per line).
typedef byte  TVGA800x80[800][80];     // The 200 or 800 really don't count.
typedef word  TVGA800x40[800][40];     // To access word at a time.
typedef dword TVGA800x20[800][20];     // To access dword at a time.

#define VGA200x320 (*((TVGA200x320*)VGA_BASEADDR))
#define VGA800x80  (*((TVGA800x80*)VGA_BASEADDR))
#define VGA800x40  (*((TVGA800x40*)VGA_BASEADDR))
#define VGA800x20  (*((TVGA800x20*)VGA_BASEADDR))

// ------------------------------------------------

    // Output a DAC register.
#define VGA_PutColor(c,r,g,b) {outb(0x3C8,(c)); outb(0x3C9,(r));   \
                               outb(0x3C9,(g)); outb(0x3C9,(b)); }
#define VGA_SetBorder(r,g,b) VGA_PutColor(0,(r),(g),(b))

    // Set some VGA common registers.
#define VGA_SetPlanes(p)    outw(0x3C4, ((p)<<8) + 0x02)
#define VGA_SetBitMask(p)   outw(0x3CE, ((p)<<8) + 0x08)
#define VGA_SetReadPlane(p) outw(0x3CE, ((p)<<8) + 0x04)

    // Video mode handling.
PUBLIC byte VGA_GetMode(void);
PUBLIC void VGA_SetMode(byte mode);
PUBLIC void VGA_Tweak(void);        // Tweak the already-set video mode.
PUBLIC void VGA_Set240(void);       // Turn Mode13h into ModeX 320x240.
PUBLIC void VGA_Set360(void);       // Turn Mode13h into ModeZ 360x480.
PUBLIC void VGA_UnTweak(void);      // Remove tweaking from the video mode.
PUBLIC void VGA_Set80x50(void);     // Set the vertical pixel size to 4 (8).
PUBLIC void VGA_Set16c(void);       // Remap the palette to the lower 16
                                    //  colors. 16-color modes are weird.

    // Polled synchronizing
PUBLIC void VGA_WaitForRetrace(void);
PUBLIC void VGA_WaitForDisplay(void);
PUBLIC void VGA_VSync(void);

    // Palette stuff. 'start' is the starting DAC reg #, 'n' is the amount of
    //   registers to process.
PUBLIC void VGA_GetPalette    (byte *dest, int start, int n);
PUBLIC void VGA_DumpPalette   (const byte *src,  int start, int n);
PUBLIC void VGA_ZeroPalette   (void);   // Total blackout

    // Palette buffer processing. 'src' is the palette as viewed right now,
    //   'dest' is where to leave the new palette, and 'target' is the final
    //   palette for a fade. 'src' and 'dest' can (usually will) be the same.
    //   Note- these only operate on the buffer, you must do a DumpPalette().

    // Fade all the buffer to a single rgb colour.
PUBLIC void VGA_FadeOutPalette(const byte *src,  byte *dst, int n, byte r, byte g, byte b);
    // Fade all the buffer towards another buffer.
PUBLIC void VGA_FadePalette   (const byte *src,  byte *dst, int n, const byte *target);

    // Fade all the buffer to a single rgb colour.
PUBLIC void VGA_FullFadeOutPalette(byte *dst, int n, byte r, byte g, byte b, int pos);
    // Fade all the buffer towards another buffer.
PUBLIC void VGA_FullFadePalette   (byte *dst, int n, const byte *target, int pos);

    // Some more register setting.
PUBLIC void VGA_SetDisplayPage(word off);
PUBLIC void VGA_SetSplitScreen(word off);
PUBLIC void VGA_SetCharHeight(byte height);

    // VGA memory processing, in Mode 13h (?) or planar.

    // VRAM-only functions. 'nbyt' is the number of bytes, so in mode
    // 13h the full screen is 64000, while in tweaked mode it's 16000.
    // For Mode 13h you will mostly use the <string.h> functions, because
    // this functions always affect the four planes.
PUBLIC void VGA_ClearPage(word off, dword nbyt);
PUBLIC void VGA_FillPage(word off, dword nbyt, byte val);
PUBLIC void VGA_CopyPage(word destoff, dword srcoff, dword nbyt);

    // (planar VRAM) <-> (Mode13h or planar RAM) copy functions.
    // A planar RAM buffer is four 16000 byte-long consecutive buffers
    // for each of the planes. For other purposes than very simple ones,
    // you may want to write custom versions of them.

    // Planar RAM to VRAM.
PUBLIC void VGA_CopyVirtScr2ScrMX(word destoff, byte *srcoff, dword nbyt);
    // VRAM to planar RAM. 4 Planes
PUBLIC void VGA_CopyVirtScr2ScrMY(word destoff, byte *srcoff, dword nbyt);
    // VRAM to planar RAM. 2 Planes.
PUBLIC void VGA_CopyScrMX2VirtScr(byte *dest, word srcoff, dword nbyt);
    // Mode13h RAM to VRAM. Dog slow.
PUBLIC void VGA_CopyRAM2Scr(word destoff, byte *srcoff, dword nbyt);
    // VRAM to Mode13h RAM. Dog slow.
PUBLIC void VGA_CopyScr2RAM(byte *destoff, word srcoff, dword nbyt);

// -------------------------------------------------------------------
// --- These are implemented in Assembler, so we force register parms.

#pragma aux VGA_GetPalette         parm caller [EDI] [EAX] [ECX]
#pragma aux VGA_DumpPalette        parm caller [ESI] [EAX] [ECX]
#pragma aux VGA_FadeOutPalette     parm caller [ESI] [EDI] [EAX] [EBX] [ECX] [EDX]
#pragma aux VGA_FadePalette        parm caller [ESI] [EDI] [EAX] [EBX]
#pragma aux VGA_FullFadeOutPalette parm caller [ESI] [EAX] [EBX] [ECX] [EDX] [EDI]
#pragma aux VGA_FullFadePalette    parm caller [ESI] [EAX] [EBX] [EDI]

#pragma aux VGA_SetDisplayPage     parm caller [EBX]
#pragma aux VGA_SetSplitScreen     parm caller [ECX]
#pragma aux VGA_ClearPage          parm caller [EDI] [ECX]
#pragma aux VGA_FillPage           parm caller [EDI] [ECX] [EBX]
#pragma aux VGA_CopyPage           parm caller [EDI] [ESI] [ECX]
#pragma aux VGA_CopyVirtScr2ScrMX  parm caller [EDI] [ESI] [ECX]
#pragma aux VGA_CopyVirtScr2ScrMY  parm caller [EDI] [ESI] [ECX]
#pragma aux VGA_CopyScrMX2VirtScr  parm caller [EDI] [ESI] [ECX]
#pragma aux VGA_CopyRAM2Scr        parm caller [EDI] [ESI] [ECX]
#pragma aux VGA_CopyScr2RAM        parm caller [EDI] [ESI] [ECX]

#pragma aux VGA_SetCharHeight      parm caller [ECX]

#endif

// --------------------------- VGA.H ------------------------------

