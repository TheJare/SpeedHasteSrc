// ----------------------------- CONFIG.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Game configuration file.

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strparse.h>
#include <llkey.h>

#include <dos.h>
#include <direct.h>

#define CFGDIR "C:\\CR"

CFG_TConfig CFG_Config = {
                            // ------ Control options.
    CFG_DETECT,             //   UseJoystick,
    CFG_DETECT,             //   UseMouse;

                            // ------ Game options.
    0,                      //   Detail;
    "cr.jcl",           //   Datafile;

                            // ------ Sound options.
    "",                     //  *MusicDevice,
    "",                     //  *FXDevice;
    0x220,                  //   MusicPort,
    7,                      //   MusicIRQ,
    1,                      //   MusicDMA,
    16000,                  //   MusicRate,
    16,                     //   MusicChannels,
    256,                    //   MusicVolume;
    CFG_YES,                //   MusicOn
    0x220,                  //   FXPort,
    7,                      //   FXIRQ,
    1,                      //   FXDMA,
    16000,                  //   FXRate,
    5,                      //   FXChannels,
    256,                    //   FXVolume;
    CFG_YES,                //   FXOn

    {0,1,2,3},              //   JoyAX,
    {0,1,2,3},              //   JoyAY,
    {0,1,2,3},              //   JoyBX,
    {0,1,2,3},              //   JoyBY;

    CFG_KEYBOARD,           //   P1Control
    CFG_KEYBOARD,           //   P3Control

    kUARROW,                //   P1KeyUp
    kDARROW,                //   P1KeyDn
    kLARROW,                //   P1KeyLt
    kRARROW,                //   P1KeyRt
    kSPACE,                 //   P1KeyGe
    kKEYPAD8,               //   P2KeyUp
    kKEYPAD2,               //   P2KeyDn
    kKEYPAD4,               //   P2KeyLt
    kKEYPAD6,               //   P2KeyRt
    kKEYPAD0,               //   P2KeyGe

    5,                      //   NumLaps

    8,                      //   nsongs;
    0, 1, 2, 3, 4, 5, 6, 7, //   mus[1-8]

    0,                      //   CarType

    16,                     //   DecorationDetail,
    1 ,                     //   FloorDetail,
    1 ,                     //   WallsDetail,
    1 ,                     //   PolygonDetail,;
    0 ,                     //   BackgroundDetail;

    "ATZ",                  //   modeminit[80],
    "ATDT",                 //   modemdial[80],
    "ATH0",                 //   modemhang[80];

    0,                      //   ipxsocket,
    2,                      //   comnum,
    0x2F8,                  //   comaddr,
    3,                      //   comirq;

    "",                     //   phone1[CFG_STRSIZE];
    "",                     //   phone2[CFG_STRSIZE];
    "",                     //   phone3[CFG_STRSIZE];
    "",                     //   phone4[CFG_STRSIZE];
    "",                     //   phone5[CFG_STRSIZE];
    "",                     //   phone6[CFG_STRSIZE];

    FALSE,                  //   SVGAOn

    TRUE,                   //   TimedRace

};

// --------------------------------

PRIVATE const char* DetectStr[] = {
    "No",
    "Yes",
    "Detect"
};

PRIVATE const char* ControlStr[] = {
    "Keyboard",
    "Joystick A",
    "Joystick B"
    "Mouse",
};

PUBLIC const char *CFG_Soundcards[CFG_NCARDS][2] = {
    {"",           "None"},
    {"gus.dev",    "Gravis Ultrasound"},
    {"gusmax.dev", "Gravis Ultrasound MAX"},
    {"sb_1_5.dev", "Soundblaster 1.5"},
    {"sb_2_0.dev", "Soundblaster mono"},
    {"sb_pro.dev", "Soundblaster Pro"},
    {"sb_16.dev",  "Soundblaster 16"},
    {"sb_awe.dev", "Soundblaster AWE32"},
};

PRIVATE void WriteYesNoDet(FILE *f, void *arg) {
    fprintf(f, "%s", DetectStr[*((byte*)arg)]);
}

PRIVATE void ReadYesNoDet(void *dest, const char *s) {
    byte k = CFG_DETECT;
    int  i;

    for (i = 0; i < SIZEARRAY(DetectStr); i++)
        if (stricmp(DetectStr[i], s) == 0) {
            k = i;
            break;
        }
    *((byte*)dest) = k;
}

PRIVATE void WriteControl(FILE *f, void *arg) {
    fprintf(f, "\"%s\"", ControlStr[*((byte*)arg)]);
}

