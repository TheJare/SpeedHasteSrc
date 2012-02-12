
#define VERSION "1.00"

#include <llscreen.h>
#include <jclib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <sincos.h>
#include <llkey.h>
#include <strparse.h>
#include <text.h>
#include <textscr.h>
#include <gamevtal.h>
#include <atan.h>
#include <is2code.h>
#include <vertdraw.h>
#include <object3d.h>
#include <timer.h>
#include <polygon.h>
#include <joystick.h>
#include <ctype.h>

#include <vbl.h>
#include <llipx.h>

#include <sqrt.h>

#include "3dfloor.h"
#include "globals.h"
#include "userctl.h"
#include "hud.h"
#include "config.h"
#include "records.h"
#include "intro.h"
#include "menus.h"
#include "race.h"
#include "credits.h"
#include "models.h"

#include "net.h"

// ==========================================

#define QUIERO_LOG

#ifdef QUIERO_LOG

#include <dpmi.h>

dword GetFreeHeap(void) {
    void *p;
    int i, d;

    d = 8192;
    i = d;
    for (;;) {
        p = malloc(i);
        if (p == NULL) {
            i -= d;
            if (d <= 64)
                break;
            d = d / 2;
        }
        if (p != NULL)
            free(p);
        i += d;
    }
    return i-1024;
}

int GetFreeFiles(void) {
    FILE *f[200];
    int i, d;

    i = 0;
    for (;;) {
        f[i] = fopen(ArgV[0], "rb");
        if (f[i] == NULL)
            break;
        i++;
    }
    for (d = 0; d < i; d++)
        fclose(f[d]);
    return i;
}

extern void LogMemory(const char *text) {
    DPMI_TMemInfo mi;

    if (BASE_CheckArg("logmem") <= 0)
        return;
    printf("\nLogMemory: %s", text);
//    printf("\nBTW, free files now are %d", GetFreeFiles());
    printf("\n     free memory now is %.4fKb", ((double)GetFreeHeap())/1024.0);
/*
    DPMI_GetMemInfo(&mi);
    printf("\n   , free DPMI memory now is %.4fKb", ((double)mi.LargestBlockAvail)/1024.0);
    printf("\n     free DOS memory now is %.4fKb\n", ((double)DPMI_DOS_MaxMem())/1024.0);
*/
}

#else

extern void LogMemory(const char *text) {}

#endif

// ==========================================

void InitMusicSystem(void)
{
    static GAME_TSetup    setup;

    printf("VTAL Sound System Initialization.\n");

    REQUIRE(XTRN_InitStack(&SH_Stack, 8192));
    printf("  Interrupt stacks ok.\n");

    if (BASE_CheckArg("nosound") <= 0)
        strcpy(setup.MusicDevice, CFG_Config.MusicDevice);
    else
        setup.MusicDevice[0] = '\0';
    setup.EffectDevice[0]       =     0;
    setup.MusicConfig.port1     = CFG_Config.MusicPort;
    setup.MusicConfig.irq1      = CFG_Config.MusicIRQ;
    setup.MusicConfig.dma1      = CFG_Config.MusicDMA;
    setup.MusicConfig.dma2      = CFG_Config.MusicDMA;
    setup.MusicConfig.tickrate  = 256*70;
    setup.MusicConfig.rate      = CFG_Config.MusicRate;
    setup.MusicConfig.maxchans  = CFG_Config.MusicChannels;
    setup.MusicConfig.stereo    =  TRUE;
    setup.MusicConfig.bits16    =  TRUE;
    setup.MusicConfig.panning   =  TRUE;
    setup.MusicConfig.surround  =  TRUE;
    setup.EffectConfig.maxchans = CFG_Config.FXChannels;
    setup.MusicParams.Period    = 65536;
    if (CFG_Config.MusicOn)
        setup.MusicParams.Volume    = CFG_Config.MusicVolume;
    else
        setup.MusicParams.Volume    = 0;
    setup.MusicParams.PanWidth  =    60;
    setup.MusicParams.PanOffs   =     0;
    setup.EffectParams.Period   = 65536;
    if (CFG_Config.FXOn)
        setup.EffectParams.Volume   = CFG_Config.FXVolume;
    else
        setup.EffectParams.Volume   = 0;
    setup.EffectParams.PanWidth =   256;
    setup.EffectParams.PanOffs  =     0;

    SH_Vtal = GAME_Init(&setup);
    printf("Music system data area = 0x%X, Music driver handle = 0x%X\n",
        SH_Vtal,
        (SH_Vtal != NULL)?SH_Vtal->MusicDev:NULL);
    GAME_SetMode(SH_Vtal, GAME_TIMER);
    if (SH_Vtal != NULL)
        printf("  Sound device is %s. Interrupt mode set to ON.\n", CFG_FindCard(CFG_Config.MusicDevice));
}

