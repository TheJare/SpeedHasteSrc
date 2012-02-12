// ----------------------- GAMEVTAL.C --------------------------
// VTAL Interface for games.
// (C) Copyright 1994-95 by JCAB of Iguana-VangeliSTeam.

#include <vtal.h>
#include <gamevtal.h>
#include <dpmi.h>

#include <stdlib.h>     // Need NULL defined
#include <conio.h>      // outp/inp inline functions.
#include <i86.h>        // _enable
#include <string.h>

#define PRIVATE static




// ********************************************************
// --------------------------------------------------------
// Local data.

PRIVATE uint32 GAME_Time = 0;  // For keeping an order of events in time.

typedef void (__interrupt __far *TIRQHandler)(void);

PRIVATE TIRQHandler OldIrq0Handler;    // Original IRQ0 handler.
PRIVATE GAME_PRec PollRec     = NULL;  // GAMEV structure used for timer mode.
PRIVATE volatile int IntCount = 0;     // Counter and increment used for
PRIVATE volatile int IntIncr  = 65536; // maintaining the system IRQ0 18.2 Hz rate.
PRIVATE XTRN_TStack GAME_Stack;        // Stack for polling from IRQs.




// ********************************************************
// --------------------------------------------------------
// Interrupt handler related functions.

PRIVATE void Handler(void)
{
    GAME_Poll(PollRec, 20);    // Poll sound up to 20 ticks.
}

PRIVATE void __far __interrupt __loadds Irq0Handler(void)
{
    // First, call the original handler when appropriate.

    IntCount += IntIncr;
    if(IntCount > 65536) {
        OldIrq0Handler();
        IntCount -= 65536;
    } else {
        outp(0x20, 0x20);
    }


    // Then, call the sound polling function.

    _enable();                              // Enable interrupts.
    XTRN_StackExec(&GAME_Stack, Handler);

}

// --------------------------
// Obtains the address of an IRQ handler.

PRIVATE TIRQHandler GetIRQVector(int n)
{
    struct SREGS sregs;
    union REGS inregs, outregs;

    inregs.x.eax = 0x3500 + n + 8;   // DOS4GW redefines the DOS get vector call.
    sregs.ds = sregs.es = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
    return (TIRQHandler)(MK_FP((uint16)sregs.es, outregs.x.ebx));

}

// --------------------------
// Sets the address of an IRQ handler.

PRIVATE void SetIRQVector(int n, TIRQHandler vec)
{
    struct SREGS sregs;
    union REGS inregs, outregs;

    inregs.x.eax = 0x2500 + n + 8;   // DOS set vector call.
    inregs.x.edx = FP_OFF (vec);
    sregs.ds     = FP_SEG (vec);     // Handler pointer.
    sregs.es     = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
}

// --------------------------
// Function to initialize the GAMEVTAL handler.

PRIVATE void InitHandler(void)
{
    if (GetIRQVector(0) != Irq0Handler) {   // If not already installed.

        XTRN_InitStack(&GAME_Stack, 8192);
                                           
        IntIncr = 1193180 / 50;

        outp(0x43, 54);
        outp(0x40, (uint8) IntIncr);
        outp(0x40, (uint8)(IntIncr>>8));    // Set the timer to 100 Hz.

        OldIrq0Handler = GetIRQVector(0);   // Get old handler.
        SetIRQVector(0, Irq0Handler);       // Set our handler.
    }
}

// --------------------------
// Function to uninitialize the GAMEVTAL handler.

PRIVATE void DoneHandler(void)
{
    outp(0x43, 54);     // Put the timer to 18.2 Hz.
    outp(0x40, 0);
    outp(0x40, 0);
                       
    if (GetIRQVector(0) == Irq0Handler)     // If it was installed.
        SetIRQVector(0, OldIrq0Handler);    // Uninstall it.
        
    XTRN_DoneStack(&GAME_Stack);
}




// ********************************************************
// --------------------------------------------------------
// Public functions.

// --------------------------
// Initializes an instance of GAMEVTAL.
// 'setup' is the desired values for it.

