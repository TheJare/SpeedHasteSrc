// ----------------------------- CURSOR.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#include <cursor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <bitmap.h>
#include <vga.h>
#include <llkey.h>
#include <llmouse.h>
#include <text.h>
#include <llscreen.h>
#include <joystick.h>
#include <graphics.h>

#define VELINC 32

PRIVATE int PosX, PosY, VelX, VelY; // Vels are in 24.8 for smooth accel.
PRIVATE int LastMouseX, LastMouseY;
PRIVATE bool Initialized = FALSE;

bool CURS_ThereIsMouse, CURS_ThereIsJoy, CURS_JoyActive;
#define JOY_NDATA 4
PRIVATE int  JoyPos, JoyX[JOY_NDATA], JoyY[JOY_NDATA];
PRIVATE uint JoyB[JOY_NDATA];

         // Lower interval.     Upper interval.
PRIVATE int JMinX, JMinCenterX, JMaxCenterX, JMaxX,
            JMinY, JMinCenterY, JMaxCenterY, JMaxY;

    // Returns FALSE if no pointing device.
bool CURS_Init(bool mouse, bool joy) {
    if (!Initialized) {
        PosX = LLS_SizeX/2;
        PosY = LLS_SizeY/2;
        VelX = 0;
        VelY = 0;
        LLK_Init();
        CURS_ThereIsJoy = (joy && JOY_Read(NULL, NULL, NULL, NULL, NULL, NULL));
        CURS_JoyActive = FALSE;
        if (CURS_ThereIsJoy)
            CURS_Calibrate();
        CURS_ThereIsMouse = (mouse && LLM_Init());
        if (CURS_ThereIsMouse)
            LLM_GetState(&LastMouseX, &LastMouseY);
        Initialized = TRUE;
    }
    return TRUE;
}

PRIVATE bool DoCal(int *X, int *Y, int x, int y, const char *t1, const char *t2) {
    uint k;
    int i, jx, jy;

    for (i = y; i < (y+20); i++)
        memset(LLS_Screen[i] + x, 0, 283);
    GFX_Rectangle(x, y, 283, 20, 2, 0);
    TEXT_Write(&FONT_System, x + 2 + ((35-strlen(t1))/2)*8, y +  2, t1, 15);
    TEXT_Write(&FONT_System, x + 2 + ((35-strlen(t2))/2)*8, y + 10, t2, 14);
    LLS_Update();

    do {
        JOY_Read(NULL, NULL, NULL, NULL, &k, NULL);
        if (LLK_Keys[kESC]) {
            LLK_PressAnyKey();
            return FALSE;
        }
    } while (k != 0);
    JOY_Read(X, Y, NULL, NULL, &k, NULL);
    do {
        JOY_Read(&jx, &jy, NULL, NULL, &k, NULL);
        if (X != NULL) *X = (*X + jx)/2;        // Poor man's average.
        if (Y != NULL) *Y = (*Y + jy)/2;        // Poor man's average.
        if (LLK_Keys[kESC]) {
            LLK_PressAnyKey();
            return FALSE;
        }
    } while (k == 0);
    return TRUE;
}

     // Calibrates the pointer if necessary (if there's a joystick).
bool CURS_Calibrate(void) {
    if (CURS_ThereIsJoy) {
        BM_TBitmap *bg;
        int  x, y;

        CURS_JoyActive = FALSE;
        bg = NEW(sizeof(BM_TBitmapHeader) + 283*20);
        REQUIRE(bg != NULL);
        bg->Header.Width  = 283;
        bg->Header.Height = 20;
        bg->Header.Flags  = 0;
        x = (LLS_SizeX - bg->Header.Width)/2;
        y = (LLS_SizeY - bg->Header.Height)/2;

        BM_Get(x, y, bg);

        if (    DoCal(&JMinX, &JMinY, x, y,
                      "MOVE JOYSTICK TO UPPER LEFT CORNER",
                      "AND PRESS A BUTTON")
             && DoCal(&JMinCenterX, &JMinCenterY, x, y,
                      "CENTER JOYSTICK",
                      "AND PRESS A BUTTON")
             && DoCal(&JMaxX, &JMaxY, x, y,
                      "MOVE JOYSTICK TO LOWER RIGHT CORNER",
                      "AND PRESS A BUTTON")
             && DoCal(&JMaxCenterX, &JMaxCenterY, x, y,
                      "CENTER JOYSTICK AGAIN",
                      "AND PRESS A BUTTON")) {
            int i;
            JoyPos = 0;
            for (i = 0; i < JOY_NDATA; i++) {
                JoyX[i] = (JMaxCenterX + JMinCenterX)/2;
                JoyY[i] = (JMaxCenterY + JMinCenterY)/2;
                JoyB[i] = 0;
            }
            CURS_JoyActive = TRUE;
        }

        BM_Draw(x, y, bg);
        DISPOSE(bg);
        LLS_Update();
    } else
        return FALSE;
    return TRUE;
}

    // Returns the pointer states, and stores the pointer coordinates.