void DoneMusicSystem(void) {
    GAME_SetMode(SH_Vtal, GAME_POLL);
    GAME_Done(SH_Vtal);
    XTRN_DoneStack(&SH_Stack);
}

// ----------------------------

PRIVATE bool BasicInitData(void) {
    byte joys;
    int  i;

    RND_Randomize(BIOS_Clock);

    if (BASE_CheckArg("novbl") > 0 || getenv("windir") != NULL)
        VBL_CompatibleMode = 70;
    else
        VBL_CompatibleMode = 0;

    if ( (i = BASE_CheckArg("ticks")) > 0)
        if (isdigit(ArgV[i][0])) NET_MaxTicks = atoi(ArgV[i]);
    if (NET_MaxTicks < 1)
        NET_MaxTicks = 1;

        // Global data created only once at the start of the program.
    InitSinCos();
    POLY_InitDivTable();
    printf("Global trig and divide table setup OK.\n");

    LLK_Init();
    LLK_DoChain = FALSE;
//    LLK_ChainChange = TRUE;
    LLK_ChainChange = FALSE;
    printf("Keyboard handler installed, ScrollLock to switch ON/OFF.\n");

    printf("Initializing joystick support...\n");
    if ( (i = BASE_CheckArg("joyval")) > 0) i = atoi(ArgV[i]);
    else                                    i = 1;
    joys = JOY_Init(BASE_CheckArg("joybios") > 0, i);
    if (joys & 1)
        printf("  Joystick A detected, Using %s input and %d levels.\n",
               (BASE_CheckArg("joybios") > 0)?"BIOS":"low-level", i);
    if (joys & 2)
        printf("  Joystick B detected, Using %s input and %d levels.\n",
               (BASE_CheckArg("joybios") > 0)?"BIOS":"low-level", i);
    memcpy(JOY_CalAX, CFG_Config.JoyAX, sizeof(JOY_CalAX));
    memcpy(JOY_CalAY, CFG_Config.JoyAY, sizeof(JOY_CalAY));
    memcpy(JOY_CalBX, CFG_Config.JoyBX, sizeof(JOY_CalBX));
    memcpy(JOY_CalBY, CFG_Config.JoyBY, sizeof(JOY_CalBY));

    printf("Loading global graphics:\n");
    FONT_Load(&FONT_Border, "fontbord.fnt");

    FONT_Load(&GL_YFont, "mfontyel.fnt");
    FONT_Load(&GL_RFont, "mfontred.fnt");
    FONT_Load(&GL_WFont, "mfontwhi.fnt");
    FONT_Load(&GL_BFont, "fontbord.fnt");
    FONT_Load(&GL_GFont, "fontbord.fnt");

    REQUIRE(JCLIB_Load("grafs.pal", GamePal, 768) == 768);
    REQUIRE(NL_Init(&GL_NameTree));
/*
    if (BASE_CheckArg("2player") > 0)
        SplitScreenMode = TRUE;
    else
        SplitScreenMode = FALSE;
*/
    if (SplitScreenMode)
        F3D_StdCams = F3D_StdCams2Player;
    else
        F3D_StdCams = F3D_StdCams1Player;

    GL_RaceLaps = CFG_Config.NumLaps;
    GL_TimedRace = CFG_Config.TimedRace;
    GL_CarType  = CFG_Config.CarType;

    DecorationDetail = CFG_Config.DecorationDetail;
    FloorDetail      = CFG_Config.FloorDetail     ;
    WallsDetail      = CFG_Config.WallsDetail     ;
    PolygonDetail    = CFG_Config.PolygonDetail   ;
    BackgroundDetail = CFG_Config.BackgroundDetail;
    SVGAOn           = CFG_Config.SVGAOn || (BASE_CheckArg("svga") > 0);

    {
        const char *clrf = "grafs.clr";
        int ofs, i;
        long l;

/*
        if ( (i = BASE_CheckArg("clr")) > 0)
            clrf = ArgV[i];
*/
        l = JCLIB_FileSize(clrf);
        if (l <= 0) {
            printf("Missing colormap file %s.\n", clrf);
            return FALSE;
        }
        printf("  Colormap %s has %d levels.\n", clrf, l/256);
        if ((GL_ClrTableOrg = NEW(l+255)) == NULL)
            BASE_Abort("Can't allocate %d bytes for colormap.\n", l+255);
        GL_ClrTable = (byte*)((dword)(GL_ClrTableOrg + 255) & ~255);
        REQUIRE(JCLIB_Load(clrf, GL_ClrTable, l) > 0);
        POLY_InitShadeTable(GL_ClrTable);
    }

    printf("Checking network interface:\n");
    COM_Init();
    if (COM_IPXPresent) {
        memcpy(IPXLocalAddr, COM_MyAddress.addr, 6);
        printf("  IPX Detected, local address is %02X:%02X:%02X:%02X:%02X:%02X\n",
               IPXLocalAddr[0], IPXLocalAddr[1],
               IPXLocalAddr[2], IPXLocalAddr[3],
               IPXLocalAddr[4], IPXLocalAddr[5]);
    } else {
        printf("No IPX driver, only serial/modem game allowed.\n");
    }
    printf("Initializing record and circuit tables:\n");
    REC_Init();
    MDL_InitCircuits();

    GL_InitCapture();

    return TRUE;
}