GAME_PRec PUBLICFUNC GAME_Init(GAME_PSetup setup)
{
    GAME_PRec rec;
    int       i;
    

    // Initialize and perform value checks...

    if (setup == NULL)
        return NULL;

    if (!VTAL_Init())
        return NULL;    // Table initialization failed.

    XTRN_GetMem(&rec, sizeof(*rec));    // Allocate structure memory.
    if (rec == NULL)
        return NULL;

    PollRec = NULL;     // Turn off POLL mode.


    // Setup channels for music and effects devices.

    rec->MusChannels = setup->MusicConfig.maxchans;
    rec->EffChannels = setup->EffectConfig.maxchans;

    // If they are the same device, mix all channels.

    if (setup->EffectDevice[0] == 0) {
        setup->MusicConfig.maxchans = setup->EffectConfig.maxchans =
                    rec->MusChannels + rec->EffChannels;
        rec->MusFirstChannel = rec->EffChannels;
    } else
        rec->MusFirstChannel = 0;
    rec->EffFirstChannel = 0;


    // Load music device.

    if (setup->MusicDevice[0] != 0) {
        rec->MusicDev = SDEV_Load(setup->MusicDevice, &setup->MusicConfig);
    } else {
        rec->MusicDev = NULL;
    }


    // Load effects device.

    if (setup->EffectDevice[0] != 0) {
        rec->EffectDev = SDEV_Load(setup->EffectDevice, &setup->EffectConfig);
    } else {
        rec->EffectDev = rec->MusicDev;
    }


    // Init the rest of values.

    memset(&rec->MusParamsChg, 0, sizeof(rec->MusParamsChg));
    memset(&rec->EffParamsChg, 0, sizeof(rec->EffParamsChg));
    rec->MusParams    = setup->MusicParams;
    rec->EffParams    = setup->EffectParams;
    rec->MusicVolume  = rec->MusParams.Volume;
    rec->EffectVolume = rec->EffParams.Volume;
    rec->MusFade      = GAME_FADEIN;
    rec->EffFade      = GAME_FADEIN;
    rec->MusTickCount = 0;
    rec->EffTickCount = 0;

    rec->PollMode     = TRUE;
    rec->SongPlaying  = FALSE;
    rec->Song         = NULL;
    rec->EffList      = NULL;

    XTRN_GetMem(&rec->EffChanList, sizeof(*rec->EffChanList)*rec->EffChannels);


    // If there was an error, abort.

    if (rec->EffChanList == NULL
     || rec->MusicDev    == NULL
     || rec->EffectDev   == NULL) {
        if (rec->EffChanList != NULL)
            XTRN_FreeMem(&rec->EffChanList, sizeof(*rec->EffChanList)*rec->EffChannels);
        if (rec->MusicDev != NULL)
            SDEV_Unload(rec->MusicDev);
        if (rec->EffectDev != NULL && rec->EffectDev != rec->MusicDev)
            SDEV_Unload(rec->EffectDev);
        XTRN_FreeMem(&rec, sizeof(*rec));
        return NULL;
    }


    // Finish init and return.

    memset(rec->EffChanList, 0, sizeof(*rec->EffChanList)*rec->EffChannels);

    rec->MusInfo    = SDEV_GetInfo(rec->MusicDev);
    rec->EffInfo    = SDEV_GetInfo(rec->EffectDev);
    rec->SoundTypes = rec->MusInfo->SoundTypes | rec->EffInfo->SoundTypes;
    
    for (i = 0; i < rec->EffChannels; i++) {
        rec->EffChanList[i].data = &rec->EffInfo->SoundData[i];
        rec->EffChanList[i].chg  = &rec->EffInfo->SoundChg[i];
    }

    return rec;
}


// --------------------------
// Uninitialization.

void PUBLICFUNC GAME_Done(GAME_PRec rec)
{
    if (rec == NULL)
        return;

    // Just unload and free all resourced that might be allocated.

    DoneHandler();      // In case the handler was runing.

    GAME_MUS_Stop(rec);
    GAME_EFF_StopAll(rec);
    GAME_SetMode(rec, FALSE);
    GAME_MUS_Unload(rec);
    GAME_EFF_UnloadAll(rec);
    XTRN_FreeMem(&rec->EffChanList, sizeof(*rec->EffChanList)*rec->EffChannels);

    if (rec->EffectDev != rec->MusicDev) {
        SDEV_Unload(rec->MusicDev);
        SDEV_Unload(rec->EffectDev);
    } else {
        SDEV_Unload(rec->MusicDev);
    }

    XTRN_FreeMem(&rec, sizeof(*rec));
    VTAL_Done();
}