PRIVATE void ReadControl(void *dest, const char *s) {
    byte k = CFG_KEYBOARD;
    int  i;

    for (i = 0; i < SIZEARRAY(ControlStr); i++)
        if (stricmp(ControlStr[i], s) == 0) {
            k = i;
            break;
        }
    *((byte*)dest) = k;
}

PRIVATE void WriteDetail(FILE *f, void *arg) {
    fprintf(f, "%d", *((byte*)arg));
}

PRIVATE void ReadDetail(void *dest, const char *s) {
    *((byte*)dest) = atoi(s);
}

PRIVATE void WriteInt(FILE *f, void *arg) {
    fprintf(f, "%d", *((int*)arg));
}

PRIVATE void ReadInt(void *dest, const char *s) {
    *((int*)dest) = atoi(s);
}

PRIVATE void WriteWord(FILE *f, void *arg) {
    fprintf(f, "%5d", *((word*)arg));
}

PRIVATE void WriteHWord(FILE *f, void *arg) {
    fprintf(f, "$%04X", *((word*)arg));
}

PRIVATE void ReadWord(void *dest, const char *s) {
    int k = 0;
    if (s[0] == '$')
        sscanf(s+1, "%X", &k);
    else
        sscanf(s, "%i", &k);
    *((word*)dest) = k;
}

PRIVATE void WriteHByte(FILE *f, void *arg) {
    fprintf(f, "$%02X", *((byte*)arg));
}

PRIVATE void ReadByte(void *dest, const char *s) {
    int k = 0;
    if (s[0] == '$')
        sscanf(s+1, "%X", &k);
    else
        sscanf(s, "%i", &k);
    *((byte*)dest) = k;
}

PRIVATE void WriteStr(FILE *f, void *arg) {
    fprintf(f, "\"%s\"", (char*)arg);
}

PRIVATE void ReadStr(void *dest, const char *s) {
    strncpy(dest, s, CFG_STRSIZE);
    ((char*)dest)[CFG_STRSIZE-1] = '\0';
}

PUBLIC const char *CFG_FindCard(const char *card) {
    int i;
    for (i = 0; i < SIZEARRAY(CFG_Soundcards); i++) {
        if (stricmp(CFG_Soundcards[i][0], card) == 0)
            return CFG_Soundcards[i][1];
        if (stricmp(CFG_Soundcards[i][1], card) == 0)
            return CFG_Soundcards[i][0];
    };
    return card;
}

PRIVATE void WriteCard(FILE *f, void *arg) {
    fprintf(f, "\"%s\"", CFG_FindCard((char*)arg));
//    printf("Writecard: %s -> %s\n", (const char *)arg, CFG_FindCard((char*)arg));
}

PRIVATE void ReadCard(void *dest, const char *s) {
    memset(dest, 0, CFG_STRSIZE);
    strncpy(dest, CFG_FindCard(s), CFG_STRSIZE);
    ((char*)dest)[CFG_STRSIZE-1] = '\0';
//    printf("Readcard: %s -> %s\n", s, (const char *)dest);
}