#pragma off (check_stack)

    // This gets called during interrupts by the timer handler.
extern void GenericTimerFunction(void) {
    XTRN_StackExec(&SH_Stack, GL_PollMusic);
}

    // This gets called during interrupts by the vbl handler.
extern void GenericVBLFunction(void) {
    VBL_FadeHandler();
    XTRN_StackExec(&SH_Stack, GL_PollMusic);
}

// ==========================================



PRIVATE void ShowEntryScreen(void) {
    char buf[200];
    byte *d;
    byte *p, *q;
    int i, j;
    byte c;

    memset(LLS_Screen[0], 0, LLS_Size);
    LLS_Update();

    d = NEW(64000);
    if (d == NULL)
        return;

    sprintf(buf, "rload%d0.pal", GL_CarType);
    JCLIB_Load(buf, d, 768);
    VGA_DumpPalette(d, 0, 256);
    sprintf(buf, "rload%d0.pix", GL_CarType);
    JCLIB_Load(buf, d, 64000);
    p = LLS_Screen[0] + LLS_SizeX*((LLS_SizeY==480)?40:0);
    q = d;
    if (LLS_SizeX == 320) {
        for (i = 0; i < 64000; i++) {
            c = *q++;
            *p++ = c;
        }
    } else if (LLS_SizeX == 640) {
        for (i = 0; i < 200; i++) {
            for (j = 0; j < 320; j++) {
                c = *q++;
                p[640] = c; *p++ = c;
                p[640] = c; *p++ = c;
            }
            p += LLS_SizeX;
        }
    }
    DISPOSE(d);
    LLS_Update();

    {
        GAME_PEffect steng;
        steng = GL_LoadEffect("startech.raw", 11025, FALSE);
        if (steng != NULL) {
            dword clock = TIMER_Clock;
            GAME_EFF_Start(SH_Vtal, 0, 256, 450, 0, steng);
            while ((TIMER_Clock - clock) < 3*70);
        }
        GAME_EFF_StopAll(SH_Vtal);
        GAME_EFF_UnloadAll(SH_Vtal);
    }
}


