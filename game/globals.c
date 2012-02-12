// ------------------------------ GLOBALS.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "globals.h"

#include <lbm.h>
#include <dos.h>
#include <stdio.h>
#include <llkey.h>
#include <llscreen.h>

GL_TViewInfo Views[2];

byte     GamePal[768];
MAP_TMap Map;

bool RaceStarted = FALSE;
bool Paused = FALSE;
bool RaceStopped = FALSE;

int  Detail = 3;
int  DecorationDetail = 16;
    // > means higher detail, < means lower.
int  FloorDetail = TRUE;    // Double pixel floor or not.
int  WallsDetail = TRUE;    // Textured walls or not.
int  PolygonDetail = TRUE;  // Gouraud or not.
int  BackgroundDetail = 10;  // Depth of drawn sectors.
int  SVGAOn = FALSE;

int  ExtraDetail = 0;

bool SplitScreenMode = FALSE;

int  AdvanceTarget = 0;
word TSTangle = 0;
bool Trace = FALSE;

int UserPlayer = 0;

uint32 MagicNumba = 0;

NL_TNameTree GL_NameTree = NULL;

dword GlobalClock = 0, GameClock = 0;

FS3_PSprite Sparks[6];

FS3_PSprite Smoke[6*3];

// ----------------

int GL_UseJoysticks = 0;    // Bits 0, 1.
int GL_JoyAX = 0, GL_JoyAY = 0, GL_JoyABut = 0, GL_JoyALastBut = 0;
int GL_JoyBX = 0, GL_JoyBY = 0, GL_JoyBBut = 0, GL_JoyBLastBut = 0;

// ----------------

GAME_PRec   SH_Vtal = NULL;
XTRN_TStack SH_Stack = { 0, 0};

PUBLIC void GL_PollMusic(void) {
    GAME_Poll(SH_Vtal, 2);
}

PUBLIC GAME_PEffect GL_LoadEffect(const char *fname, int freq, bool loop) {
    static SINS_TLoadRec  lrec;
    lrec.Types = SINS_TYP_DIGITAL;
    lrec.digiFileName       = fname;    // Load from a named file.
    lrec.digiFile           = NULL;
    lrec.digiPtr            = NULL;
    lrec.digiSize           = XTRN_FileSize(lrec.digiFileName);
    lrec.digiLoopStart      = 0;
    lrec.digiLoopLen        = loop? lrec.digiSize:0;
    lrec.digiAdjNum         = 0x2000;
    lrec.digiAdjDen         = 0x2000;
    lrec.digiSigned         = TRUE;
    lrec.digiBits16         = FALSE;
    lrec.digiLEndian        = FALSE;

    return GAME_EFF_Load(SH_Vtal, &lrec, freq);
}

// ----------------

    // Number of capture file.
int  CaptureNumber = 0;

    // Init the Capture stuff.
PUBLIC void GL_InitCapture(void) {
    struct find_t ft;
    int rez;

    CaptureNumber = 0;
    rez = _dos_findfirst("SPH*.PCX", _A_NORMAL, &ft);
    while (rez == 0) {
        int n;
        n = atoi(ft.name + 5);
        if (n >= CaptureNumber)
            CaptureNumber = n+1;
        rez = _dos_findnext(&ft);
    }
    _dos_findclose(&ft);
}

    // Perform a capture if kscan is the capture key.
PUBLIC void GL_Capture(byte kscan) {
    if (kscan == kSYSREQ || kscan == kPRTSC) {
        char buf[100];

        sprintf(buf, "SPH%.5d.PCX", CaptureNumber);
        if (PCX_Write(buf, LLS_SizeX, LLS_SizeY, GamePal, LLS_Screen[0]))
            CaptureNumber++;
    }
}

// -----------

int GL_SelCar[] = {0,0,0,0,0,0,0,0},
    GL_SelCircuit = 0;

int GL_CarType = 0;
int GL_ConsolePlayers = 1;

dword GL_Seed = 0x12345678;

    // Number of laps to race,
int GL_RaceLaps = 0;

    // Timed race?
bool GL_TimedRace = FALSE;

    // Time to give for each completed lap in timerace mode.
int GL_RaceTime[5] = {
    60*70,
    55*70,
    50*70,
    45*70,
    40*70,
};

byte *GL_ClrTable = NULL, *GL_ClrTableOrg = NULL;

PUBLIC bool GL_Registered = FALSE;

// ----------------
// Cheats

PUBLIC bool GL_NoCollide = FALSE;

// -----------

bool IPXPresent   = FALSE;
byte IPXLocalAddr[6];

// ----------------

FONT_TFont GL_YFont, GL_RFont, GL_WFont, GL_BFont, GL_GFont;

// ----------------

int GL_ScreenW, GL_ScreenH,              // Width and height of play area.
    GL_ScreenMinX, GL_ScreenMinY,        // Min. coordinates of play area.
    GL_ScreenMaxX, GL_ScreenMaxY,        // Max. coordinates of play area.
    GL_ScreenCenterX, GL_ScreenCenterY,  // Central coordinates of play area.
    GL_ScreenXRatio, GL_ScreenYRatio;    // Aspect ratio compared to 320x200, in 16.16 (Use FP16Mult).


// ------------------------------ GLOBALS.C ----------------------------
