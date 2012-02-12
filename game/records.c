// ------------------------------ RECORDS.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "records.h"

#include "globals.h"
#include "menus.h"

#include <vertdraw.h>
#include <llkey.h>
#include <keynames.h>
#include <strparse.h>
#include <llscreen.h>
#include <text.h>
#include <jclib.h>
#include <vbl.h>
#include <string.h>
#include <stdio.h>
#include <direct.h>

// ---------------------------

#define CFGDIR "C:\\CR"


    // --------- Tablas de records.

PUBLIC REC_PBestRecord REC_Best = NULL;
PUBLIC int             REC_NBest = 0;

PUBLIC REC_PCircRecord REC_Circ = NULL;
PUBLIC int             REC_NCirc = 0;

PRIVATE const char *CarName[2] = {
    "Formula  1 ",
    "Stock Car",
};

    // --------- Funciones

PUBLIC bool REC_Init(void) {
    FILE *f;
    char line[200], *tok[10];
    int i, n, ntok;

    if (REC_Best != NULL)
        return TRUE;
    if ( (f = fopen("records.lst", "rt")) == NULL)
        if ( (f = fopen(CFGDIR "\\records.lst", "rt")) == NULL)
            f = JCLIB_OpenText("records.lst");
    if ( (f = JCLIB_OpenText("records.lst")) == NULL)
        BASE_Abort("Can't read RECORDS.LST file!");
    do {
        fgets(line, 200, f);
        STRP_CleanLine(line, line);
    } while (!feof(f) && (line[0] == '\0' || line[0] == ';'));
    REC_NBest = STRP_ReadWord(line);
    REQUIRE(REC_NBest > 0);
    REC_Best = NEW(sizeof(*REC_Best)*REC_NBest);
    REQUIRE(REC_Best != NULL);
    memset(REC_Best, 0, sizeof(*REC_Best)*REC_NBest);
    n = 0;
    i = -1;
    while (!feof(f)) {
        fgets(line, 200, f);
        STRP_CleanLine(line, line);
        if (line[0] == '\0' || line[0] == ';')
            continue;
        ntok = STRP_SplitLine(tok, SIZEARRAY(tok), line);
        if (ntok <= 0)
            continue;
        if (ntok == 1 && stricmp(tok[0], "End") == 0)
            break;
        else if (ntok == 7 && n < REC_NBest) {
            REC_Best[n].ncirc[0] = n+1; //STRP_ReadWord(tok[0]);
            REC_Best[n].ncirc[1] = n+1; //STRP_ReadWord(tok[0]);
            strncpy(REC_Best[n].name[0], tok[1], sizeof(REC_Best[n].name[0]));
            REC_Best[n].name[0][sizeof(REC_Best[n].name)-1] = '\0';
            REC_Best[n].nlaps[0] = STRP_ReadWord(tok[2]);
            REC_Best[n].time[0]  = STRP_ReadWord(tok[3]);
            strncpy(REC_Best[n].name[1], tok[4], sizeof(REC_Best[n].name[1]));
            REC_Best[n].name[1][sizeof(REC_Best[n].name[1])-1] = '\0';
            REC_Best[n].nlaps[1] = STRP_ReadWord(tok[5]);
            REC_Best[n].time[1]  = STRP_ReadWord(tok[6]);
            n++;
        }
    }
    if (n != REC_NBest)
        BASE_Abort("Error in best records, should be %d but is %d", REC_NBest, n);

    do {
        fgets(line, 200, f);
        STRP_CleanLine(line, line);
    } while (!feof(f) && (line[0] == '\0' || line[0] == ';'));
    REC_NCirc = STRP_ReadWord(line);
    REQUIRE(REC_NCirc > 0);
    REC_Circ = NEW(sizeof(*REC_Circ)*REC_NCirc);
    REQUIRE(REC_Circ != NULL);
    memset(REC_Circ, 0, sizeof(*REC_Circ)*REC_NCirc);
    n = 0;
    while (!feof(f)) {
        fgets(line, 200, f);
        STRP_CleanLine(line, line);
        if (line[0] == '\0' || line[0] == ';')
            continue;
        ntok = STRP_SplitLine(tok, SIZEARRAY(tok), line);
        if (ntok <= 0)
            continue;
        if (ntok == 1 && stricmp(tok[0], "End") == 0)
            break;
        else if (ntok == 5 && n < REC_NCirc) {
            REC_Circ[n].ncirc[0] = n+1; //STRP_ReadWord(tok[0]);
            REC_Circ[n].ncirc[1] = n+1; //STRP_ReadWord(tok[0]);
            strncpy(REC_Circ[n].name[0], tok[1], sizeof(REC_Circ[n].name[0]));
            REC_Circ[n].name[0][sizeof(REC_Circ[n].name[0])-1] = '\0';
            REC_Circ[n].time[0]  = STRP_ReadWord(tok[2]);
            strncpy(REC_Circ[n].name[1], tok[3], sizeof(REC_Circ[n].name[1]));
            REC_Circ[n].name[1][sizeof(REC_Circ[n].name[1])-1] = '\0';
            REC_Circ[n].time[1]  = STRP_ReadWord(tok[4]);
            n++;
        }
    }
    if (n != REC_NCirc)
        BASE_Abort("Error in circuit records, should be %d but is %d", REC_NCirc, n);

    JCLIB_Close(f);
    return TRUE;
}