struct {
    const char *name;
    void *dest;
    void (*funcw)(FILE *f, void *arg);          // Write function.
    void (*funcr)(void *dest, const char *arg); // Read function.
} DataConfigs[] = {
    {"UseJoystick",   &CFG_Config.UseJoystick,  WriteYesNoDet,ReadYesNoDet},
    {"UseMouse",      &CFG_Config.UseMouse,     WriteYesNoDet,ReadYesNoDet},

    {"Detail",        &CFG_Config.Detail,       WriteDetail,  ReadDetail},
    {"Datafile",      &CFG_Config.Datafile,     WriteStr,     ReadStr},

    {"MusicDevice",   &CFG_Config.MusicDevice,  WriteCard,    ReadCard},
    {"FXDevice",      &CFG_Config.FXDevice,     WriteCard,    ReadCard},

    {"MusicPort",     &CFG_Config.MusicPort,    WriteHWord,   ReadWord},
    {"MusicIRQ",      &CFG_Config.MusicIRQ,     WriteWord,    ReadWord},
    {"MusicDMA",      &CFG_Config.MusicDMA,     WriteWord,    ReadWord},
    {"MusicRate",     &CFG_Config.MusicRate,    WriteWord,    ReadWord},
    {"MusicChannels", &CFG_Config.MusicChannels,WriteWord,    ReadWord},
    {"MusicVolume",   &CFG_Config.MusicVolume,  WriteWord,    ReadWord},
    {"MusicOn",       &CFG_Config.MusicOn,      WriteYesNoDet,ReadYesNoDet},
    {"FXPort",        &CFG_Config.FXPort,       WriteHWord,   ReadWord},
    {"FXIRQ",         &CFG_Config.FXIRQ,        WriteWord,    ReadWord},
    {"FXDMA",         &CFG_Config.FXDMA,        WriteWord,    ReadWord},
    {"FXRate",        &CFG_Config.FXRate,       WriteWord,    ReadWord},
    {"FXChannels",    &CFG_Config.FXChannels,   WriteWord,    ReadWord},
    {"FXVolume",      &CFG_Config.FXVolume,     WriteWord,    ReadWord},
    {"FXOn",          &CFG_Config.FXOn,         WriteYesNoDet, ReadYesNoDet},

    {"JoyAX0",        &CFG_Config.JoyAX[0],     WriteWord,    ReadWord},
    {"JoyAX1",        &CFG_Config.JoyAX[1],     WriteWord,    ReadWord},
    {"JoyAX2",        &CFG_Config.JoyAX[2],     WriteWord,    ReadWord},
    {"JoyAX3",        &CFG_Config.JoyAX[3],     WriteWord,    ReadWord},
    {"JoyAY0",        &CFG_Config.JoyAY[0],     WriteWord,    ReadWord},
    {"JoyAY1",        &CFG_Config.JoyAY[1],     WriteWord,    ReadWord},
    {"JoyAY2",        &CFG_Config.JoyAY[2],     WriteWord,    ReadWord},
    {"JoyAY3",        &CFG_Config.JoyAY[3],     WriteWord,    ReadWord},
    {"JoyBX0",        &CFG_Config.JoyBX[0],     WriteWord,    ReadWord},
    {"JoyBX1",        &CFG_Config.JoyBX[1],     WriteWord,    ReadWord},
    {"JoyBX2",        &CFG_Config.JoyBX[2],     WriteWord,    ReadWord},
    {"JoyBX3",        &CFG_Config.JoyBX[3],     WriteWord,    ReadWord},
    {"JoyBY0",        &CFG_Config.JoyBY[0],     WriteWord,    ReadWord},
    {"JoyBY1",        &CFG_Config.JoyBY[1],     WriteWord,    ReadWord},
    {"JoyBY2",        &CFG_Config.JoyBY[2],     WriteWord,    ReadWord},
    {"JoyBY3",        &CFG_Config.JoyBY[3],     WriteWord,    ReadWord},

    {"P1Control",     &CFG_Config.P1Control,    WriteControl, ReadControl},
    {"P2Control",     &CFG_Config.P2Control,    WriteControl, ReadControl},

    {"P1KeyUp",       &CFG_Config.P1KeyUp,      WriteHByte,   ReadByte},
    {"P1KeyDn",       &CFG_Config.P1KeyDn,      WriteHByte,   ReadByte},
    {"P1KeyLt",       &CFG_Config.P1KeyLt,      WriteHByte,   ReadByte},
    {"P1KeyRt",       &CFG_Config.P1KeyRt,      WriteHByte,   ReadByte},
    {"P1KeyGe",       &CFG_Config.P1KeyGe,      WriteHByte,   ReadByte},
    {"P2KeyUp",       &CFG_Config.P2KeyUp,      WriteHByte,   ReadByte},
    {"P2KeyDn",       &CFG_Config.P2KeyDn,      WriteHByte,   ReadByte},
    {"P2KeyLt",       &CFG_Config.P2KeyLt,      WriteHByte,   ReadByte},
    {"P2KeyRt",       &CFG_Config.P2KeyRt,      WriteHByte,   ReadByte},
    {"P2KeyGe",       &CFG_Config.P2KeyGe,      WriteHByte,   ReadByte},

    {"NumLaps",       &CFG_Config.NumLaps,      WriteWord,    ReadWord},

    {"NSongs",        &CFG_Config.nsongs,       WriteInt,     ReadInt},
    {"Mus1",          &CFG_Config.mus1,         WriteInt,     ReadInt},
    {"Mus2",          &CFG_Config.mus2,         WriteInt,     ReadInt},
    {"Mus3",          &CFG_Config.mus3,         WriteInt,     ReadInt},
    {"Mus4",          &CFG_Config.mus4,         WriteInt,     ReadInt},
    {"Mus5",          &CFG_Config.mus5,         WriteInt,     ReadInt},
    {"Mus6",          &CFG_Config.mus6,         WriteInt,     ReadInt},
    {"Mus7",          &CFG_Config.mus7,         WriteInt,     ReadInt},
    {"Mus8",          &CFG_Config.mus8,         WriteInt,     ReadInt},

    {"CarType",       &CFG_Config.CarType,      WriteWord,    ReadWord},

    {"DecorationDetail", &CFG_Config.DecorationDetail,  WriteInt, ReadInt},
    {"FloorDetail",      &CFG_Config.FloorDetail,       WriteInt, ReadInt},
    {"WallsDetail",      &CFG_Config.WallsDetail,       WriteInt, ReadInt},
    {"PolygonDetail",    &CFG_Config.PolygonDetail,     WriteInt, ReadInt},
    {"BackgroundDetail", &CFG_Config.BackgroundDetail,  WriteInt, ReadInt},

    {"ModemInit",        &CFG_Config.modeminit,         WriteStr, ReadStr},
    {"ModemDial",        &CFG_Config.modemdial,         WriteStr, ReadStr},
    {"ModemHang",        &CFG_Config.modemhang,         WriteStr, ReadStr},

    {"IPXSocket",        &CFG_Config.ipxsocket,         WriteWord,    ReadWord},
    {"COMNum",           &CFG_Config.comnum,            WriteWord,    ReadWord},
    {"COMAddr",          &CFG_Config.comaddr,           WriteHWord,   ReadWord},
    {"COMIrq",           &CFG_Config.comirq,            WriteWord,    ReadWord},

    {"Phone1",           &CFG_Config.phone1,            WriteStr, ReadStr},
    {"Phone2",           &CFG_Config.phone2,            WriteStr, ReadStr},
    {"Phone3",           &CFG_Config.phone3,            WriteStr, ReadStr},
    {"Phone4",           &CFG_Config.phone4,            WriteStr, ReadStr},
    {"Phone5",           &CFG_Config.phone5,            WriteStr, ReadStr},
    {"Phone6",           &CFG_Config.phone6,            WriteStr, ReadStr},

    {"SVGAOn",           &CFG_Config.SVGAOn,            WriteInt, ReadInt},

    {"TimedRace",        &CFG_Config.TimedRace,         WriteYesNoDet,ReadYesNoDet},

};