// --------------------------
// Set the mode of operation. A value of 'timer = TRUE' means timer
// mode is requested. 'FALSE' means polling mode is requested.
// Returns the previous state of 'timer'.

uint PUBLICFUNC GAME_SetMode(GAME_PRec rec, uint mode)
{
    uint r;

    if (rec == NULL)
        return GAME_POLL;

    r = (PollRec != NULL)? GAME_TIMER: GAME_POLL;  // Previous state.

    if (mode == GAME_TIMER) {
        if (PollRec == NULL) {  // Init timer.
            PollRec = rec;
            InitHandler();
        }
    } else {
        if (PollRec != NULL)    // Uninit timer.
            DoneHandler();
        PollRec = NULL;
    }
    
    return r;       // Return previous state.
}


PRIVATE void PUBLICFUNC GAME_MUS_Poll(GAME_PRec rec)
{
    GSND_PData raw;
    GSND_TData draw;
    int        i;

    if (rec == NULL)
        return;

    rec->MusTickCount++;      // Increment counter.
        
    GSND_GenPlay(&rec->MusParams, &rec->MusParamsChg);

    // If the tick was processed by the device,
    // we can get the info for the next tick.

    if (rec->SongPlaying && rec->Player != NULL_handle) {

        // Advance the partiture interpreter.

        PLAY_DoPartiture(rec->Player);


        // Send sound to device.

        raw = rec->PlayInfo->RawData;   // Channel data pointer.

        for (i = 0; i < rec->PlayInfo->Song->NumChannels && i < rec->MusChannels; i++) {
            SDEV_DoChannel(rec->MusicDev, rec->MusFirstChannel+i, raw, &rec->MusParams);
            raw++;
        }
    } else {

        // Send silence to device.

        memset(&draw, 0, sizeof(draw));
        draw.Trigger = TRUE;
        
        for (i = 0; i < rec->MusChannels; i++)
            SDEV_DoRawChannel(rec->MusicDev, rec->MusFirstChannel+i, &draw);
    }


}


PRIVATE void PUBLICFUNC GAME_EFF_Poll(GAME_PRec rec)
{
    int i;

    if (rec == NULL)
        return;

    rec->EffTickCount++;      // Increment counter.

    GSND_GenPlay(&rec->EffParams, &rec->EffParamsChg);

    for (i = 0; i < rec->EffChannels; i++)
        SDEV_DoChannel(rec->EffectDev, rec->EffFirstChannel+i, rec->EffChanList[i].data, &rec->EffParams);
}


// --------------------------
// Polling functions. This is the function that must be called to
// keep the sound running. In fact, if it's not called often enough,
// the sound does some funny things (try it!).

void PUBLICFUNC GAME_Poll(GAME_PRec rec, int TimeOut)
{
    uint   j;            // Counters.
    uint   margin;
    static PollSema = 0; // Semaphore to avoid reentrancy.


    if (rec == NULL)
        return;

    if (PollSema != 0)      // Avoid reentry to this function.
        return;
    PollSema++;

    // תתתת Mix both loops.

    j = 0;

    margin = SDEV_GetSetMixMargin(rec->MusicDev, 0);
    SDEV_GetSetMixMargin(rec->MusicDev, margin);   // The first time,
                                                   // we use the default margin.
    while (j < TimeOut && SDEV_DoMix(rec->MusicDev)) {  // Process one tick

        j++;

        SDEV_GetSetMixMargin(rec->MusicDev, margin+1);  // The rest of the times,
                                                        // we use a bigger margin.
        GAME_MUS_Poll(rec);
        if (rec->MusicDev == rec->EffectDev)
            GAME_EFF_Poll(rec);
    }
    SDEV_GetSetMixMargin(rec->MusicDev, margin);

    // Do the same thing for the effects dev
    // if it's not the same as the music dev.

    if (rec->EffectDev != rec->MusicDev) {
        uint margin = SDEV_GetSetMixMargin(rec->EffectDev, 0);
        SDEV_GetSetMixMargin(rec->EffectDev, margin);   // The first time,
                                                        // we use the default margin.
        j = 0;
        while (j < TimeOut && SDEV_DoMix(rec->EffectDev)) {
            j++;
            SDEV_GetSetMixMargin(rec->EffectDev, margin+1);  // The rest of the times,
            GAME_EFF_Poll(rec);                              // we use a bigger margin.
        }
        SDEV_GetSetMixMargin(rec->EffectDev, margin);
    }

    PollSema--;         // Reset semaphore.
}




