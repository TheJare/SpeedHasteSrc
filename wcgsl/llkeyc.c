// -------------------------- LLKEYC.C-------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana.
//
// Based on code bye Patch/Avalanche. Most of it is not modified, except
// the strange behaviour of the _numkeypress variable. Now it does reflect
// the real number of keys pressed (as far as the keyboard can tell, which
// by no means is complete: many combinations fail). To know how many keys
// have been pressed since some time, use the timer along with this.

#include <llkey.h>
#include <hard.h>

PUBLIC void __interrupt __far LLK_NewInt9(void);

HARD_TIRQHandler LLK_OldHandler = NULL;

void LLK_Init(void) {
    if (LLK_OldHandler == NULL) {
        LLK_OldHandler = HARD_GetIRQVector(1);
        HARD_SetIRQVector(1, &LLK_NewInt9);
    }
}

void LLK_End(void) {
    if (LLK_OldHandler != NULL) {
        HARD_SetIRQVector(1, LLK_OldHandler);
        LLK_OldHandler = NULL;
    }
}

// -------------------------- LLKEYC.C-------------------------

