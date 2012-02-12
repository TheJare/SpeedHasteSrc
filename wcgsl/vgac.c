// --------------------------- VGAC.C ------------------------------
// For use with WATCOM 9.5 + DOS4GW
// (C) Copyright 1993/4 by Jare & JCAB of Iguana.

#include <vga.h>
#include <i86.h>


byte VGA_GetMode(void) {
    inregs.w.ax = 0x0f00;
    int386( 0x10, &inregs, &outregs );
    return outregs.h.al;
}

void VGA_SetMode(byte mode) {
    inregs.w.ax = mode;
    int386( 0x10, &inregs, &outregs );
}

void VGA_Tweak(void) {
    outw(0x3C4, 0x0604);    // Turn on byte mode.
    outw(0x3D4, 0x0014);    // Turn off dword mode.
    outw(0x3D4, 0xE317);    // Dot clock.
}

void VGA_Set240(void) {
    VGA_Tweak();
    outb(0x3C2, 0xE3);           // Vertical sync. polarity.
    outw(0x3D4, 0x2C11);         // Turn off write protect.
    outw(0x3D4, 0x0D06);         // Vertical total.
    outw(0x3D4, 0x3E07);         // Overflow register.
    outw(0x3D4, 0xEA10);         // Vertical retrace start.
    outw(0x3D4, 0xAC11);         // Vertical retrace end and wr.prot.
    outw(0x3D4, 0xDF12);         // Vertical display enable end.
    outw(0x3D4, 0xE715);         // Start vertical blanking.
    outw(0x3D4, 0x0616);         // End vertical blanking.
}

void VGA_Set360(void) {
    outw(0x3C4, 0x0604);    // Turn on byte mode.
    outw(0x3D4, 0x0014);    // Turn off dword mode.
    outw(0x3D4, 0xE317);    // Dot clock.
    outb(0x3C2, 0xE7);      // Vertical sync. polarity.
    outw(0x3C4, 0x0300);    // Restart sequencer.

    outw(0x3D4, 0x2C11);    // Turn off write protect.
    outw(0x3D4, 0x6B00);    // Horizontal total.
    outw(0x3D4, 0x5901);    // Horz displayed
    outw(0x3D4, 0x5A02);    // Start horz blanking
    outw(0x3D4, 0x8E03);    // End horz blanking
    outw(0x3D4, 0x5E04);    // Start h sync
    outw(0x3D4, 0x8A05);    // End h sync
    outw(0x3D4, 0x0D06);    // Vertical total
    outw(0x3D4, 0x3E07);    // Overflow
    outw(0x3D4, 0x4009);    // Cell height
    outw(0x3D4, 0xEA10);    // V sync start
    outw(0x3D4, 0xAC11);    // V sync end and protect cr0-cr7
    outw(0x3D4, 0xDF12);    // Vertical displayed
    outw(0x3D4, 0x2D13);    // Offset
    outw(0x3D4, 0x0014);    // Turn off dword mode
    outw(0x3D4, 0xE715);    // V blank start
    outw(0x3D4, 0x0616);    // V blank end
    outw(0x3D4, 0xE317);    // Turn on byte mode
}


void VGA_UnTweak(void) {
    outw(0x3C4, 0x0E04);
    outw(0x3D4, 0x4014);
    outw(0x3D4, 0xA317);
}

void VGA_Set80x50(void) {
    outb(0x3D4, 9);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 0x07);
}

void VGA_Set16c(void) {
    byte k;
    inb(0x3DA);             // Clear flip-flop.
    outb(0x3C0, 0x10);
    k = inb(0x3C1);
    inb(0x3DA);             // Clear flip-flop.
    outb(0x3C0, 0x10);
    outb(0x3C0, k | 0x80);
    outb(0x3C0, 0x14);
    outb(0x3C0, 0);
    for (k = 0; k < 16; k++) {
        outb(0x3C0, k);     // Index
        outb(0x3C0, k);     // Value  (== index).
    }
    outb(0x3C0, 0x34);
    outb(0x3C0, 0);
}

void VGA_WaitForRetrace(void) {
    while ((inb(0x3DA) & 8) == 0);
}

void VGA_WaitForDisplay(void) {
    while ((inb(0x3DA) & 8) == 8);
}

void VGA_VSync(void) {
    VGA_WaitForDisplay();
    VGA_WaitForRetrace();
}

// ----- The rest is in VGAA.ASM, as they may be expected to be
//       very fast, and there's really no point in trying to use
//       C language all the time I guess.

// --------------------------- VGAC.C ------------------------------