// ********************************************************
// --------------------------------------------------------
// Music handling public functions.

// --------------------------
// Load a music file. Returns TRUE if successful.

bool PUBLICFUNC GAME_MUS_Load(GAME_PRec rec, LPconststr fname)
{
    if (rec == NULL)
        return FALSE;


    // If a song was previously loaded, unload it.

    if (rec->Song != NULL)
        GAME_MUS_Unload(rec);


    // Load the song file.

    rec->Song     = SONG_Load(fname, 0xFFFF|rec->SoundTypes);
    rec->SongInfo = SONG_GetInfo(rec->Song);
    rec->Player   = NULL_handle;

    return rec->Song != NULL;
}


// --------------------------
// Unloads the previously loaded music.

void PUBLICFUNC GAME_MUS_Unload(GAME_PRec rec)
{
    if (rec == NULL)
        return;

    GAME_MUS_Stop(rec);     // Stop in case it was running.

    if (rec->Song != NULL) {
        SONG_Unload(rec->Song); // Unload it.
        rec->Song = NULL;
    }
}


// --------------------------
// Start playing the music. 'id' contains the values
// desired for the music.

void PUBLICFUNC GAME_MUS_Start(GAME_PRec rec, PLAY_PInitData id)
{
    if (rec == NULL || rec->Song == NULL || rec->Player != NULL
     || rec->SongPlaying || id == NULL)
        return;


    // Init with the tick rate calculated by the device.

    id->RealTimerVal = rec->MusInfo->RealTickRate;


    // Init the player.

    SONG_BindToDevice(rec->Song, rec->MusicDev);
    rec->Player   = PLAY_InitSong(rec->Song, id);
    rec->PlayInfo = PLAY_GetInfo(rec->Player);
    rec->SongPlaying = TRUE;
}


// --------------------------
// Stop playing the music. This totally stops it. If you want
// to pause the music, you should set 'rec->SongPlaying = FALSE'

void PUBLICFUNC GAME_MUS_Stop(GAME_PRec rec)
{
    if (rec == NULL)
        return;

    if (rec->Player == NULL_handle)
        return;
         
    rec->SongPlaying = FALSE;
    PLAY_DoneSong(rec->Player);
    rec->Player = NULL_handle;
    SONG_UnbindToDevice(rec->Song, rec->MusicDev);
}


// --------------------------
// Change the volume of the music.

void PUBLICFUNC GAME_MUS_ChangeVolume(GAME_PRec rec, uint vol)
{
    if (rec == NULL)
        return;

    rec->MusicVolume = vol;
    rec->MusParamsChg.Flags |= GSND_GC_TOVOLUME;
    if (rec->MusFade == GAME_FADEIN)
        rec->MusParamsChg.Volume = vol;
}


// --------------------------
// Change the fading of the music.

void PUBLICFUNC GAME_MUS_SetFading(GAME_PRec rec, uint in, uint time)
{
    if (rec == NULL)
        return;                        

    if (in == GAME_FADEIN) {
        rec->MusParamsChg.Volume  = rec->MusicVolume;
        rec->MusParamsChg.VolTime = time;
        rec->MusParamsChg.Flags  |= GSND_GC_TOVOLUME;
        rec->MusFade = GAME_FADEIN;
    } else {
        rec->MusParamsChg.Volume  = 0;
        rec->MusParamsChg.VolTime = time;
        rec->MusParamsChg.Flags  |= GSND_GC_TOVOLUME;
        rec->MusFade = GAME_FADEOUT;
    }
}




// ********************************************************
// --------------------------------------------------------
// Sound effect handling public functions.