word CURS_GetPosition(int *x, int *y) {
    if (Initialized) {
        word but = 0;
        int incvx = 0, incvy = 0;

        while (kbhit())
            getch();

        if (CURS_ThereIsMouse) {
            int x, y;
            but |= LLM_GetMovement(&x, &y);
            PosX += x;
            PosY += y;
/*
            but |= LLM_GetState(&x, &y);
            x -= LastMouseX;
            y -= LastMouseY;
            if (x == 0 || y == 0) {
                LastMouseX = PosX*640/LLS_SizeX;
                LastMouseY = PosY*480/LLS_SizeY;
                LLM_SetPosition(LastMouseX, LastMouseY);
            } else {
                PosX += x;
                PosY += y;
                LastMouseX = x;
                LastMouseY = y;
            }
*/
        }

        if (CURS_JoyActive) {
            uint k;
            int  x, y, i;
            JOY_Read(&x, &y, NULL, NULL, &k, NULL);

            JoyX[JoyPos] = x;
            JoyY[JoyPos] = y;
            JoyB[JoyPos] = k;
            JoyPos = (JoyPos + 1) % JOY_NDATA;
            x = 0; y = 0; k = 0;
            for (i = 0; i < JOY_NDATA; i++) {
                x += JoyX[i];
                y += JoyY[i];
                k |= JoyB[i];
            }
            x /= JOY_NDATA;
            y /= JOY_NDATA;

            if (x < (JMinX + JMinCenterX)/2)
                incvx -= VELINC;
            if (x >= (JMaxCenterX + JMaxX)/2)
                incvx += VELINC;
            if (y < (JMinY + JMinCenterY)/2)
                incvy -= VELINC;
            if (y >= (JMaxCenterY + JMaxY)/2)
                incvy += VELINC;
            but |= k;
        }
        if (LLK_Keys[kUARROW])
            incvy -= VELINC;
        if (LLK_Keys[kDARROW])
            incvy += VELINC;
        if (LLK_Keys[kLARROW])
            incvx -= VELINC;
        if (LLK_Keys[kRARROW])
            incvx += VELINC;
        if (LLK_Keys[kENTER] || LLK_Keys[kKEYPADENTER] || LLK_Keys[kSPACE])
            but |= 1;
        if (LLK_Keys[kESC])
            but |= 2;

        if (incvx == 0)
            VelX = 0;
        if (incvy == 0)
            VelY = 0;
        VelX += incvx;
        VelY += incvy;
        PosX += (VelX/256);
        PosY += (VelY/256);

        if (PosX < 0)
            PosX = 0;
        if (PosX >= LLS_SizeX)
            PosX = LLS_SizeX-1;
        if (PosY < 0)
            PosY = 0;
        if (PosY >= LLS_SizeY)
            PosY = LLS_SizeY-1;
        if (x != NULL)
            *x = PosX;
        if (y != NULL)
            *y = PosY;
        return but;
/*
            uint k;
            int  x, y, i;
            JOY_Read(&x, &y, &k, NULL, NULL, NULL);

            JoyX[JoyPos] = x;
            JoyY[JoyPos] = y;
            JoyB[JoyPos] = k;
            JoyPos = (JoyPos + 1) % JOY_NDATA;
            x = 0; y = 0; k = 0;
            for (i = 0; i < JOY_NDATA; i++) {
                x += JoyX[i];
                y += JoyY[i];
                k |= JoyB[i];
            }
            x /= JOY_NDATA;
            y /= JOY_NDATA;

            if (x < (JMinX + JMinCenterX)/2)
                PosX -= 5;
            if (x >= (JMaxCenterX + JMaxX)/2)
                PosX += 5;
            if (y < (JMinY + JMinCenterY)/2)
                PosY -= 5;
            if (y >= (JMaxCenterY + JMaxY)/2)
                PosY += 5;
            but |= k;
        }
        if (LLK_Keys[kUARROW])
            PosY-=5;
        if (LLK_Keys[kDARROW])
            PosY+=5;
        if (LLK_Keys[kLARROW])
            PosX-=5;
        if (LLK_Keys[kRARROW])
            PosX+=5;
        if (LLK_Keys[kENTER] || LLK_Keys[kKEYPADENTER] || LLK_Keys[kSPACE])
            but |= 1;
        if (LLK_Keys[kESC])
            but |= 2;
        if (PosX < 0)
            PosX = 0;
        if (PosX >= LLS_SizeX)
            PosX = LLS_SizeX-1;
        if (PosY < 0)
            PosY = 0;
        if (PosY >= LLS_SizeY)
            PosY = LLS_SizeY-1;
        if (x != NULL)
            *x = PosX;
        if (y != NULL)
            *y = PosY;
        return but;
*/
    }
    return 0;
}

    // Sets the pointer coordinates.
void CURS_SetPosition(int x, int y) {
    if (Initialized) {
        PosX = x;
        PosY = y;
        if (PosX < 0)
            PosX = 0;
        if (PosX >= LLS_SizeX)
            PosX = LLS_SizeX-1;
        if (PosY < 0)
            PosY = 0;
        if (PosY >= LLS_SizeY)
            PosY = LLS_SizeY-1;
    }
}

    // Ends it all.
void CURS_End(void) {
    if (Initialized) {
//        LLK_End();
//        LLM_End();
        Initialized = FALSE;
    }
}

// ----------------------------- CURSOR.C -------------------------------