PUBLIC void REC_Save(void) {
    FILE *f;
    int i;

    if (REC_Best == NULL)
        return;
    f = fopen("records.lst", "wt");
    if (f == NULL) {
        mkdir(CFGDIR);
        f = fopen(CFGDIR "\\records.lst", "wt");
        if (f == NULL)
            BASE_Abort("Can't write RECORDS.LST file!");
    }

    fprintf(f, "%d\n", REC_NBest);
    for (i = 0; i < REC_NBest; i++)
        fprintf(f, "%d \"%.8s\" %d %d \"%.8s\" %d %d\n",
                i, REC_Best[i].name[0], REC_Best[i].nlaps[0], REC_Best[i].time[0],
                REC_Best[i].name[1], REC_Best[i].nlaps[1], REC_Best[i].time[1]);
    fprintf(f, "End\n");

    fprintf(f, "%d\n", REC_NCirc);
    for (i = 0; i < REC_NCirc; i++)
        fprintf(f, "%d \"%.8s\" %d \"%.8s\" %d\n",
                i, REC_Circ[i].name[0], REC_Circ[i].time[0],
                REC_Circ[i].name[1], REC_Circ[i].time[1]);
    fprintf(f, "End\n");

    fclose(f);
}

PUBLIC void REC_End(void) {
    DISPOSE(REC_Best);
    DISPOSE(REC_Circ);
    REC_NBest = 0;
    REC_NCirc = 0;
}



PUBLIC int REC_AddBest(REC_PBestRecord list, int n, REC_PBestRecord rec, int ncar) {
    int i;
    if (ncar < 0 || ncar > 1 || list == NULL || rec == NULL
     || rec->time[ncar] == 0 || rec->nlaps[ncar] <= 0)
        return -1;
    for (i = 0; i < n; i++)
        if (rec->ncirc[ncar] == list[i].ncirc[ncar] && list[i].nlaps[ncar] > 0
         && rec->time[ncar]/(rec->nlaps[ncar]) < list[i].time[ncar]/(list[i].nlaps[ncar]))
            break;
    if (i == n)
        return -1;
/*
    for (j = n-1; j > i; j--)
        memcpy(list + j, list + j - 1, sizeof(*list));
*/
    strncpy(list[i].name[ncar], rec->name[ncar], sizeof(rec->name[ncar]));
    list[i].name[ncar][sizeof(rec->name[ncar])-1] = '\0';
    list[i].time[ncar]  = rec->time[ncar];
    list[i].nlaps[ncar] = rec->nlaps[ncar];
    list[i].ncirc[ncar] = rec->ncirc[ncar];
    return i;
}

PUBLIC int REC_AddCirc(REC_PCircRecord list, int n, REC_PCircRecord rec, int ncar) {
    int i;
    if (ncar < 0 || ncar > 1 || list == NULL || rec == NULL || rec->time[ncar] == 0)
        return -1;
    for (i = 0; i < n; i++)
        if (rec->ncirc[ncar] == list[i].ncirc[ncar] && rec->time[ncar] < list[i].time[ncar]) {
            strncpy(list[i].name[ncar], rec->name[ncar], sizeof(rec->name[ncar]));
            list[i].name[ncar][sizeof(list[i].name[ncar])-1] = '\0';
            list[i].time[ncar] = rec->time[ncar];
            return i;
        }
    return -1;
}

