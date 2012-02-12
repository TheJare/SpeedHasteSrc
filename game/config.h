// ----------------------------- CONFIG.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Game configuration file.

#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifndef _BASE_H_
#include <base.h>
#endif

enum {
    CFG_NO      = 0,        // Yes/no/detect options.
    CFG_YES     = 1,
    CFG_DETECT  = 2,

    CFG_KEYBOARD = 0,       // Player control methods.
    CFG_JOYA     = 1,
    CFG_JOYB     = 2,
    CFG_MOUSE    = 3,

    CFG_STRSIZE = 50
};

typedef struct {
        // Control options.
    byte  UseJoystick,
          UseMouse;

        // Game options.
    byte  Detail;
    char  Datafile[CFG_STRSIZE];

        // Sound options.
    char  MusicDevice[CFG_STRSIZE],
          FXDevice[CFG_STRSIZE];
    word  MusicPort,
          MusicIRQ,
          MusicDMA,
          MusicRate,
          MusicChannels,
          MusicVolume;
    bool  MusicOn;
    word  FXPort,
          FXIRQ,
          FXDMA,
          FXRate,
          FXChannels,
          FXVolume;
    bool  FXOn;

        // Calibration values.
    word  JoyAX[4],
          JoyAY[4],
          JoyBX[4],
          JoyBY[4];

        // Control method.
    byte  P1Control,
          P2Control;

        // Keyboard scancodes.
    byte  P1KeyUp,
          P1KeyDn,
          P1KeyLt,
          P1KeyRt,
          P1KeyGe;
    byte  P2KeyUp,
          P2KeyDn,
          P2KeyLt,
          P2KeyRt,
          P2KeyGe;

    word  NumLaps;

    int   nsongs;
    int   mus1;
    int   mus2;
    int   mus3;
    int   mus4;
    int   mus5;
    int   mus6;
    int   mus7;
    int   mus8;

    word  CarType;

    int   DecorationDetail,
          FloorDetail,
          WallsDetail,
          PolygonDetail,
          BackgroundDetail;

    char modeminit[CFG_STRSIZE],
         modemdial[CFG_STRSIZE],
         modemhang[CFG_STRSIZE];

    word ipxsocket,
         comnum,
         comaddr,
         comirq;

    char phone1[CFG_STRSIZE];
    char phone2[CFG_STRSIZE];
    char phone3[CFG_STRSIZE];
    char phone4[CFG_STRSIZE];
    char phone5[CFG_STRSIZE];
    char phone6[CFG_STRSIZE];

    int  SVGAOn;

    bool TimedRace;

} CFG_TConfig;

PUBLIC CFG_TConfig CFG_Config;

enum {
    CFG_NCARDS = 8
};

PUBLIC const char *CFG_Soundcards[CFG_NCARDS][2];

PUBLIC const char *CFG_FindCard(const char *card);

PUBLIC bool CFG_Load(const char *fname);

PUBLIC bool CFG_Save(const char *fname);

#endif

// ----------------------------- CONFIG.H -------------------------------

