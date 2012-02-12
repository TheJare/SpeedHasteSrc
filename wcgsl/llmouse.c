// ----------------------------- LLMOUSE.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana.

#include <llmouse.h>

PRIVATE bool Present = FALSE;
PRIVATE bool Initialized = FALSE;
PRIVATE volatile int  PosX, PosY;
PRIVATE volatile word Buttons;

#ifdef MOUSEHANDLER

#pragma off (check_stack)
extern void _loadds far Handler(int max, int mcx, int mdx);
#pragma aux Handler parm caller [EAX] [ECX] [EDX]

PRIVATE void _loadds far Handler(int max, int mcx, int mdx)
{
    if (max & 1) {
        PosX    = mcx;
        PosY    = mdx;
    }
    if (max & 2)
        Buttons |= 1;
    if (max & 4)
        Buttons &= ~1;
    if (max & 8)
        Buttons |= 2;
    if (max & 16)
        Buttons &= ~2;
    if (max & 32)
        Buttons |= 4;
    if (max & 64)
        Buttons &= ~4;
}
#endif

bool LLM_Init(void) {
    if (!Initialized) {
#ifdef PMODEW
        Present = FALSE;
#else
        Buttons = 0;
        inregs.w.ax = 0;
        int386 (0x33, &inregs, &outregs);
        Present = (outregs.w.ax == (word) -1);
        if (Present) {
                // Set limits.
            inregs.w.ax = 0x8;
            inregs.x.ecx = 0;
            inregs.x.edx = 479;
            int386( 0x33, &inregs, &outregs);
#ifdef MOUSEHANDLER
                // Set Handler
            inregs.w.ax = 0xC;
            inregs.w.cx = 0x007F;
            inregs.x.edx = FP_OFF(&Handler);
            sregs.es     = FP_SEG(&Handler);
            int386x( 0x33, &inregs, &outregs, &sregs );
#endif
        }
#endif
        Initialized = TRUE;
    }
    return Present;
}

bool LLM_Present(void) {
    return Present;
}

PUBLIC void LLM_ShowCursor(void) {
    if (Present) {
        inregs.w.ax = 0x1;
        int386( 0x33, &inregs, &outregs);
    }
}

PUBLIC void LLM_HideCursor(void) {
    if (Present) {
        inregs.w.ax = 0x2;
        int386( 0x33, &inregs, &outregs);
    }
}

PRIVATE word GetButtons(void) {
#ifdef MOUSEHANDLER
        return Buttons;
#else
    if (Present) {
        inregs.w.ax = 0x3;
        int386( 0x33, &inregs, &outregs);
        return outregs.w.bx;
    } else
        return 0;
#endif
}


word LLM_GetState(int *x, int *y) {
    if (Present) {
#ifdef MOUSEHANDLER
        if (x != NULL)
            *x = PosX;
        if (y != NULL)
            *y = PosY;
        return Buttons;
#else
        inregs.w.ax = 0x3;
        int386( 0x33, &inregs, &outregs);
        if (x != NULL)
            *x = (short int)outregs.w.cx;
        if (y != NULL)
            *y = (short int)outregs.w.dx;
        return outregs.w.bx;
#endif
    } else {
        if (x != NULL)
            *x = 0;
        if (y != NULL)
            *y = 0;
        return 0;
    }
}

word LLM_GetMovement(int *x, int *y) {
    if (Present) {
        inregs.w.ax = 0xB;
        int386( 0x33, &inregs, &outregs);
        if (x != NULL)
            *x = (short int)outregs.w.cx;
        if (y != NULL)
            *y = (short int)outregs.w.dx;
    } else {
        if (x != NULL)
            *x = 0;
        if (y != NULL)
            *y = 0;
    }
    return GetButtons();
}

void LLM_SetPosition(int x, int y) {
    PosX = x;
    PosY = y;
    inregs.w.ax = 0x4;
    inregs.x.ecx = x;
    inregs.x.edx = y;
    int386( 0x33, &inregs, &outregs);
}

PUBLIC void LLM_SetRange(int x, int y, int w, int h) {
    if (w > 0) {
        inregs.w.ax = 0x7;
        inregs.x.ecx = x;
        inregs.x.edx = x+w-1;
        int386( 0x33, &inregs, &outregs);
    }
    if (h > 0) {
        inregs.w.ax = 0x8;
        inregs.x.ecx = y;
        inregs.x.edx = y+h-1;
        int386( 0x33, &inregs, &outregs);
    }
}

PUBLIC void LLM_SetSensitivity(int x, int y) {
    inregs.w.ax = 15;
    inregs.x.ecx = x;
    inregs.x.edx = y;
    int386( 0x33, &inregs, &outregs);
}

void LLM_End(void) {
    if (Initialized) {
        LLM_HideCursor();
#ifdef MOUSEHANDLER
        inregs.w.ax = 0;
        int386 (0x33, &inregs, &outregs);
#endif
        Present     = FALSE;
        Initialized = FALSE;
    }
}

// ----------------------------- LLMOUSE.C -------------------------------