// --------------------------
// Load an effect.

GAME_PEffect PUBLICFUNC GAME_EFF_Load(GAME_PRec rec, SINS_PLoadRec lrec, uint rate)
{
    GAME_PEffect  eff;


    // Init values.

    if (rec == NULL || lrec == NULL)
        return NULL;

    XTRN_GetMem(&eff, sizeof(*eff));
    if (eff == NULL)
        return NULL;
    eff->Size    = sizeof(*eff);
    eff->EffRate = rate;

    eff->EffData = SINS_InitSound(lrec);


    // If there was an error, abort.

    if (eff->EffData == NULL) {
        XTRN_FreeMem(&eff, sizeof(*eff));
        return NULL;
    }


    // Send it to the device.

    SDEV_InitSound(rec->EffectDev, eff->EffData);


    // Insert in a linked list, so we can later unload all of them at once.

    if (rec->EffList != NULL)
        rec->EffList->Prev = eff;
    eff->Next    = rec->EffList;
    eff->Prev    = NULL;
    eff->Head    = &rec->EffList;
    rec->EffList = eff;

    return eff;
}


// --------------------------
// Unload an effect from memory.

void PUBLICFUNC GAME_EFF_Unload(GAME_PRec rec, GAME_PEffect eff)
{
    if (rec == NULL || eff == NULL)
        return;


    // Unlink from the list.

    if (eff->Next != NULL)
        eff->Next->Prev = eff->Prev;
    if (eff->Prev != NULL)
        eff->Prev->Next = eff->Next;
    else if (eff->Head != NULL)
        *eff->Head = eff->Next;


    // Unload and free the structure.

    SDEV_DoneSound(rec->EffectDev, eff->EffData);
    SINS_DoneSound(eff->EffData);
    XTRN_FreeMem(&eff, sizeof(*eff));
}


// --------------------------
// Unload all effects.

void PUBLICFUNC GAME_EFF_UnloadAll(GAME_PRec rec)
{
    if (rec == NULL)
        return;

    while(rec->EffList != NULL)
        GAME_EFF_Unload(rec, rec->EffList);
}


// --------------------------
// Start an effect in an effect channel, with the values specified.
// 'rate' is specified as a fixed point (8 bit frational)
// multiplier to the natural frequency of the effect.
// 'vol' is a volume being 256 the standard value and 512 the max.
// 'pan' is:    0 - front
//            -64 - left
//             64 - right
//            128 \
//           -128 - rear (or front if surround isn't activated.
// Values in between are used if fine panning is activated.
// If the channel specified is -1, it looks for a channel to play.
// Returns the channel used or -1 if error.

sint PUBLICFUNC GAME_EFF_Start(GAME_PRec rec, sint chan, ulong rate, uint vol, sint pan, GAME_PEffect eff)
{
    GAME_PEffChan   ec;
//    SDEV_TChanData   cd;
    int              i;

    if (rec == NULL || chan < -1 || chan >= rec->EffChannels || eff == NULL)
        return -1;

    if (chan == -1) {
        for (chan = 0; chan < rec->EffChannels; chan++)
            if ((!rec->EffChanList[chan].Locked) &&
                SDEV_ChannelFree(rec->EffectDev, chan))
                break;
        if (chan == rec->EffChannels) {
            uint32 l = GAME_Time;
            chan = 0;
            for (i = 0; i < rec->EffChannels; i++)
                if ((!rec->EffChanList[i].Locked) &&
                    rec->EffChanList[i].TimeLaunched < l) {
                    chan = i;
                    l = rec->EffChanList[i].TimeLaunched;
                }
        }
    }
    if (chan < 0)
        return -1;

    if (rec->EffChanList[chan].Locked)
        return -1;


    // Init Channel list values.

    ec = rec->EffChanList + chan;
    ec->TimeLaunched = GAME_Time;
    ec->Freq    = 13900U*0x40000U / (uint)(rate*eff->EffRate);
    ec->Volume  = vol;
    ec->Panning = pan;
    ec->eff     = eff;


    // Trigger the effect in the channel.

    ec->data->Period     = SINS_AdjPeriod(eff->EffData, 13900U*0x40000U / (uint)(rate*eff->EffRate));
    ec->data->Volume     = vol;
    ec->data->Panning    = pan;
    ec->data->SOffset    = 0;
    ec->data->Instrument = eff->EffData;
    ec->data->Trigger    = TRUE;

    ec->chg->Flags       = 0;

    GAME_Time++;   // Keep an order of events.

    return chan;
}


