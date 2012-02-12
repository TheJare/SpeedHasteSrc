// ------------------------------ TIMER.C ----------------------------
// For use with Watcom C 9.5 and DOS4GW
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include <timer.h>
#include <dos.h>
#include <vga.h>

volatile dword TIMER_Clock;
void (*TIMER_HookFunction)(void);
dword TIMER_Speed;

PRIVATE void __interrupt (__far *OldTimerISR)(void) = NULL;
PRIVATE volatile dword clock18;

PRIVATE void __interrupt __far TimerISR(void) {
    SetES(_DS());
//    VGA_PutColor(0, 63, 63, 63);
    if (TIMER_HookFunction != NULL)
        TIMER_HookFunction();
//    VGA_PutColor(0, 0, 0, 0);
    TIMER_Clock++;
    clock18 += TIMER_Speed;
    if (clock18 >= 0x10000) {
        clock18 -= 0x10000;
        _chain_intr(OldTimerISR);
    }
    outb(0x20, 0x20);
}

PUBLIC void TIMER_Init(dword speed) {
    if (OldTimerISR == NULL) {
        TIMER_HookFunction = NULL;
        clock18 = 0;
        TIMER_Clock = 0;
        TIMER_Speed = 0x10000;
        OldTimerISR = _dos_getvect(8);
        _dos_setvect(8, TimerISR);
        TIMER_SetSpeed(speed);
    }
}

PUBLIC void TIMER_SetSpeed(dword speed) {
    if (OldTimerISR != NULL) {
        TIMER_Speed = speed;
        outb(0x43,0x36);
        outb(0x40, (byte)(TIMER_Speed & 0xFF));
        outb(0x40, (byte)(TIMER_Speed >> 8));
    }
}

PUBLIC void TIMER_End(void) {
    if (OldTimerISR != NULL) {
        _dos_setvect(8, OldTimerISR);
        outb(0x43,0x36);
        outb(0x40, 0);
        outb(0x40, 0);
        OldTimerISR = NULL;
    }
}

// ------------------------------ TIMER.C ----------------------------