PRIVATE bool ShowRaceResults(int racemode, int np, RACE_TResult *result, int *scores, bool last) {
    static int pospts[] = {
        25,
        18,
        15,
        12,
         9,
         7,
         6,
         5,
         4,
         3,
         2,
         1,
    };
    int i, l;
    bool abandon = FALSE;

    for (i = 0; i < np; i++)
        if (result[i].pos < SIZEARRAY(pospts))
            scores[i] += pospts[result[i].pos];

    {   // Start VBL fading.
        VBL_DestRed = VBL_DestGreen = VBL_DestBlue = 0;
        VBL_DestPal = NULL;
        VBL_FadeSpeed = 3;
        VBL_FadeMode = VBL_FADEFAST;
        VBL_FadePos = 1;    // Go!
    }
    LLK_LastScan = 0;
    while (VBL_FadePos > 0 && LLK_LastScan == 0);
    VBL_FadePos = 0;
    VBL_ZeroPalette();
    LLK_LastScan = 0;

    MENU_Init("mbg_resu.pix", NULL);
    memcpy(LLS_Screen[0], MENU_Back, LLS_Size);

    for (i = 0; i < np; i++) {
        int j, k;
        for (j = 0; j < 23; j++) {
            byte *p;
            p = LLS_Screen[0] + (j+70+30*i-2)*LLS_SizeX + 10;
            for (k = 10; k < 310; k++, p++) {
                *p = GL_ClrTable[(16-11+Abs32(j-11)*15/11)*256 + *p];
            }
        }
    }

    memcpy(MENU_Back, LLS_Screen[0], LLS_Size);
    LLS_Update();

    {   // Start VBL fading.
        VBL_DestPal = GamePal;
        VBL_FadeSpeed = 3;
        VBL_FadeMode = VBL_FADEFAST;
        VBL_FadePos = 1;    // Go!
    }

    l = 0;
    VBL_VSync(0);
    LLK_LastScan = 0;
    do {
        memcpy(LLS_Screen[0], MENU_Back, LLS_Size);

        for (i = 0; i < np; i++) {
            if (NET_State == NETS_NONE || ((l & 0x10) || !(result[i].flags & RACERF_ISCONSOLE))) {
                static const char *th[] = {
                    "st",
                    "nd",
                    "rd"
                };
                const char *p;
                if (result[i].pos >= 0 && result[i].pos < SIZEARRAY(th))
                    p = th[result[i].pos];
                else
                    p = "th";
                TEXT_Printf(&MENU_FontWhite,    10, 70+30*i, 0, "Player %d", i+1);
                TEXT_Printf(&MENU_FontRed,  120, 70+30*i, 0, "Finished %d%s",
                            result[i].pos+1, p);
                TEXT_Printf(&MENU_FontRed,  120, 82+30*i, 0, "Total score is %d",
                            scores[i]);
            }
        }
        l += VBL_VSync(1);
        LLS_Update();
        GL_Capture(LLK_LastScan);

        if (NET_State != NETS_NONE) {
            int nlink;
            NET_TPacket p;

            nlink = COM_GetPacket(&p.p);
            if (nlink > 0)
                if (p.command == NETC_BYE && p.len == sizeof(NET_TStartPacket))
                    abandon = TRUE;
        }

        if (LLK_LastScan == kESC)
            abandon = TRUE;
        if (abandon && racemode == 0 && !last) {
            if (LLK_LastScan == kESC) {
                int x, y;

                x = LLS_SizeX/2 - 160;
                y = LLS_SizeY/2 - 100;
                GFX_Rectangle(x+20, y+70, 280,  60, 15, -1);
                GFX_Rectangle(x+21, y+71, 278,  58,  7, -1);
                GFX_Rectangle(x+22, y+72, 276,  56,  8, -1);
                GFX_Rectangle(x+23, y+73, 274,  54,  7,  8);

                TEXT_Write(&GL_YFont, x+35, y+  80, "Abandon Championship", 15);
                TEXT_Write(&GL_RFont, x+25, y+ 105, "Press  ESC  to confirm", 15);
                LLS_Update();
                LLK_LastScan = 0;
                while (LLK_LastScan == 0);
                if (LLK_LastScan != kESC)
                    abandon = FALSE;
            }
            if (NET_State != NETS_NONE && abandon) {
                int numoknodes = 1;
                int i, j, nlink;
                int nodeok[NET_MAXNODES], numok = 1;

                for (i = 0; i < SIZEARRAY(nodeok); i++)
                    nodeok[i] = 0;

                    // Ensure all other nodes know it.
                VBL_VSync(0);
                LLK_LastScan = 0;
                j = 70;
                for (;;) {
                    NET_TPacket p;

                    if (LLK_LastScan == kESC)
                        break;

                    if (j >= 30) {  // Send packets every 30 ticks.
                        p.command = NETC_BYE;
                        p.len  = sizeof(NET_TStartPacket);
                        p.time = 0;
                        p.oknodes = numoknodes;
                        COM_SendPacket(0, &p.p);
                        if (numok == NET_NumNodes)
                            break;
                        j = 0;
                    }

                    j += VBL_VSync(1);
                    COM_Housekeep();
                    nlink = COM_GetPacket(&p.p);
                    if (nlink > 0) {
                        if (p.command == NETC_BYE && p.len == sizeof(NET_TStartPacket)) {
                            if (nodeok[nlink] == 0) {
                                nodeok[nlink]++;
                                numoknodes++;
                            }
                            if (p.oknodes == NET_NumNodes) {
                                if (nodeok[nlink] == 1) {
                                    numok++;
                                    nodeok[nlink]++;
                                }
                            }
                        }
                    } else if (nlink == -2) {
                    }
                }
            }
            LLK_LastScan = 0;
        }
    } while (!abandon && LLK_LastScan == 0);

    VBL_FadePos = 0;    // Stop fading.
    VBL_DumpPalette(GamePal, 0, 256);
    VBL_VSync(0);   // Ensure you have it.
    VBL_VSync(1);
    MENU_End();
    return abandon;
}