// --------------------------
// Stops playing an effect in a channel.

void PUBLICFUNC GAME_EFF_Stop(GAME_PRec rec, sint chan)
{
    if (rec == NULL)
        return;

    if (chan < 0 || chan >= rec->EffChannels)
        return;

    rec->EffChanList[chan].eff              = NULL;
    rec->EffChanList[chan].data->Instrument = NULL;
    rec->EffChanList[chan].data->Trigger    = TRUE;
}


// --------------------------
// Stops playing all effects.

void PUBLICFUNC GAME_EFF_StopAll(GAME_PRec rec)
{
    int i;

    if (rec == NULL)
        return;

    for (i = 0; i < rec->EffChannels; i++)
        GAME_EFF_Stop(rec, i);
}


// --------------------------
// Change the global effects volume.

void PUBLICFUNC GAME_EFF_ChangeVolume(GAME_PRec rec, uint vol)
{
    if (rec == NULL)
        return;

    rec->EffectVolume = vol;
    rec->EffParamsChg.Flags |= GSND_GC_TOVOLUME;
    if (rec->EffFade == GAME_FADEIN)
        rec->EffParamsChg.Volume = vol;
}


// --------------------------
// Lock a channel so it can't be choosed when
// looking for a free effect channel.

void PUBLICFUNC GAME_EFF_LockChannel(GAME_PRec rec, sint chan)
{
    if (rec == NULL || chan < 0 || chan >= rec->EffChannels)
        return;

    rec->EffChanList[chan].Locked = TRUE;
}


// --------------------------
// Unlock a previously locked channel.

void PUBLICFUNC GAME_EFF_UnlockChannel(GAME_PRec rec, sint chan)
{
    if (rec == NULL || chan < 0 || chan >= rec->EffChannels)
        return;


    rec->EffChanList[chan].Locked = FALSE;
}


// --------------------------
// Change the volume of an effect channel on the fly.

void PUBLICFUNC GAME_EFF_SetChannelVolume(GAME_PRec rec, sint chan, uint vol, uint time)
{
    if (rec == NULL || chan < 0 || chan >= rec->EffChannels)
        return;                        

    rec->EffChanList[chan].chg->Volume   = vol;
    rec->EffChanList[chan].chg->VolTime  = time;
    rec->EffChanList[chan].chg->Flags   |= GSND_SC_TOVOLUME;
}


// --------------------------
// Change the frequency of an effect channel on the fly.
// 'rate' is specified as a fixed point (8 bit frational)
// multiplier to the natural frequency of the effect.

void PUBLICFUNC GAME_EFF_SetChannelFreq(GAME_PRec rec, sint chan, uint rate, uint time)
{
    if (rec == NULL || chan < 0 || chan >= rec->EffChannels
     || rec->EffChanList[chan].eff == NULL)
        return;

    rec->EffChanList[chan].chg->Period   = 13900U*0x40000U / (uint)(rate*rec->EffChanList[chan].eff->EffRate);
    rec->EffChanList[chan].chg->PerTime  = time;
    rec->EffChanList[chan].chg->Flags   |= GSND_SC_TOPERIOD;
}


// --------------------------
// Change the panning of an effect channel on the fly.

void PUBLICFUNC GAME_EFF_SetChannelPanning(GAME_PRec rec, sint chan, uint pan, uint time)
{
    if (rec == NULL || chan < 0 || chan >= rec->EffChannels)
        return;                        

    rec->EffChanList[chan].chg->Panning  = pan;
    rec->EffChanList[chan].chg->PanTime  = time;
    rec->EffChanList[chan].chg->Flags   |= GSND_SC_TOPANNING;
}


// --------------------------
// Change the fading of the effects.