PUBLIC int REC_ShowBest(REC_PBestRecord list, int n, int ncar) {
    int i, l;

    if (ncar < 0 || ncar > 1)
        return -1;
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
    LLK_LastScan = 0;

    MENU_Init("mbg_reco.pix", NULL);
    memcpy(LLS_Screen[0], MENU_Back, LLS_Size);
    {
        IS2_PSprite sp;
        sp = IS2_Load("rec_txt.is2");
        REQUIRE(sp != NULL);
        DRW_TranslatePtr = GL_ClrTable + 15*256;
        IS2_Draw(sp, 160-sp->w/2, 5, sp->w, sp->h);
        DISPOSE(sp);
    }
    {
        char buf[200];
        sprintf(buf, "%s Best Racers", CarName[ncar]);
        TEXT_GetExtent(&MENU_FontYellow, 0, 0, buf, &i, 0);
        TEXT_Write(&MENU_FontYellow, (320-i)/2, 60, buf, 0);
    }
    TEXT_Write(&MENU_FontWhite,  10, 73, "Name" , 0);
    TEXT_Write(&MENU_FontWhite, 130, 73, "Track", 0);
    TEXT_Write(&MENU_FontWhite, 190, 73, "Laps" , 0);
    TEXT_Write(&MENU_FontWhite, 240, 73, "Time" , 0);

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
        for (i = 0; i < n; i++) {
            if (list[i].nlaps <= 0)
                break;
            TEXT_Printf(&MENU_FontYellow,  10, 90+13*i, 0, "%.8s", list[i].name[ncar]);
            TEXT_Printf(&MENU_FontYellow, 130, 90+13*i, 0, "%d",    list[i].ncirc[ncar]);
            TEXT_Printf(&MENU_FontYellow, 190, 90+13*i, 0, "%d",    list[i].nlaps[ncar]);
            TEXT_Printf(&MENU_FontYellow, 240, 90+13*i, 0, "%d'%02d\"%02d",
                list[i].time[ncar]/70/60, (list[i].time[ncar]/70)%60,
                list[i].time[ncar]%70*100/70);
        }
        LLS_Update();
        l += VBL_VSync(1);
    } while (l < 10*70 && LLK_LastScan == 0);

    VBL_FadePos = 0;    // Stop fading.
    VBL_DumpPalette(GamePal, 0, 256);
    VBL_VSync(0);   // Ensure you have it.
    VBL_VSync(1);
    MENU_End();
    if (LLK_LastScan == kESC)
        return -1;
    return 0;
}

PUBLIC int REC_ShowCirc(REC_PCircRecord list, int n, int ncar) {
    int i, l;

    if (ncar < 0 || ncar > 1)
        return -1;
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
    LLK_LastScan = 0;

    MENU_Init("mbg_reco.pix", NULL);
    memcpy(LLS_Screen[0], MENU_Back, LLS_Size);
    {
        IS2_PSprite sp;
        sp = IS2_Load("rec_txt.is2");
        REQUIRE(sp != NULL);
        DRW_TranslatePtr = GL_ClrTable + 15*256;
        IS2_Draw(sp, 160-sp->w/2, 5, sp->w, sp->h);
        DISPOSE(sp);
    }
    {
        char buf[200];
        sprintf(buf, "%s Track records", CarName[ncar]);
        TEXT_GetExtent(&MENU_FontYellow, 0, 0, buf, &i, 0);
        TEXT_Write(&MENU_FontYellow, (320-i)/2, 60, buf, 0);
    }
    TEXT_Printf(&MENU_FontWhite,  10, 73, 0, "Name");
    TEXT_Printf(&MENU_FontWhite, 140, 73, 0, "Track");
    TEXT_Printf(&MENU_FontWhite, 200, 73, 0, "Best Lap");
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
        for (i = 0; i < n; i++) {
            TEXT_Printf(&MENU_FontYellow,  10, 90+13*i, 0, "%.8s", list[i].name[ncar]);
            TEXT_Printf(&MENU_FontYellow, 140, 90+13*i, 0, "%d",   list[i].ncirc[ncar]);
            TEXT_Printf(&MENU_FontYellow, 200, 90+13*i, 0, "%d'%02d\"%02d",
                list[i].time[ncar]/70/60, (list[i].time[ncar]/70)%60,
                list[i].time[ncar]%70*100/70);
        }
        LLS_Update();
        l += VBL_VSync(1);
    } while (l < 10*70 && LLK_LastScan == 0);

    VBL_FadePos = 0;    // Stop fading.
    VBL_DumpPalette(GamePal, 0, 256);
    VBL_VSync(0);   // Ensure you have it.
    VBL_VSync(1);
    MENU_End();
    if (LLK_LastScan == kESC)
        return -1;
    return 0;
}

