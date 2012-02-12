// ----------------------- GAMEVTAL.H --------------------------
// VTAL Interface for games.
// (C) Copyright 1994-95 by JCAB of Iguana-VangeliSTeam.

#ifndef _GAMEVTAL_H_
#define _GAMEVTAL_H_

#include <vtal.h>




// ********************************************************
// --------------------------------------------------------
// Data structures.


// --------------------------
// Game interface data structures.

typedef struct GAME_SEffect {

    struct GAME_SEffect   LP Next;
    struct GAME_SEffect   LP Prev;
    struct GAME_SEffect LPLP Head;
    uint                     Size;

    SINS_handle EffData;
    uint        EffRate;

} GAME_TEffect, LP GAME_PEffect;

typedef struct GAME_SEffChan {

    uint32        TimeLaunched;
    uint32        Freq;
    sint32        Volume;
    sint32        Panning;
    GAME_PEffect  eff;
    GSND_PData    data;
    GSND_PSndChg  chg;
    bool          Locked;

} GAME_TEffChan, LP GAME_PEffChan;

typedef struct GAME_SBufRec {

    GAME_PEffect Eff;
    sint         Channel;
    uint         Which;
    SDEV_TBuffer Bufs[3];

} GAME_TBufRec, LP GAME_PBufRec;

typedef struct GAME_SSetup {

    char          MusicDevice[80];
    SDEV_TConfig  MusicConfig;
    GSND_TGenData MusicParams;
    char          EffectDevice[80];
    SDEV_TConfig  EffectConfig;
    GSND_TGenData EffectParams;
    
} GAME_TSetup, LP GAME_PSetup;

typedef struct {

    GAME_TSetup    Setup;

    SDEV_handle     MusicDev;
    uint            MusicVolume;
    sint            MusFirstChannel;
    sint            MusChannels;
    SDEV_PInfo      MusInfo;
    GSND_TGenData   MusParams;
    GSND_TGenChg    MusParamsChg;
    volatile uint32 MusTickCount;
    sint            MusFade;
    SDEV_handle     EffectDev;
    uint            EffectVolume;
    sint            EffFirstChannel;
    sint            EffChannels;
    SDEV_PInfo      EffInfo;
    GSND_TGenData   EffParams;
    GSND_TGenChg    EffParamsChg;
    volatile uint32 EffTickCount;
    sint            EffFade;

    uint            SoundTypes;

    bool            PollMode;
    bool            SongPlaying;

    SONG_handle     Song;
    SONG_PInfo      SongInfo;
    PLAY_TInitData  PlayerID;
    PLAY_handle     Player;
    PLAY_PInfo      PlayInfo;

    GAME_PEffect    EffList;
    GAME_PEffChan   EffChanList;

} GAME_TRec, FAR *GAME_PRec;

enum {

    GAME_FADEOUT = 0,   // Fade to silence.
    GAME_FADEIN  = 1,   // Slide up the volume.

    GAME_POLL    = 0,   // Manual polling mode.
    GAME_TIMER   = 1    // Automatic timer mode.

};




// ********************************************************
// --------------------------------------------------------
// Function prototypes.

PUBLIC GAME_PRec    PUBLICFUNC GAME_Init   (GAME_PSetup setup);
PUBLIC void         PUBLICFUNC GAME_Done   (GAME_PRec rec);
PUBLIC uint         PUBLICFUNC GAME_SetMode(GAME_PRec rec, uint mode);
PUBLIC void         PUBLICFUNC GAME_Poll   (GAME_PRec rec, int TimeOut);

PUBLIC bool         PUBLICFUNC GAME_MUS_Load            (GAME_PRec rec, LPconststr fname);
PUBLIC void         PUBLICFUNC GAME_MUS_Unload          (GAME_PRec rec);
PUBLIC void         PUBLICFUNC GAME_MUS_Start           (GAME_PRec rec, PLAY_PInitData id);
PUBLIC void         PUBLICFUNC GAME_MUS_Stop            (GAME_PRec rec);
PUBLIC void         PUBLICFUNC GAME_MUS_ChangeVolume    (GAME_PRec rec, uint vol);
PUBLIC void         PUBLICFUNC GAME_MUS_SetFading       (GAME_PRec rec, uint in, uint time);

PUBLIC GAME_PEffect PUBLICFUNC GAME_EFF_Load             (GAME_PRec rec, SINS_PLoadRec lrec, uint rate);
PUBLIC void         PUBLICFUNC GAME_EFF_Unload           (GAME_PRec rec, GAME_PEffect eff);
PUBLIC void         PUBLICFUNC GAME_EFF_UnloadAll        (GAME_PRec rec);
PUBLIC sint         PUBLICFUNC GAME_EFF_Start            (GAME_PRec rec, uint chan, ulong rate, uint vol, sint pan, GAME_PEffect eff);
PUBLIC void         PUBLICFUNC GAME_EFF_Stop             (GAME_PRec rec, uint chan);
PUBLIC void         PUBLICFUNC GAME_EFF_StopAll          (GAME_PRec rec);
PUBLIC void         PUBLICFUNC GAME_EFF_ChangeVolume     (GAME_PRec rec, uint vol);
PUBLIC void         PUBLICFUNC GAME_EFF_LockChannel      (GAME_PRec rec, uint chan);
PUBLIC void         PUBLICFUNC GAME_EFF_UnlockChannel    (GAME_PRec rec, uint chan);
PUBLIC void         PUBLICFUNC GAME_EFF_SetChannelVolume (GAME_PRec rec, sint chan, uint vol,  uint time);
PUBLIC void         PUBLICFUNC GAME_EFF_SetChannelFreq   (GAME_PRec rec, sint chan, uint rate, uint time);
PUBLIC void         PUBLICFUNC GAME_EFF_SetChannelPanning(GAME_PRec rec, sint chan, uint pan,  uint time);
PUBLIC void         PUBLICFUNC GAME_EFF_SetFading        (GAME_PRec rec, uint in, uint time);

PUBLIC GAME_PBufRec PUBLICFUNC GAME_EFF_InitBuffered      (GAME_PRec rec, sint chan, SINS_PLoadRec lrec, uint rate, uint vol, sint pan);
PUBLIC void         PUBLICFUNC GAME_EFF_DoneBuffered      (GAME_PRec rec, GAME_PBufRec brec);
PUBLIC void LP      PUBLICFUNC GAME_EFF_NewBuffer         (GAME_PRec rec, GAME_PBufRec brec, void LP buf, uint size);
PUBLIC bool         PUBLICFUNC GAME_EFF_NewBufferAvailable(GAME_PRec rec, GAME_PBufRec brec);
PUBLIC bool         PUBLICFUNC GAME_EFF_BufferedFinished  (GAME_PRec rec, GAME_PBufRec brec);




#endif

