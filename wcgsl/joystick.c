// --------------------------- JOYSTICK.C ------------------------------
// For use with WATCOM 9.5 + DOS4GW
// (C) Copyright 1993/4 by Jare & JCAB of Iguana.

#include <joystick.h>
#include <dpmi.h>

#include <string.h>

enum {
    JOY_PORT = 0x201
};


bool JOY_Read(int *xa, int *ya, int *xb, int *yb, uint *ba, uint *bb) {
    byte axis = (yb != NULL)*8 + (xb != NULL)*4 + (ya != NULL)*2 + (xa != NULL);
    int count = 0;

    outp(JOY_PORT, 0);
    if (ba != NULL)
        *ba = ((inp(JOY_PORT) >> 4) ^ 15) & 3;
    if (bb != NULL)
        *bb = ((inp(JOY_PORT) >> 6) ^ 15) & 3;
    while (axis != 0) {
        byte k;
        k = (inp(JOY_PORT) & axis) ^ axis;
        if (k != 0) {
            if (k & 1)
                *xa = count;
            if (k & 2)
                *ya = count;
            if (k & 4)
                *xb = count;
            if (k & 8)
                *yb = count;
            axis &= k ^ 0xF;
        }
        count++;
        if (count > 0x10000)
            return FALSE;
    }
    return TRUE;
}

bool JOY_BIOSRead(int *xa, int *ya, int *xb, int *yb, uint *ba, uint *bb) {
    if (ba != NULL || bb != NULL) {
        DPMI_rmi.b.AH = 0x84;
        DPMI_rmi.w.DX = 0;
        DPMI_RealModeInt(0x15);
        if (ba != NULL)
            *ba = ((DPMI_rmi.b.AL >> 4) ^ 15) & 3;
        if (bb != NULL)
            *bb = ((DPMI_rmi.b.AL >> 6) ^ 15) & 3;
    }
    if (xa != NULL || ya != NULL || xb != NULL || yb != NULL) {
        DPMI_rmi.b.AH = 0x84;
        DPMI_rmi.w.DX = 1;
        DPMI_RealModeInt(0x15);
        if (xa != NULL)
            *xa = DPMI_rmi.w.AX;
        if (ya != NULL)
            *ya = DPMI_rmi.w.BX;
        if (xb != NULL)
            *xb = DPMI_rmi.w.CX;
        if (yb != NULL)
            *yb = DPMI_rmi.w.DX;
    }
    return TRUE;
}

// -----------------------------
// High level functions.
// These perform averaging of values to avoid multitaskers' interference.

#define NVALUES 4

PRIVATE bool UseBIOS = FALSE;
PRIVATE struct {
    int n, max;
    int x[NVALUES],
        y[NVALUES];
} JoyA, JoyB;

PRIVATE int NValues = NVALUES;

PUBLIC byte JOY_Init(bool bios, int nvalues) {
    int dummy, i;
    byte is;

    UseBIOS = bios;
    if (nvalues < 1)
        nvalues = 1;
    else if (nvalues > NVALUES)
        nvalues = NVALUES;
    nvalues = NValues;
    memset(&JoyA, 0, sizeof(JoyA));
    memset(&JoyB, 0, sizeof(JoyB));
    is  = JOY_Read(&dummy, NULL, NULL, NULL, NULL, NULL);
    is |= 2*JOY_Read(NULL, NULL, &dummy, NULL, NULL, NULL);
    for (i = 0; i < NValues; i++) {
        if (is&1)
            JOY_Get(&dummy, NULL, NULL, NULL, NULL, NULL);
        if (is&2)
            JOY_Get(NULL, NULL, &dummy, NULL, NULL, NULL);
    }
    return is;
}

PUBLIC void JOY_End(void);