PUBLIC int REC_InputBest(REC_PBestRecord list, int n, int nplayer, int ncirc, int nlaps, dword time, int ncar) {
    int i, l;
    int nr;
    REC_TBestRecord rec;
    int namelen;
    bool leave;
    char buf[100];

    if (ncar < 0 || ncar > 1)
        return -1;
    rec.time[ncar]  = time;
    rec.nlaps[ncar] = nlaps;
    rec.ncirc[ncar] = ncirc;
    memset(rec.name[ncar], 0, sizeof(rec.name[ncar]));
    nr = REC_AddBest(REC_Best, REC_NBest, &rec, ncar);
    if (nr < 0)
        return -1;

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
    LLK_LastScan = 0;

    MENU_Init("mbg_reco.pix", NULL);
    memcpy(LLS_Screen[0], MENU_Back, LLS_Size);
    {
        IS2_PSprite sp;
        sp = IS2_Load("rec_txt.is2");
        REQUIRE(sp != NULL);
        DRW_TranslatePtr = GL_ClrTable + 15*256;
        IS2_Draw(sp, 160-sp->w/2, 5, sp->w, sp->h);
        DISPOSE(sp);
    }
    sprintf(buf, "Best racer player %d", nplayer+1);
    TEXT_GetExtent(&MENU_FontYellow, 0, 0, buf, &i, 0);
    TEXT_Write(&MENU_FontYellow, (320-i)/2, 60, buf, 0);
    TEXT_Write(&MENU_FontWhite,  10, 73, "Name" , 0);
    TEXT_Write(&MENU_FontWhite, 130, 73, "Track", 0);
    TEXT_Write(&MENU_FontWhite, 190, 73, "Laps" , 0);
    TEXT_Write(&MENU_FontWhite, 240, 73, "Time" , 0);

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
    namelen = 0;
    leave = FALSE;
    do {
        byte c;
        memcpy(LLS_Screen[0], MENU_Back, LLS_Size);
        for (i = 0; i < n; i++) {
            if (list[i].nlaps[ncar] <= 0)
                break;
            if (i == nr) {
                FONT_TFont *f;
                int x;
                f = (l&16)?&MENU_FontYellow:&MENU_FontWhite;
                x = TEXT_Printf(f,  10, 90+13*i, 0, "%.8s", list[i].name[ncar]);
                if (l&32)
                    TEXT_Write(f, x, 90+13*i+5, "-", 0);
                TEXT_Printf(f, 130, 90+13*i, 0, "%d",    list[i].ncirc[ncar]);
                TEXT_Printf(f, 190, 90+13*i, 0, "%d",    list[i].nlaps[ncar]);
                TEXT_Printf(f, 240, 90+13*i, 0, "%d'%02d\"%02d",
                    list[i].time[ncar]/70/60, (list[i].time[ncar]/70)%60,
                    list[i].time[ncar]%70*100/70);
            } else {
                TEXT_Printf(&MENU_FontYellow,  10, 90+13*i, 0, "%.8s",  list[i].name[ncar]);
                TEXT_Printf(&MENU_FontYellow, 130, 90+13*i, 0, "%d",    list[i].ncirc[ncar]);
                TEXT_Printf(&MENU_FontYellow, 190, 90+13*i, 0, "%d",    list[i].nlaps[ncar]);
                TEXT_Printf(&MENU_FontYellow, 240, 90+13*i, 0, "%d'%02d\"%02d",
                    list[i].time[ncar]/70/60, (list[i].time[ncar]/70)%60,
                    list[i].time[ncar]%70*100/70);
            }
        }
        l += VBL_VSync(1);
        LLS_Update();

        c = LLK_LastScan;
        switch (c) {
            case kBACKSPACE:
                if (namelen > 0) list[nr].name[ncar][--namelen] = '\0';
                break;
            case kESC:
            case kENTER:
            case kKEYPADENTER:
                if (namelen == 0)
                    strcpy(list[nr].name[ncar], "Mr. Shy");
                leave = TRUE;
                break;
            default:
                if (namelen < sizeof(list[nr].name[ncar])) {
                    const char *p;
                    p = KEYN_FindKey(c);
                    if ((c != kSPACE) && (p == NULL || strlen(p) > 1))
                        break;
                    if (c == kSPACE)
                        list[nr].name[ncar][namelen++] = ' ';
                    else if (MENU_FontYellow.data[(byte)p[0]] != 255)
                        list[nr].name[ncar][namelen++] = p[0];
                    if (namelen < sizeof(list[nr].name[ncar]))
                        list[nr].name[ncar][namelen] = '\0';
                }
        }
        LLK_LastScan = 0;

    } while (!leave);
    REC_Save();

    VBL_FadePos = 0;    // Stop fading.
    VBL_DumpPalette(GamePal, 0, 256);
    VBL_VSync(0);   // Ensure you have it.
    VBL_VSync(1);
    MENU_End();
    return nr;
}



