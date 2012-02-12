// ------------------------------ GLOBALS.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <namelist.h>
#include <gamevtal.h>
#include <text.h>

#include "flsprs.h"
#include "cars.h"
#include "racemap.h"

#include "net.h"

// ------------------------------

enum {
    MAX_HUMAN_PLAYERS = 8,
};

// ------------------------------

    // Views on the console have this data.
typedef struct {
    F3D_TCamera c;
    F3D_TCameraData CamFrom, CamTo;
    int CamChangePos;
    THN_PThing thn;             // target.
} GL_TViewInfo, *GL_PViewInfo;

PUBLIC GL_TViewInfo Views[2];

PUBLIC byte     GamePal[768];
PUBLIC MAP_TMap Map;

    // TRUE if the start countdown has reached the end.
PUBLIC bool RaceStarted;
    // TRUE if the race is paused by pressing the pause key.
PUBLIC bool Paused;
    // TRUE if the race is stopped by an atomic operation (like
    // capturing a screen, entering a menu, etc.).
PUBLIC bool RaceStopped;

    // Detail level.
PUBLIC int  Detail;
    // Density of decoration objects. 0: none, 16: full.
PUBLIC int  DecorationDetail;

    // > means higher detail, < means lower.
PUBLIC int  FloorDetail;    // Double pixel floor or not.
PUBLIC int  WallsDetail;    // Textured walls or not.
PUBLIC int  PolygonDetail;  // Gouraud or not.
PUBLIC int  BackgroundDetail;  // Background or not.
PUBLIC int  SVGAOn;

PUBLIC int  ExtraDetail;    // For temporarily bettering quality (external views, etc).

    // If there are two views on the console.
PUBLIC bool SplitScreenMode;

    // ...
PUBLIC int  AdvanceTarget;
PUBLIC word TSTangle;
PUBLIC bool Trace;

    // Useful sanity-checking counter.
PUBLIC uint32 MagicNumba;

PUBLIC int UserPlayer;

    // Holds the list of sprite names loaded in the race.
PUBLIC NL_TNameTree GL_NameTree;

    // All-around time clock during the race.
PUBLIC dword GlobalClock;
    // Keeps the race's time (stopped while the race is stopped).
PUBLIC dword GameClock;

    // Motor sparks sprites.
PUBLIC FS3_PSprite Sparks[6];

    // Smoke sprites.
PUBLIC FS3_PSprite Smoke[6*3];

// ----------------

PUBLIC int GL_UseJoysticks;    // Bits 0, 1.

PUBLIC int GL_JoyAX, GL_JoyAY, GL_JoyABut, GL_JoyALastBut;
PUBLIC int GL_JoyBX, GL_JoyBY, GL_JoyBBut, GL_JoyBLastBut;

// ----------------

PUBLIC GAME_PRec   SH_Vtal;
PUBLIC XTRN_TStack SH_Stack;

PUBLIC void GL_PollMusic(void);

    // Nice freq will be 11025.
PUBLIC GAME_PEffect GL_LoadEffect(const char *fname, int freq, bool loop);
    // To unload use GAME_EFF_Unload(); or UnloadAll();

// ----------------

    // Number of capture file.
PUBLIC int  CaptureNumber;

    // Init the Capture stuff.
PUBLIC void GL_InitCapture(void);

    // Perform a capture if kscan is the capture key.
PUBLIC void GL_Capture(byte kscan);

// ----------------

PUBLIC byte *GL_ClrTable, *GL_ClrTableOrg;

PUBLIC int GL_SelCar[2*NET_MAXNODES], GL_SelCircuit;

PUBLIC dword GL_Seed;

PUBLIC int GL_CarType;

PUBLIC int GL_ConsolePlayers;

PUBLIC int GL_RaceLaps;     // Number of laps to race.

    // Timed race?
PUBLIC bool GL_TimedRace;

PUBLIC int GL_RaceTime[5];  // Time to give for each completed lap in " mode.

PUBLIC bool GL_Registered;

// ----------------
// Cheats

PUBLIC bool GL_NoCollide;

// ----------------

PUBLIC bool IPXPresent;
PUBLIC byte IPXLocalAddr[6];

// ----------------

PUBLIC FONT_TFont GL_YFont, GL_RFont, GL_WFont, GL_BFont, GL_GFont;

// ----------------

PUBLIC int GL_ScreenW, GL_ScreenH,              // Width and height of play area.
           GL_ScreenMinX, GL_ScreenMinY,        // Min. coordinates of play area.
           GL_ScreenMaxX, GL_ScreenMaxY,        // Max. coordinates of play area.
           GL_ScreenCenterX, GL_ScreenCenterY,  // Central coordinates of play area.
           GL_ScreenXRatio, GL_ScreenYRatio;    // Aspect ratio compared to 320x200, in 16.16 (Use FP16Mult).

#endif

// ------------------------------ GLOBALS.H ----------------------------

