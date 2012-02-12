// ----------------------------- LLMOUSE.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

/*
 * Init with LLM_Init(), de-init with LLM_End(). Init and Present return
 * FALSE if there's no mouse installed. GetState will return the button
 * states (bits 0, 1, 2 for buttons 1, 2, 3), and store the coordinates
 * in *x and *y, if they're not NULL.
 */

#ifndef _LLMOUSE_H_
#define _LLMOUSE_H_

#ifndef _BASE_H_
#include <base.h>
#endif

PUBLIC bool LLM_Init(void);
PUBLIC bool LLM_Present(void);
PUBLIC void LLM_ShowCursor(void);
PUBLIC void LLM_HideCursor(void);
PUBLIC word LLM_GetState(int *x, int *y);
PUBLIC word LLM_GetMovement(int *x, int *y);
PUBLIC void LLM_SetPosition(int x, int y);
PUBLIC void LLM_SetRange(int x, int y, int w, int h);
PUBLIC void LLM_SetSensitivity(int x, int y);
PUBLIC void LLM_End(void);

#endif

// ----------------------------- LLMOUSE.H -------------------------------