PUBLIC bool JOY_Get(int *xa, int *ya, int *xb, int *yb, uint *ba, uint *bb) {
    int *dxa=NULL, *dya=NULL, *dxb=NULL, *dyb=NULL;
    bool rez;

    if (xa != NULL || ya != NULL) {
        dxa = JoyA.x + JoyA.n;
        dya = JoyA.y + JoyA.n;
    }
    if (xb != NULL || yb != NULL) {
        dxb = JoyB.x + JoyB.n;
        dyb = JoyB.y + JoyB.n;
    }
    if (UseBIOS)
        rez = JOY_BIOSRead(dxa, dya, dxb, dyb, ba, bb);
    else
        rez = JOY_Read(dxa, dya, dxb, dyb, ba, bb);
    if (xa != NULL || ya != NULL) {
        int x, y, i;
        JoyA.n = (JoyA.n+1)%NValues;
        x = JoyA.x[0];
        y = JoyA.y[0];
        for (i = 1; i < NValues; i++) {
            x += JoyA.x[i];
            y += JoyA.y[i];
        }
        x = x/NValues;
        y = y/NValues;
        if (xa != NULL) *xa = x;
        if (ya != NULL) *ya = y;
    }
    if (xb != NULL || yb != NULL) {
        int x, y, i;
        JoyB.n = (JoyB.n+1)%NValues;
        x = JoyB.x[0];
        y = JoyB.y[0];
        for (i = 1; i < NValues; i++) {
            x += JoyB.x[i];
            y += JoyB.y[i];
        }
        x = x/NValues;
        y = y/NValues;
        if (xb != NULL) *xb = x;
        if (yb != NULL) *yb = y;
    }
    return rez;
}

// -----------------------
// Calibrated reading.

PUBLIC word JOY_CalAX[4] = {0, 1, 2, 3};
PUBLIC word JOY_CalAY[4] = {0, 1, 2, 3};
PUBLIC word JOY_CalBX[4] = {0, 1, 2, 3};
PUBLIC word JOY_CalBY[4] = {0, 1, 2, 3};

PUBLIC bool JOY_CalGet(int wx, int wy, int *xa, int *ya, int *xb, int *yb, uint *ba, uint *bb) {
    int ax, ay, bx, by;
    int *dxa=NULL, *dya=NULL, *dxb=NULL, *dyb=NULL;
    bool rez;

    if (xa != NULL) dxa = &ax;
    if (ya != NULL) dya = &ay;
    if (xb != NULL) dxb = &bx;
    if (yb != NULL) dyb = &by;
    rez = JOY_Get(dxa, dya, dxb, dyb, ba, bb);
    if (xa != NULL) {
        if (ax < JOY_CalAX[1] && JOY_CalAX[1] - JOY_CalAX[0] > 0)
                *xa = (ax-JOY_CalAX[1])*wx/(JOY_CalAX[1] - JOY_CalAX[0]);
        else if (ax > JOY_CalAX[2] && JOY_CalAX[3] - JOY_CalAX[2] > 0)
                *xa = (ax-JOY_CalAX[2])*wx/(JOY_CalAX[3] - JOY_CalAX[2]);
        else
            *xa = 0;
    }
    if (ya != NULL) {
        if (ay < JOY_CalAY[1] && JOY_CalAY[1] - JOY_CalAY[0] > 0)
                *ya = (ay-JOY_CalAY[1])*wy/(JOY_CalAY[1] - JOY_CalAY[0]);
        else if (ay > JOY_CalAY[2] && JOY_CalAY[3] - JOY_CalAY[2] > 0)
                *ya = (ay-JOY_CalAY[2])*wy/(JOY_CalAY[3] - JOY_CalAY[2]);
        else
            *ya = 0;
    }
    if (xb != NULL) {
        if (bx < JOY_CalBX[1] && JOY_CalBX[1] - JOY_CalBX[0] > 0)
            *xb = (bx-JOY_CalBX[1])*wx/(JOY_CalBX[1] - JOY_CalBX[0]);
        else if (bx > JOY_CalBX[2] && JOY_CalBX[3] - JOY_CalBX[2] > 0)
            *xb = (bx-JOY_CalBX[2])*wx/(JOY_CalBX[3] - JOY_CalBX[2]);
        else
            *xb = 0;
    }
    if (yb != NULL) {
        if (by < JOY_CalBY[1] && JOY_CalBY[1] - JOY_CalBY[0] > 0)
            *yb = (by-JOY_CalBY[1])*wy/(JOY_CalBY[1] - JOY_CalBY[0]);
        else if (by > JOY_CalBY[2] && JOY_CalBY[3] - JOY_CalBY[2] > 0)
            *yb = (by-JOY_CalBY[2])*wy/(JOY_CalBY[3] - JOY_CalBY[2]);
        else
            *yb = 0;
    }
    return rez;
}

// --------------------------- JOYSTICK.C ------------------------------
