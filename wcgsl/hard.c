// ----------------------------- HARD.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana.

#include <hard.h>

HARD_TIRQHandler HARD_GetIRQVector(int n) {
/*
    if (n >= 16)
        return NULL;
    if (n > 7)          // DOS4GW 1.95 won't handle IRQ's 8-15, but...
        n += 0x68;
*/
    inregs.x.eax = 0x3500 + n + 8;   /* DOS get vector */
    sregs.ds = sregs.es = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
    return (HARD_TIRQHandler)(MK_FP((word)sregs.es, outregs.x.ebx));

}

void HARD_SetIRQVector(int n, HARD_TIRQHandler vec) {
/*
    if (n >= 16)
        return;
    if (n > 7)          // DOS4GW 1.95 won't handle IRQ's 8-15, but...
        n += 0x68;
*/
    /*
        Install the new protected-mode vector.  Because IRQs
        are in the auto-passup range, its normal "passdown"
        behavior will change as soon as we install a
        protected-mode handler.  After this next call, when a
        real mode IRQ is generated, it will be resignalled
        in protected mode and handled by the new handler.
    */
    inregs.x.eax = 0x2500 + n + 8;   /* DOS set vector */
    /* DS:EDX == &handler */
    inregs.x.edx = FP_OFF (vec);
    sregs.ds = FP_SEG (vec);
    sregs.es = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
}

// ----------------------------- HARD.C -------------------------------