bool CFG_Load(const char *fname) {
    FILE *f;
    char buf[200];
    int i;

    f = fopen(fname, "rt");
    if (f == NULL) {
        char buf[300];
        const char *p;
        p = strrchr(fname, '\\');
        if (p == NULL) {
            if ( (p = strrchr(fname, ':')) == NULL)
                p = fname;
            else
                p = p+1;
        } else
            p = p +1;
        sprintf(buf, CFGDIR "\\%s", p);
        if ( (f = fopen(buf, "rt")) == NULL)
            return FALSE;
    }

    while (!feof(f)) {
        char *pp[2];
        fgets(buf, 200, f);                     // Read line.
        STRP_CleanLine(buf, buf);                  // Clean it up.
        if (buf[0] == '\0' || buf[0] == ';')    // Skip blanks and comments.
            continue;
        if (STRP_SplitLine(pp, 2, buf) != 2)         // Split in two words.
            continue;                           // Skip if one word only.

        for (i = 0; i < SIZEARRAY(DataConfigs); i++)
            if (stricmp(pp[0], DataConfigs[i].name) == 0) {
                DataConfigs[i].funcr(DataConfigs[i].dest, pp[1]);
                break;
            }
    }
    fclose(f);
    return TRUE;
}

bool CFG_Save(const char *fname) {
    FILE *f;
    int   i;
    char buf[300];
    const char *p;

    f = fopen(fname, "wt");
    if (f == NULL) {
        p = strrchr(fname, '\\');
        if (p == NULL) {
            if ( (p = strrchr(fname, ':')) == NULL)
                p = fname;
            else
                p = p+1;
        } else
            p = p +1;
        mkdir(CFGDIR);
        sprintf(buf, CFGDIR "\\%s", p);
        p = buf;
        if ( (f = fopen(buf, "wt")) == NULL)
            return FALSE;
    } else
        p = fname;

    fprintf(f, "; Speed Haste configuration file\n");

    for (i = 0; i < SIZEARRAY(DataConfigs); i++) {
        fprintf(f, "%-30s", DataConfigs[i].name);
        DataConfigs[i].funcw(f, DataConfigs[i].dest);
        fputc('\n', f);
    }

    fclose(f);

//    _dos_setfileattr(p, _A_NORMAL | _A_HIDDEN);
    return TRUE;
}

// ----------------------------- CONFIG.C -------------------------------