PUBLIC int REC_InputCirc(REC_PCircRecord list, int n, int nplayer, int ncirc, dword time, int ncar) {
    int i, l;
    int nr;
    REC_TCircRecord rec;
    int namelen;
    bool leave;
    char buf[100];

    if (ncar < 0 || ncar > 1)
        return -1;
    rec.time[ncar]  = time;
    rec.ncirc[ncar] = ncirc;
    memset(rec.name[ncar], 0, sizeof(rec.name[ncar]));
    nr = REC_AddCirc(REC_Circ, REC_NCirc, &rec, ncar);
    if (nr < 0)
        return -1;

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
    LLK_LastScan = 0;

    MENU_Init("mbg_reco.pix", NULL);
    memcpy(LLS_Screen[0], MENU_Back, LLS_Size);
    {
        IS2_PSprite sp;
        sp = IS2_Load("rec_txt.is2");
        REQUIRE(sp != NULL);
        DRW_TranslatePtr = GL_ClrTable + 15*256;
        IS2_Draw(sp, 160-sp->w/2, 5, sp->w, sp->h);
        DISPOSE(sp);
    }
    sprintf(buf, "Track Record player %d", nplayer+1);
    TEXT_GetExtent(&MENU_FontYellow, 0, 0, buf, &i, 0);
    TEXT_Write(&MENU_FontYellow, (320-i)/2, 60, buf, 0);
    TEXT_Printf(&MENU_FontWhite,  10, 73, 0, "Name");
    TEXT_Printf(&MENU_FontWhite, 140, 73, 0, "Track");
    TEXT_Printf(&MENU_FontWhite, 200, 73, 0, "Best Lap");

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
    namelen = 0;
    leave = FALSE;
    do {
        byte c;
        memcpy(LLS_Screen[0], MENU_Back, LLS_Size);
        for (i = 0; i < n; i++) {
            if (i == nr) {
                FONT_TFont *f;
                int x;
                f = (l&16)?&MENU_FontYellow:&MENU_FontWhite;
                x = TEXT_Printf(f,  10, 90+13*i, 0, "%.8s", list[i].name[ncar]);
                if (l&32)
                    TEXT_Write(f, x, 90+13*i+5, "-", 0);
                TEXT_Printf(f, 140, 90+13*i, 0, "%d",   list[i].ncirc[ncar]);
                TEXT_Printf(f, 200, 90+13*i, 0, "%d'%02d\"%02d",
                    list[i].time[ncar]/70/60, (list[i].time[ncar]/70)%60,
                    list[i].time[ncar]%70*100/70);
            } else {
                TEXT_Printf(&MENU_FontYellow,  10, 90+13*i, 0, "%.8s", list[i].name[ncar]);
                TEXT_Printf(&MENU_FontYellow, 140, 90+13*i, 0, "%d",   list[i].ncirc[ncar]);
                TEXT_Printf(&MENU_FontYellow, 200, 90+13*i, 0, "%d'%02d\"%02d",
                    list[i].time[ncar]/70/60, (list[i].time[ncar]/70)%60,
                    list[i].time[ncar]%70*100/70);
            }
        }
        l += VBL_VSync(1);
        LLS_Update();

        c = LLK_LastScan;
        switch (c) {
            case kBACKSPACE:
                if (namelen > 0) list[nr].name[ncar][--namelen] = '\0';
                break;
            case kESC:
            case kENTER:
            case kKEYPADENTER:
                if (namelen == 0)
                    strcpy(list[nr].name[ncar], "Mr. Shy");
                leave = TRUE;
                break;
            default:
                if (namelen < sizeof(list[nr].name[ncar])) {
                    const char *p;
                    p = KEYN_FindKey(c);
                    if ((c != kSPACE) && (p == NULL || strlen(p) > 1))
                        break;
                    if (c == kSPACE)
                        list[nr].name[ncar][namelen++] = ' ';
                    else if (MENU_FontYellow.data[(byte)p[0]] != 255)
                        list[nr].name[ncar][namelen++] = p[0];
                    if (namelen < sizeof(list[nr].name[ncar]))
                        list[nr].name[ncar][namelen] = '\0';
                }
        }
        LLK_LastScan = 0;
    } while (!leave);
    REC_Save();

    VBL_FadePos = 0;    // Stop fading.
    VBL_DumpPalette(GamePal, 0, 256);
    VBL_VSync(0);   // Ensure you have it.
    VBL_VSync(1);
    MENU_End();
    return nr;
}


// ------------------------------ RECORDS.C ----------------------------