void PUBLICFUNC GAME_EFF_SetFading(GAME_PRec rec, uint in, uint time)
{
    if (rec == NULL)
        return;                        

    if (in == GAME_FADEIN) {
        rec->EffParamsChg.Volume  = rec->EffectVolume;
        rec->EffParamsChg.VolTime = time;
        rec->EffParamsChg.Flags  |= GSND_GC_TOVOLUME;
        rec->EffFade = GAME_FADEIN;
    } else {
        rec->EffParamsChg.Volume  = 0;
        rec->EffParamsChg.VolTime = time;
        rec->EffParamsChg.Flags  |= GSND_GC_TOVOLUME;
        rec->EffFade = GAME_FADEOUT;
    }
}


PUBLIC GAME_PBufRec PUBLICFUNC GAME_EFF_InitBuffered(GAME_PRec rec, sint chan,
        SINS_PLoadRec lrec, uint rate, uint vol, sint pan)
{
    GAME_PBufRec brec;

    if (rec  == NULL || chan < -1 || chan >= rec->EffChannels
     || lrec == NULL || lrec->Types != SINS_TYP_DIGITAL)
        return NULL;

    XTRN_GetMem(&brec, sizeof(*brec));
    if (brec == NULL)
        return NULL;

    lrec->digiFileName  = NULL;
    lrec->digiFile      = NULL;
    lrec->digiPtr       = NULL;
    lrec->digiLoopStart = 0;
    lrec->digiLoopLen   = 0;
    lrec->digiSize      = 0;

    brec->Eff     = GAME_EFF_Load(rec, lrec, rate);
    if (brec->Eff == NULL) {
        XTRN_FreeMem(&brec, sizeof(*brec));
        return NULL;
    }
    brec->Channel = GAME_EFF_Start(rec, chan, 0x100, vol, pan, brec->Eff);
    if (brec->Channel == -1) {
        XTRN_FreeMem(&brec, sizeof(*brec));
        return NULL;
    }
    brec->Which   = 0;
    brec->Bufs[0].BufData = NULL;
    brec->Bufs[1].BufData = NULL;
    brec->Bufs[2].BufData = NULL;
    brec->Bufs[0].BufSize = 0;
    brec->Bufs[1].BufSize = 0;
    brec->Bufs[2].BufSize = 0;
    GAME_EFF_LockChannel(rec, brec->Channel);

    return brec;
}


PUBLIC void PUBLICFUNC GAME_EFF_DoneBuffered(GAME_PRec rec, GAME_PBufRec brec)
{
    if (rec == NULL || brec == NULL)
        return;

    GAME_EFF_UnlockChannel(rec, brec->Channel);
    GAME_EFF_Stop         (rec, brec->Channel);
    GAME_EFF_Unload       (rec, brec->Eff);
    XTRN_FreeMem(&brec, sizeof(*brec));
}


PUBLIC void LP PUBLICFUNC GAME_EFF_NewBuffer(GAME_PRec rec, GAME_PBufRec brec, void LP buf, uint size)
{
    void LP p;

    if (rec == NULL || brec == NULL)
        return NULL;

    brec->Bufs[brec->Which].BufData = buf;
    brec->Bufs[brec->Which].BufSize = size;
    if (!SDEV_NewBuffer(rec->EffectDev, brec->Channel+rec->EffFirstChannel,
                          &brec->Bufs[brec->Which]))
        return NULL;

    brec->Which = (brec->Which + 1) % 3;
    p = brec->Bufs[brec->Which].BufData;
    brec->Bufs[brec->Which].BufData = NULL;
    brec->Bufs[brec->Which].BufSize = 0;

    return p;
}


PUBLIC bool PUBLICFUNC GAME_EFF_NewBufferAvailable(GAME_PRec rec, GAME_PBufRec brec)
{
    if (rec == NULL || brec == NULL)
        return FALSE;

    return SDEV_BufferAvailable(rec->EffectDev, brec->Channel+rec->EffFirstChannel);
}


PUBLIC bool PUBLICFUNC GAME_EFF_BufferedFinished(GAME_PRec rec, GAME_PBufRec brec)
{
    if (rec == NULL || brec == NULL)
        return TRUE;

    if (!GAME_EFF_NewBufferAvailable(rec, brec))
        return FALSE;
    else
        return GAME_EFF_NewBuffer(rec, brec, NULL, 0) == NULL;
}