PRIVATE void RunRace(int racemode) {
    RACE_TResult result[NET_MAXNODES];
    int          scores[NET_MAXNODES];
    int i, ns, np;
    int lastm = -1;

    memset(scores, 0, sizeof(scores));

    MENU_End();
    do {
            // Choose song
        do {
            ns = RND_GetNum() & 63;
        } while (CFG_Config.nsongs > 1 && ns == lastm);
        lastm = ns;

        MENU_DoneMusic();
        if (CFG_Config.nsongs > 0) {
            char buf[200];
            ns = ns%CFG_Config.nsongs;
            sprintf(buf, "haste%d.s3m", ((&CFG_Config.mus1)[ns])+1);
            MENU_InitMusic(buf);
        } else {
            MENU_InitMusic(NULL);
        }
        VBL_Done();
        TIMER_Init(TIMER_70HZ);
        TIMER_HookFunction = &GenericTimerFunction;
        if (!SVGAOn
         || (!LLS_Init(LLSM_VIRTUAL, LLSVM_640x400x256)
          && !LLS_Init(LLSM_VIRTUAL, LLSVM_640x480x256))) {
            SVGAOn = FALSE;
            LLS_Init(LLSM_VIRTUAL, LLSVM_MODE13);
        }
        ShowEntryScreen();
        memset(result, 0, sizeof(result));
        np = RACE_DoRace(racemode, result);
        if (SVGAOn)
            LLS_Init(LLSM_VIRTUAL, LLSVM_MODE13);
        MENU_DoneMusic();
        TIMER_End();
        VBL_Init(0);
        VBL_FullHandler = &GenericVBLFunction;
        MENU_InitMusic("final.s3m");
        if (racemode < 2) {
            int i, j;
            j = 0;
            for (i = 0; i < SIZEARRAY(result); i++) {
                if (result[i].flags & (RACERF_ISCONSOLE)) {
                    REC_InputBest(REC_Best, REC_NBest, j, GL_SelCircuit+1, result[i].nlaps, result[i].time, GL_CarType);
                    REC_InputCirc(REC_Circ, REC_NCirc, j, GL_SelCircuit+1, result[i].best, GL_CarType);
                    j++;
                }
            }
        }
        if (racemode == 2
         || ShowRaceResults(racemode, np, result, scores, GL_SelCircuit >= (MDL_NCircuits-1)))
            break;
        if (racemode == 0)
            GL_SelCircuit++;
    } while (racemode == 0 && GL_SelCircuit < MDL_NCircuits);
    if (REC_ShowBest(REC_Best, REC_NBest, GL_CarType) >= 0)
        REC_ShowCirc(REC_Circ, REC_NCirc, GL_CarType);
}

PRIVATE void DrawBar(void) {
    TXS_DrawString(0, 80, 0, "CIRCUIT RACER v" VERSION , 0x2E, TXS_STCENTER);
    TXS_SetCursor(0, 1);
}

extern void FinishProgram(void) {
    VBL_Done();
    TIMER_End();
    DoneMusicSystem();
    LLS_End();
    NET_End();
    LLK_End();
    LLK_BIOSFlush();
    VBL_RestoreSystemTime();

    DrawBar();
    {
        int c;
        char *p;
#include "blueprin.c"

        p = BluePrint;
        c = 0x34;
        do {
            if ((byte)((*p - c)^c) == 0)
                break;
            printf("%c", (*p - c) ^ c);
            c += ((*p + 93) ^ 11) + 205;
            p++;
        } while (TRUE);
    }
}

void main(int argc, char *argv[]) {
    dword startDelay, oldClock, nFrames;
    int  i;
    sint efftime, lefftime;
    bool firstmenu = TRUE;

    ArgC = argc; ArgV = argv;

    VGA_SetMode(3);
    DrawBar();

    printf("%s",
        "Circuit Racer, Copyright 1995-1997 by J. Arevalo & Noriaworks Entertainment.\n"
//        "All rights reserved. DISTRIBUTION OR DISCLOSURE IN ANY FORM IS FORBIDDEN.\n"
        "\n"
    );
    if (BASE_CheckArg("help") > 0|| BASE_CheckArg("h") > 0 || BASE_CheckArg("?") > 0) {
        printf(
            "Command-line arguments:\n"
            " -help    to see this help page.\n"
            " -novbl   if you experience slowdown and/or sound problems in the menus.\n"
            "          This may be required under multitaskers.\n"
            " -nologo  to skip initial logos.\n"
//            " -nodemo  to skip the initial demo.\n"
            " -slave   to force 'Join multiplayer game' option in net games.\n"
            " -master  to force 'Start multiplayer game' option in net games.\n"
            " -nosound to avoid loading the sound support.\n"
            " -noquit  to prevent quitting from the game.\n"
            " -joybios if the joystick doesn't work.\n"
            " -joyval number (>= 1)\n"
            "          if the joystick behaves erratically (1 if not given).\n"
            " -file file1.jcl file2.jcl ...\n"
            "          to load external datafiles.\n"
            " -ticks number (>= 1)\n"
            "          to allow better resync in fast networks (1 if not given).\n"
            " -car   number (>= 1, < 6) to force choice of a car.\n"
            " -track number (>= 1, < 8) to force choice of a track.\n"
        );
        exit(0);
    } else
        printf("Run CR -help to find about command-line parameters.\n");
    printf(
        "\n"
        "Initializing ...\n"
    );

    if (!CFG_Load("speed.cfg")) {
        char buf[300];
        char *p;

        if (JCLIB_FileSize("setup.exe") <= 0) {
            p = buf;
            strcpy(p, ArgV[0]);
            if ( (p = strrchr(buf, '\\')) != NULL
              || (p = strrchr(buf, ':')) != NULL)
                p[1] = '\0';
            else
                p = "";
            sprintf(buf, "%ssetup > nul", buf);
        } else
            strcpy(buf, "setup > nul");
        system(buf);
        VGA_SetMode(3);
        DrawBar();
        if (!CFG_Load("speed.cfg")) {
            printf("No configuration file, assuming nosound and keyboard-only controls.\n"
                   " Run SETUP to configure the game.\n"
                   "\n"
                   "Press a key to continue.\n");
            LLK_BIOSWaitKey(); LLK_BIOSFlush();
        }
    }

    printf("Datafile setup:\n");
    {
        if (!JCLIB_Init(CFG_Config.Datafile)) {
            char buf[300];
            char *p;
            strcpy(buf, ArgV[0]);
            p = strrchr(buf, '\\');
            if (p == NULL)
                BASE_Abort("Unable to find main datafile %s", CFG_Config.Datafile);
            strcpy(p+1, CFG_Config.Datafile);
            if (!JCLIB_Init(buf))
                BASE_Abort("Unable to open main datafile %s", buf);
            printf("Using original main datafile %s\n", buf);
        } else
            printf("Using main datafile %s\n", CFG_Config.Datafile);
    }
    if (JCLIB_FileSize("map07.sec") > 0) {
        GL_Registered = TRUE;
        if ( (i = BASE_CheckArg("file")) > 0) {
            while (i < ArgC && ArgV[i][0] != '-' && ArgV[i][0] != '/') {
                if (!JCLIB_Init(ArgV[i]))
                    printf("Unable to open additional datafile %s\n", ArgV[i]);
                else
                    printf("Using additional datafile %s\n", ArgV[i]);
                i++;
            }
        }
    }
    printf("%d metafiles with %d total files.\n", JCLIB_GetNLibs(), JCLIB_GetNFiles());
    printf("-------------------------------------------------------------------------------\n");
    if (JCLIB_GetNLibs() > 1) {
        printf("  This REGISTERED VERSION has been modified. You are using an unsupported\n"
               "     version of Circuit Racer.\n"
               "You are not allowed to copy, lend or distribute this game.\n");
        printf("       Press a key to continue.\n");
        printf("-------------------------------------------------------------------------------\n");
        LLK_BIOSWaitKey(); LLK_BIOSFlush();
    } else if (GL_Registered) {
        printf("  This is the REGISTERED VERSION of Circuit Racer.\n"
               "You are not allowed to copy, lend or distribute this game.\n");
        printf("-------------------------------------------------------------------------------\n");
    } else {
        printf("  This is the SHAREWARE VERSION of Circuit Racer.\n"
               "You can copy this version to your friends.\n");
        printf("-------------------------------------------------------------------------------\n");
    }

    //LogMemory();

    if (!BasicInitData())
        return;

    InitMusicSystem();

/*
    if (BASE_CheckArg("nologo") <= 0) {
//        VGA_SetMode(3);
        puts("\n"
            "Speed Haste PREVIEW, Copyright 1995 by J. Arevalo & Noriaworks Entertainment.\n"
            "All rights reserved. DISTRIBUTION OR DISCLOSURE IN ANY FORM IS FORBIDDEN.\n"
            "  Keys:\n"
            "    Player 1:\n"
            "       ARROWS for movement, SPACE to change gear.\n"
            "    Player 2:\n"
            "       Keypad ARROWS for movement, keypad INS to change gear.\n"
            "    F1-F4 and F7-F10 select different standard cameras.\n"
            "    S to toggle two player mode (experimental).\n"
*/ /*
            "    V to cycle around vector detail level.\n"
            "    T for turning color trace on/off.\n"
            "    D for changing detail level and robot cars on/off.\n"
            "    N/P to cycle thru the robot cars in the bottom window.\n"
            "    INS/DEL   change camera focus.\n"
            "    HOME/END  change horizon line.\n"
            "    PGUP/PGDN change height.\n"
            "    Q/A       change camera distance.\n"
            "    Gray +/-  change map zoom.\n"
            "    \",\" and \".\" to rotate around camera target.\n"
*/ /*
            "\n"
        );
        printf("    Press a key.\n");
        LogMemory();
        LLK_PressAnyKey();
    }
*/
    if (BASE_CheckArg("nologo") <= 0) {
        INTRO_DoIntro();
    }

    LLS_Init(LLSM_VIRTUAL, LLSVM_MODE13);
    DRW_SetClipZone(0, 0, LLS_SizeX, LLS_SizeY, NULL);
    POLY_MaxX = LLS_SizeX; POLY_MaxY = LLS_SizeY;
/*
    if ( (i = BASE_CheckArg("solid")) > 0)
        O3DM_MaxDetail = atoi(ArgV[i+1])%(O3DD_TEXGOURAUD+1);
    else
*/
        O3DM_MaxDetail = O3DD_TEXGOURAUD;
    VGA_GetPalette(GamePal, 0, 16);
    VGA_GetPalette(GamePal + (256-16)*3, 256-16, 16);
    VGA_ZeroPalette();
//    VGA_DumpPalette(GamePal, 0, 256);

//    if (BASE_CheckArg("nodemo") > 0)
        firstmenu = FALSE;
/*
    else
        MENU_InitMusic("haste8.s3m");
*/

    GAME_SetMode(SH_Vtal, GAME_POLL);
        // Once in poll mode, we can fuck the timer any way we want.

    VBL_Init(0);
    VBL_DumpPalette(GamePal, 0, 256);
    VBL_FullHandler = &GenericVBLFunction;

        // Enter demo-menu-game loop.
    for (;;) {
        int rez;
        LogMemory("demo-menu game loop");

        if (firstmenu) {
            int bgd = BackgroundDetail;
            BackgroundDetail = 50;

            MENU_End();
            VBL_Done();
            TIMER_Init(TIMER_70HZ);
//            TIMER_Init(TIMER_35HZ);
            TIMER_HookFunction = &GenericTimerFunction;
            if (!SVGAOn
             || (!LLS_Init(LLSM_VIRTUAL, LLSVM_640x400x256)
              && !LLS_Init(LLSM_VIRTUAL, LLSVM_640x480x256))) {
                SVGAOn = FALSE;
                LLS_Init(LLSM_VIRTUAL, LLSVM_MODE13);
            }
            GL_SelCircuit = 0;
            ShowEntryScreen();
            RACE_DoDemo();
            if (SVGAOn)
                LLS_Init(LLSM_VIRTUAL, LLSVM_MODE13);
            TIMER_End();
            VBL_Init(0);
            VBL_FullHandler = &GenericVBLFunction;
            MENU_DoneMusic();
            firstmenu = FALSE;
            MENU_InitMusic("final.s3m");
            if (MENU_DoCredits() >= 0)
                if (REC_ShowBest(REC_Best, REC_NBest, GL_CarType) >= 0)
                    REC_ShowCirc(REC_Circ, REC_NCirc, GL_CarType);
            MENU_DoneMusic();
            MENU_InitMusic("menus.s3m");
            BackgroundDetail = bgd;
        } else {
            if (strncmp(MENU_SongName, "menus.s3m", sizeof(MENU_SongName)) != 0) {
                MENU_DoneMusic();
                MENU_InitMusic("menus.s3m");
            }
        }
        GL_Seed = RND_GetNum();

        COM_EndGame();
        NET_State = NETS_NONE;
        NET_NumNodes = 0;
        memset(NET_NodeData, 0, sizeof(NET_NodeData));
        memset(NET_NodeNum,  0, sizeof(NET_NodeNum));

        MENU_Enter(MENU_MAIN);
        rez = MENU_Run();
        if (rez >= 0) {
            int racemode = rez;

            if (racemode == 0)
                GL_SelCircuit = 0;
            if (racemode != 0 && (NET_State == NETS_NONE || !NET_Slave)) {
                if (BASE_CheckArg("car") <= 0 || BASE_CheckArg("track") <= 0) {
                    MENU_DoneMusic();
                    MENU_InitMusic("scircuit.s3m");
                }
                MENU_Enter(MENU_CHOOSECIRC);
                rez = MENU_Run();
                if (rez >= 0)
                    GL_SelCircuit = rez;
            }
            if (rez >= 0 && NET_State != NETS_NONE) {
                GL_ConsolePlayers = 1;
                rez = NET_ContactPlayers(&racemode);
            }
            if (rez >= 0) {
                MENU_SelCarNumPlayer = 0;
                MENU_Enter(MENU_CHOOSECAR);
                rez = MENU_Run();
                if (rez >= 0) {
                    GL_SelCar[0] = rez;
                    if (GL_ConsolePlayers == 2) {
                        MENU_SelCarNumPlayer = 1;
                        MENU_Enter(MENU_CHOOSECAR);
                        rez = MENU_Run();
                        if (rez >= 0)
                            GL_SelCar[1] = rez;
                    } else
                        rez = 0;
                    if (rez >= 0 && NET_State != NETS_NONE) {
                        rez = NET_GetCars();
                    }
                    if (rez >= 0)
                        RunRace(racemode);
                }
            }
            MENU_End();
            rez = 0;
        } else if (rez == -2) {
            MENU_DoneMusic();
            MENU_InitMusic("haste9.s3m");
            firstmenu = TRUE;
            rez = 0;
        } else if (rez == -3) {
            rez = 0;        // Reinit music, etc.
        } else if (rez == -4) {     // Greetings.
            MENU_End();
            INTRO_DoGreets();
            rez = 0;
        }
        if (rez < 0)
            break;
        COM_EndGame();
    }

    memset(LLS_Screen[0], 0, LLS_Size);
    LLS_Update();
    VGA_ZeroPalette();
    VBL_ZeroPalette();
    LLK_LastScan = 0;
    if (JCLIB_Load("sphlogo.pix", LLS_Screen[0], 320*200) == 320*200
     && JCLIB_Load("sphlogo.pal", GamePal, 768) == 768) {
        int nloops;

        MENU_DoneMusic();
        MENU_InitMusic("fin.s3m");

        VBL_VSync(0);
        VBL_VSync(2);
        LLS_Update();
        {   // Start VBL fading.
            VBL_DestPal = GamePal;
            VBL_FadeSpeed = 1;
            VBL_FadeMode = VBL_FADEFAST;
            VBL_FadePos = 1;    // Go!
        }
        while (VBL_FadePos != 0 && LLK_LastScan == 0);
        VBL_DumpPalette(GamePal, 0, 256);
        VBL_VSync(0);
        VBL_VSync(2);
        nloops = 0;
        LLK_LastScan = 0;
        while (nloops < 22*70 && LLK_LastScan == 0) {
            VBL_VSync(1);
            nloops++;
        }
        GAME_MUS_SetFading(SH_Vtal, GAME_FADEOUT, 150);
        LLK_LastScan = 0;
        {   // Start VBL fading.
            VBL_DestPal = NULL;
            VBL_DestRed = VBL_DestGreen = VBL_DestBlue = 0;
            VBL_FadeSpeed = 1;
            VBL_FadeMode = VBL_FADEFAST;
            VBL_FadePos = 1;    // Go!
        }
        while (VBL_FadePos != 0 && LLK_LastScan == 0);
    } else
        MENU_DoneMusic();

        // Wait for previous song to end its fade.
    if (SH_Vtal != NULL && SH_Vtal->MusicDev != NULL && SH_Vtal->SongPlaying)
        while (SH_Vtal->MusParams.Volume > 0 && !LLK_Keys[kESC])
            GAME_Poll(SH_Vtal, 20);   // Time up to 20 ticks (don't allow music to stop).
    VBL_Done();
    LLS_End();
    //LogMemory();

    //FinishProgram();

    //LogMemory();

    VBL_Done();
    TIMER_End();
    DoneMusicSystem();
    LLS_End();
    NET_End();
    LLK_End();
    LLK_BIOSFlush();
    VBL_RestoreSystemTime();

    {
        byte endtext[4000];

        if (JCLIB_Load("endtext.bin", endtext, 4000) == 4000) {
            memcpy(TXS_ScrMem, endtext, 4000);
            TXS_SetCursor(0, 22);
        } else {
            DrawBar();
        }
    }
}

