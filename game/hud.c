// ------------------------------ HUD.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "hud.h"

#include <polygon.h>
#include <sincos.h>
#include <string.h>
#include <stdio.h>
#include <text.h>
#include <llscreen.h>
#include <vertdraw.h>
#include <jclib.h>

#include "globals.h"
#include "3dfloor.h"


PRIVATE IS2_PSprite RExttime, RFinlap, RTimeout, REndRace, RYouWin;

PRIVATE byte *Salp = NULL;
PRIVATE int   SalpSize = 0;

PRIVATE void LoadFont(IS2_PSprite *p, const char *name) {
    char buf[50];
    int i;

    for (i = 0; i < 10; i++) {
        sprintf(buf, "%s%d.is2", name, i);
        p[i] = IS2_Load(buf);
    }
}

PRIVATE void EndFont(IS2_PSprite *p) {
    int i;
    for (i = 0; i < 10; i++)
        DISPOSE(p[i]);
}

PUBLIC bool HUD_Init(void) {
    if (HUD_Gear == NULL) {
        char buf[50];
        int i;

        for (i = 0; i < 7; i++) {
            sprintf(buf, "mg%d.is2", i);
            HUD_Gears[i] = IS2_Load(buf);
        }

        HUD_Gear = IS2_Load("mgear.is2");
        sprintf(buf, "mrevo%d.is2", GL_CarType);
        HUD_Revo = IS2_Load(buf);
        HUD_Time = IS2_Load("mtime.is2");
        HUD_Laps = IS2_Load("mlaps.is2");
        HUD_Pos  = IS2_Load("mpos.is2");

        HUD_LittleWhite[10] = IS2_Load("mwquote.is2");
        HUD_LittleWhite[11] = IS2_Load("mwdquote.is2");
        HUD_LittleGold[10]  = IS2_Load("mgquote.is2");
        HUD_LittleGold[11]  = IS2_Load("mgdquote.is2");

        LoadFont(HUD_BigGold,     "mfbg");
        HUD_BigGoldB = IS2_Load("mfbgb.is2");
        LoadFont(HUD_MedGold,     "mfmg");
        HUD_MedGoldB = IS2_Load("mfmgb.is2");
        LoadFont(HUD_BigWhite,    "mfbw");
        LoadFont(HUD_MedWhite,    "mfmw");
        LoadFont(HUD_LittleWhite, "mflw");
        LoadFont(HUD_LittleGold,  "mflg");

        HUD_Lap    = IS2_Load("mlap.is2");
        HUD_Best   = IS2_Load("mbest.is2");
        HUD_Record = IS2_Load("mrecord.is2");

        HUD_PosBar = IS2_Load("mposbar.is2");

        HUD_Pause  = IS2_Load("pause.is2");

        RExttime = IS2_Load("rexttime.is2");
        RFinlap  = IS2_Load("rfinlap.is2");
        RTimeout = IS2_Load("rtimeout.is2");
        REndRace = IS2_Load("endrace.is2");
        RYouWin  = IS2_Load("youwin.is2");

        sprintf(buf, "salp%d.pix", GL_CarType);
        SalpSize = JCLIB_FileSize(buf);
        Salp = NEW(SalpSize);
        if (Salp != NULL)
            if (JCLIB_Load(buf, Salp, SalpSize) < SalpSize)
                DISPOSE(Salp);

    }
    return TRUE;
}


PRIVATE void DrawPolyMap(const POLY_TFullVertex *verts, int nverts) {
    int i;
    sint32 minx, miny, maxx, maxy;
    int minyindex;
    int nscans;
    byte c;

    assert(verts != NULL);
    assert(nverts > 0);

    minyindex = 0;
    minx = maxx = verts[0].x;
    miny = maxy = verts[0].y;
    for (i = 1; i < nverts; i++) {
        if (minx > verts[i].x)
            minx = verts[i].x;
        if (maxx < verts[i].x)
            maxx = verts[i].x;
        if (miny > verts[i].y) {
            miny = verts[i].y;
            minyindex = i;
        }
        if (maxy < verts[i].y)
            maxy = verts[i].y;
    }
    if (maxx <= 0 || maxy <= 0 || minx >= LLS_SizeX || miny >= LLS_SizeY
        || maxx <= minx || maxy <= miny)
        return;

    POLY_MaxX = LLS_SizeX;
    POLY_MaxY = LLS_SizeY;
    nscans = POLY_TracePoly(POLY_FullEdgeBuf, verts,
                            minyindex, -1, nverts,
                            PTRF_EDGE);
    if (nscans <= 0)
        return;
    assert(nscans <= 200);
    if (nscans != POLY_TracePoly(POLY_FullEdgeBufRight, verts,
                                 minyindex, 1, nverts,
                                 PTRF_EDGE))
        return;
    if (miny <= 0)
        miny = 0;
    for (i = 0; i < nscans; i++) {
        int w;

        w = POLY_FullEdgeBuf[i].x1-POLY_FullEdgeBuf[i].x0;
        if (w <= 0)
            continue;
/*
        FL_DrawRasterTrans(((byte*)LLS_Screen) + LLS_SizeX*(miny+i) + POLY_FullEdgeBuf[i].x1,
                       w,
                       POLY_FullEdgeBuf, nscans, LLS_SizeX, c);
*/
    }
}

PUBLIC void HUD_DrawMap(MAP_PMap map, uint32 x, uint32 y, word angle) {
    byte *p, *dest;
    sint32 dx, dy;
    int r, w, h;

    assert(map != NULL);
    FL_SetMap(map->map, NULL);

    r = 32;

    POLY_ScrapPoly[0].x = 320-r + FPMult(r, Cos(angle+0*65536/4));
    POLY_ScrapPoly[0].y = 320-r + FPMult(r, Sin(angle+0*65536/4));
    POLY_ScrapPoly[1].x = 320-r + FPMult(r, Cos(angle+1*65536/4));
    POLY_ScrapPoly[1].y = 320-r + FPMult(r, Sin(angle+1*65536/4));
    POLY_ScrapPoly[2].x = 320-r + FPMult(r, Cos(angle+2*65536/4));
    POLY_ScrapPoly[2].y = 320-r + FPMult(r, Sin(angle+2*65536/4));
    POLY_ScrapPoly[3].x = 320-r + FPMult(r, Cos(angle+3*65536/4));
    POLY_ScrapPoly[3].y = 320-r + FPMult(r, Sin(angle+3*65536/4));

    dx = FPMult(1 << 22, Cos(angle));
    dy = FPMult(1 << 22, Sin(angle));

    h = 64; w = 64;
    dest = LLS_Screen[0]+0;
    p = dest;
    while (h--) {
        FL_DrawRasterTrans(p, w, x, y, dx, dy);
        p += LLS_SizeX;
        x += -dy;
        y +=  dx;
    }
}

PUBLIC void HUD_End(void) {
    if (HUD_Gear != NULL) {
        int i;

        DISPOSE(Salp);

        DISPOSE(RYouWin);
        DISPOSE(REndRace);
        DISPOSE(RTimeout);
        DISPOSE(RFinlap);
        DISPOSE(RExttime);

        DISPOSE(HUD_Pause);

        DISPOSE(HUD_PosBar);

        DISPOSE(HUD_Record);
        DISPOSE(HUD_Best);
        DISPOSE(HUD_Lap);

        EndFont(HUD_LittleGold);
        EndFont(HUD_LittleWhite);
        EndFont(HUD_MedWhite);
        EndFont(HUD_BigWhite);
        DISPOSE(HUD_MedGoldB);
        EndFont(HUD_MedGold);
        DISPOSE(HUD_BigGoldB);
        EndFont(HUD_BigGold);

        DISPOSE(HUD_LittleGold[11]);
        DISPOSE(HUD_LittleGold[10]);
        DISPOSE(HUD_LittleWhite[11]);
        DISPOSE(HUD_LittleWhite[10]);

        DISPOSE(HUD_Pos);
        DISPOSE(HUD_Laps);
        DISPOSE(HUD_Time);
        DISPOSE(HUD_Revo);
        DISPOSE(HUD_Gear);

        for (i = 0; i < 7; i++)
            DISPOSE(HUD_Gears[i]);

    }
}

PRIVATE int DrawNum(int n, IS2_PSprite *font, int x, int y, int mode) {
    char buf[50];
    IS2_PSprite p;
    int i, l, w;

    sprintf(buf, "%u", n);
    l = strlen(buf);
    if (mode != 0) {
        w = 0;
        for (i = 0; i < l; i++) {
            p = font[buf[i] - '0'];
            w += p->w + 1;
        }
        w--; // Remove the last +1
        if (mode == 1)
            x -= w/2;
        else if (mode == 2)
            x -= w;
    }
    for (i = 0; i < l; i++) {
        IS2_PSprite p;

        p = font[buf[i] - '0'];
        IS2_DrawHorizontal(p, x, y);
        x += p->w + 1;
    }
    return x-1;
}

PRIVATE void DrawTime(int n, IS2_PSprite *font, int x, int y) {
    char buf[200];
    int i, l;

    sprintf(buf, "%02.2d\'%02.2d\"%02.2d", n/70/60, (n/70)%60, (n%70)*100/70);
    for (i = 0; buf[i] != '\0'; i++) {
        IS2_PSprite p;

        p = NULL;
        if (buf[i] >= '0' && buf[i] <= '9')
            p = font[buf[i] - '0'];
        else if (buf[i] == '\'')
            p = font[10];
        else if (buf[i] == '\"')
            p = font[11];
        if (p != NULL) {
            IS2_DrawHorizontal(p, x, y);
            x += p->w;
        } else
            x += 4;
    }
}

extern void DmpTrans(byte *dest, const byte *org, int nb);
#pragma aux DmpTrans modify [EAX] parm [EDI] [ESI] [ECX] = \
    "drl:             "  \
    "    MOV AL,[ESI] "  \
    "    INC ESI      "  \
    "    TEST AL,AL   "  \
    "    JZ trc       "  \
    "drc:             "  \
    "    MOV [EDI],AL "  \
    "    INC EDI      "  \
    "    DEC ECX      "  \
    "    JNZ drl      "  \
    "    JMP bye      "  \
    "trl:             "  \
    "    MOV AL,[ESI] "  \
    "    INC ESI      "  \
    "    TEST AL,AL   "  \
    "    JNZ drc      "  \
    "trc:             "  \
    "    INC EDI      "  \
    "    DEC ECX      "  \
    "    JNZ trl      "  \
    "bye:             "

extern void DmpTrans640(byte *dest, const byte *org, int nb);
#pragma aux DmpTrans640 modify [EAX] parm [EDI] [ESI] [ECX] = \
    "drl:             "  \
    "    MOV AL,[ESI] "  \
    "    INC ESI      "  \
    "    TEST AL,AL   "  \
    "    JZ trc       "  \
    "drc:             "  \
    "    MOV AH,AL    "  \
    "    MOV [EDI],AX "  \
    "    ADD EDI,2    "  \
    "    DEC ECX      "  \
    "    MOV [EDI+640-2],AX "  \
    "    JNZ drl      "  \
    "    JMP bye      "  \
    "trl:             "  \
    "    MOV AL,[ESI] "  \
    "    INC ESI      "  \
    "    TEST AL,AL   "  \
    "    JNZ drc      "  \
    "trc:             "  \
    "    ADD EDI,2    "  \
    "    DEC ECX      "  \
    "    JNZ trl      "  \
    "bye:             "


PUBLIC void HUD_DrawSalp(int cy) {
    byte *p = Salp;
    int i;

    if (GL_ScreenW == 320)
        cy = cy - SalpSize/320;
    else
        cy = cy - SalpSize/320*2;
    if (p == NULL)
        return;
    i = SalpSize;
    while (i >= 320 && cy < DRW_MaxY) {
        if (GL_ScreenW == 320)
            DmpTrans(LLS_Screen[0] + LLS_SizeX*cy, p, 320);
        else {
            DmpTrans640(LLS_Screen[0] + LLS_SizeX*cy, p, 320);
            cy++;
        }
        p += 320;
        i -= 320;
        cy++;
    }
}

PUBLIC void HUD_Draw(int time, int speed, int revo, int gear, int pos, int total) {
    int i;

    IS2_DrawHorizontal(HUD_Gear,   298, 123);
    IS2_DrawHorizontal(HUD_Gears[gear], 299, 123+12+8*gear);
    IS2_DrawHorizontal(HUD_Revo,   228, 138);
    DrawNum(speed, HUD_MedWhite,   263, 197-HUD_MedWhite[0]->h, 0);
    IS2_DrawHorizontal(HUD_Pos,      8, 158);
    i = DrawNum(pos, HUD_BigWhite,   3, 197-HUD_BigWhite[0]->h, 0);
    IS2_DrawHorizontal(HUD_PosBar, i+1, 197-HUD_PosBar->h);
    i += HUD_PosBar->w+1;
    DrawNum(total, HUD_MedWhite, i, 197-HUD_MedWhite[0]->h, 0);

    IS2_DrawHorizontal(HUD_Time,   160-HUD_Time->w/2,   8);
    DrawNum(time, HUD_BigGold, 160, 20, 2);

    {
        sint32 l = 0xC000-FPMultDiv(revo, 0xC000, 13000);

#define CX 258
#define CY 165

        POLY_ScrapPoly[0].l = 0xF0000;
        POLY_ScrapPoly[0].x = CX+FPMult( 21, Cos(l-0*65536/4));
        POLY_ScrapPoly[0].y = CY+FPMult( 21, Sin(l-0*65536/4));
        POLY_ScrapPoly[1].x = CX+FPMult(  3, Cos(l-1*65536/4));
        POLY_ScrapPoly[1].y = CY+FPMult(  3, Sin(l-1*65536/4));
        POLY_ScrapPoly[2].x = CX+FPMult(  3, Cos(l-2*65536/4));
        POLY_ScrapPoly[2].y = CY+FPMult(  3, Sin(l-2*65536/4));
        POLY_ScrapPoly[3].x = CX+FPMult(  3, Cos(l-3*65536/4));
        POLY_ScrapPoly[3].y = CY+FPMult(  3, Sin(l-3*65536/4));
        if (revo < 10000)
            POLY_SolidDraw(POLY_ScrapPoly, 4, 14);
        else
            POLY_SolidDraw(POLY_ScrapPoly, 4, 4);
    }
    if (Paused)
        TEXT_Write(&FONT_Border, 130, 75, "PAUSED", 12);
}

PUBLIC void HUD_DrawPlayer(PLY_PPlayer p, dword clock, int mode, bool salp) {
    int i;
    int time, laps, gear, revo, speed, pos, total;

    laps = p->racer->nlap+1;
    if (GL_TimedRace)
        time = (p->racer->laptimeleft/70);
    gear = p->gear;
    if (gear > 0)
        gear = gear-1;
    revo = FPMultDiv(p->revo, 13000, 1 << 22);
    speed = Abs32(FPMultDiv(p->v, p->maxspeed, 1 << 22));
    for (i = 0; i < Map.nracers; i++)
        if (Map.racers[i]->thn == p->thn) {
            pos = i+1;
            break;
        }
    REQUIRE(i < Map.nracers);
    total = Map.nracers;

    if (salp) {
        sint32 l = 0xC000-FPMultDiv(revo, 0xC000, 13000);
        int cx, cy;
        sint32 r;

        switch (GL_CarType) {
            case 0: cx = 182; cy = 160; break;
            case 1: cx = 119; cy = 175; break;
            default:
            case 2: cx =  98; cy = 175; break;
        }
        if (GL_ScreenW == 320)
            r = 3;
        else {
            cx = cx*2+GL_ScreenMinX;
            cy = cy*2+GL_ScreenMinY;
            r = 6;
        }

        POLY_ScrapPoly[0].l = 0xF0000;
        POLY_ScrapPoly[0].x = cx+FPMult(7*r, Cos(l-0*65536/4));
        POLY_ScrapPoly[0].y = cy+FPMult(7*r, Sin(l-0*65536/4));
        POLY_ScrapPoly[1].x = cx+FPMult(  r, Cos(l-1*65536/4));
        POLY_ScrapPoly[1].y = cy+FPMult(  r, Sin(l-1*65536/4));
        POLY_ScrapPoly[2].x = cx+FPMult(  r, Cos(l-2*65536/4));
        POLY_ScrapPoly[2].y = cy+FPMult(  r, Sin(l-2*65536/4));
        POLY_ScrapPoly[3].x = cx+FPMult(  r, Cos(l-3*65536/4));
        POLY_ScrapPoly[3].y = cy+FPMult(  r, Sin(l-3*65536/4));
        if (revo < 10000)
            POLY_SolidDraw(POLY_ScrapPoly, 4, 14);
        else
            POLY_SolidDraw(POLY_ScrapPoly, 4, 4);
    }

    if (salp && GL_CarType != 2) {
        sint32 l = 0xC000-FPMultDiv(speed, 0xC000, p->maxspeed);
        int cx, cy;
        sint32 r;

        switch (GL_CarType) {
            case 0: cx = 132; cy = 160; break;
            default:
            case 1: cx =  70; cy = 175; break;
        }
        if (GL_ScreenW == 320)
            r = 3;
        else {
            cx = cx*2+GL_ScreenMinX;
            cy = cy*2+GL_ScreenMinY;
            r = 6;
        }

        POLY_ScrapPoly[0].l = 0xF0000;
        POLY_ScrapPoly[0].x = cx+FPMult(7*r, Cos(l-0*65536/4));
        POLY_ScrapPoly[0].y = cy+FPMult(7*r, Sin(l-0*65536/4));
        POLY_ScrapPoly[1].x = cx+FPMult(  r, Cos(l-1*65536/4));
        POLY_ScrapPoly[1].y = cy+FPMult(  r, Sin(l-1*65536/4));
        POLY_ScrapPoly[2].x = cx+FPMult(  r, Cos(l-2*65536/4));
        POLY_ScrapPoly[2].y = cy+FPMult(  r, Sin(l-2*65536/4));
        POLY_ScrapPoly[3].x = cx+FPMult(  r, Cos(l-3*65536/4));
        POLY_ScrapPoly[3].y = cy+FPMult(  r, Sin(l-3*65536/4));
        POLY_SolidDraw(POLY_ScrapPoly, 4, 7);
    }

    IS2_DrawHorizontal(HUD_Gear,   GL_ScreenMaxX - 22, GL_ScreenMaxY - 77);
    IS2_DrawHorizontal(HUD_Gears[gear], GL_ScreenMaxX - 21, GL_ScreenMaxY - 77+12+8*gear);
    if (!salp)
        IS2_DrawHorizontal(HUD_Revo,   GL_ScreenMaxX - 92, GL_ScreenMaxY - 62);
    if (!salp) {
        if (GL_CarType != 1)
            DrawNum(speed, HUD_MedWhite, GL_ScreenMaxX - 320 + 263, GL_ScreenMaxY - 200 + 197 - HUD_MedWhite[0]->h, 0);
        else
            DrawNum(speed, HUD_MedWhite, GL_ScreenMaxX - 320 + 263, GL_ScreenMaxY - 200 + 191-HUD_MedWhite[0]->h, 0);
    }
    if (mode < 2) {
        IS2_DrawHorizontal(HUD_Pos,      8, GL_ScreenMaxY - 200 + 158);
        i = DrawNum(pos, HUD_BigWhite,   3, GL_ScreenMaxY - 200 + 197-HUD_BigWhite[0]->h, 0);
        IS2_DrawHorizontal(HUD_PosBar, i+1, GL_ScreenMaxY - 200 + 197-HUD_PosBar->h);
        i += HUD_PosBar->w+1;
        DrawNum(total, HUD_MedWhite, i, GL_ScreenMaxY - 200 + 197-HUD_MedWhite[0]->h, 0);
    }

    if (!GL_TimedRace) {
        if (mode < 2) {
            IS2_DrawHorizontal(HUD_Laps,   GL_ScreenCenterX-HUD_Laps->w/2,        GL_ScreenMinY + 8);
            IS2_DrawHorizontal(HUD_BigGoldB, GL_ScreenCenterX-HUD_BigGoldB->w/2,  GL_ScreenMinY + 20+HUD_BigGold[0]->h-HUD_BigGoldB->h);
            DrawNum(laps, HUD_BigGold, GL_ScreenCenterX-HUD_BigGoldB->w/2,        GL_ScreenMinY + 20,                                     2);
            DrawNum(GL_RaceLaps, HUD_MedGold, GL_ScreenCenterX+HUD_BigGoldB->w/2, GL_ScreenMinY + 20+HUD_BigGold[0]->h-HUD_MedGold[0]->h, 0);
        } else {
            IS2_DrawHorizontal(HUD_Laps, GL_ScreenCenterX-HUD_Laps->w/2, GL_ScreenMinY + 8);
            DrawNum(laps, HUD_BigGold, GL_ScreenCenterX, GL_ScreenMinY + 20, 1);
        }
    } else {
        IS2_DrawHorizontal(HUD_Time,   GL_ScreenCenterX-HUD_Time->w/2, GL_ScreenMinY + 8);
        DrawNum(time, HUD_BigGold, GL_ScreenCenterX,                   GL_ScreenMinY + 20, 1);
        IS2_DrawHorizontal(HUD_Laps,   GL_ScreenCenterX-HUD_Time->w/2-30-HUD_Laps->w, GL_ScreenMinY + 8);
        DrawNum(laps, HUD_BigGold, GL_ScreenCenterX-HUD_Laps->w-30,                   GL_ScreenMinY + 20, 1);
    }

    if (p->racer->bestlaptime != 0) {
        IS2_Draw(HUD_Best, 4, GL_ScreenMinY + 65, HUD_Best->w, HUD_Best->h);
        if (p->status == PLYST_RACING || p->clock & 0x40)
            DrawTime(p->racer->bestlaptime, HUD_LittleWhite, 10, GL_ScreenMinY + 75);
    }
    if (p->status == PLYST_RACING) {
        int nl = p->racer->nlap;
        IS2_Draw(HUD_Lap, 4, GL_ScreenMinY + 85, HUD_Lap->w, HUD_Lap->h);
        if (nl >= SIZEARRAY(p->racer->laptime))
            nl =  SIZEARRAY(p->racer->laptime) - 1;
        i = 0;
        while (nl >= 0) {
            DrawTime(p->racer->laptime[nl], HUD_LittleGold, 10, GL_ScreenMinY + 95+i*10);
            nl--;
            i++;
        }
    }

    if (!salp) {
        sint32 l = 0xC000-FPMultDiv(revo, 0xC000, 13000);
        int cx, cy;
        sint32 r;

        cx = 258;
        cy = 165;

        if (GL_ScreenW == 320)
            r = 3;
        else {
            cx = GL_ScreenMaxX - (320 - cx);
            cy = GL_ScreenMaxY - (200 - cy);
            r = 3;
        }

        POLY_ScrapPoly[0].l = 0xF0000;
        POLY_ScrapPoly[0].x = cx+FPMult(7*r, Cos(l-0*65536/4));
        POLY_ScrapPoly[0].y = cy+FPMult(7*r, Sin(l-0*65536/4));
        POLY_ScrapPoly[1].x = cx+FPMult(  r, Cos(l-1*65536/4));
        POLY_ScrapPoly[1].y = cy+FPMult(  r, Sin(l-1*65536/4));
        POLY_ScrapPoly[2].x = cx+FPMult(  r, Cos(l-2*65536/4));
        POLY_ScrapPoly[2].y = cy+FPMult(  r, Sin(l-2*65536/4));
        POLY_ScrapPoly[3].x = cx+FPMult(  r, Cos(l-3*65536/4));
        POLY_ScrapPoly[3].y = cy+FPMult(  r, Sin(l-3*65536/4));
        if (revo < 10000)
            POLY_SolidDraw(POLY_ScrapPoly, 4, 14);
        else
            POLY_SolidDraw(POLY_ScrapPoly, 4, 4);
    }
}

PUBLIC void HUD_DrawPlayerLogos(PLY_PPlayer p, dword clock, int mode, THN_PThing first) {
    if (Paused)
//        TEXT_Write(&FONT_Border, GL_ScreenCenterX-160+130, GL_ScreenCenterX-100+75, "PAUSED", 12);
        IS2_Draw(HUD_Pause, GL_ScreenCenterX, GL_ScreenCenterY-GL_ScreenH/4, HUD_Pause->w, HUD_Pause->h);

    if (p->racer->clexttime & 0x10)
//        TEXT_Write(&FONT_Border, 130, 65, "EXTENDED TIME", 14);
        IS2_Draw(RExttime, GL_ScreenCenterX, GL_ScreenCenterY-40, RExttime->w, RExttime->h);
    if (p->racer->clfinallap & 0x10)
//        TEXT_Write(&FONT_Border, 130, 65, "FINAL LAP", 14);
        IS2_Draw(RFinlap, GL_ScreenCenterX, GL_ScreenCenterY-40, RFinlap->w, RFinlap->h);

    if (p->status == PLYST_WON && (p->clock & 0x20))
//        TEXT_Write(&FONT_Border, 130, 65, "END OF RACE", 15);
        if (p->thn == first)
            IS2_Draw(RYouWin, GL_ScreenCenterX, GL_ScreenCenterY-40, RYouWin->w, RYouWin->h);
        else
            IS2_Draw(REndRace, GL_ScreenCenterX, GL_ScreenCenterY-40, REndRace->w, REndRace->h);
    if (p->status == PLYST_TIMEOUT && (p->clock & 0x20))
//        TEXT_Write(&FONT_Border, 130, 65, "TIME OUT", 15);
        IS2_Draw(RTimeout, GL_ScreenCenterX, GL_ScreenCenterY-40, RTimeout->w, RTimeout->h);
}


PUBLIC void HUD_Draw2Player(PLY_PPlayer p, dword clock, int mode, int y, bool salp) {
    int i;
    int time, laps, gear, revo, speed, pos, total;

    laps = p->racer->nlap+1;
    if (GL_TimedRace)
        time = (p->racer->laptimeleft/70);
    gear = p->gear;
    if (gear > 0)
        gear = gear-1;
    revo = FPMultDiv(p->revo, 13000, 1 << 22);
    speed = Abs32(FPMultDiv(p->v, p->maxspeed, 1 << 22));
    for (i = 0; i < Map.nracers; i++)
        if (Map.racers[i]->thn == p->thn) {
            pos = i+1;
            break;
        }
    REQUIRE(i < Map.nracers);
    total = Map.nracers;

    IS2_DrawHorizontal(HUD_Gear,   GL_ScreenMaxX - 22, y + GL_ScreenH/2 - 77);
//    IS2_DrawHorizontal(HUD_Gear,   298, y+23+3);
    IS2_DrawHorizontal(HUD_Gears[gear], GL_ScreenMaxX - 21, y + GL_ScreenH/2 - 77+12+8*gear);
//    IS2_DrawHorizontal(HUD_Gears[gear], 299, y+3+23+12+8*gear);
    IS2_DrawHorizontal(HUD_Revo,   GL_ScreenMaxX - 92, y + GL_ScreenH/2 - 62);
//    IS2_DrawHorizontal(HUD_Revo,   228, y+40);
    if (GL_CarType != 1)
/*
        DrawNum(speed, HUD_MedWhite,   263, y+97+6-HUD_MedWhite[0]->h, 0);
    else
        DrawNum(speed, HUD_MedWhite,   263, y+91+6-HUD_MedWhite[0]->h, 0);
*/
        DrawNum(speed, HUD_MedWhite, GL_ScreenMaxX - 320 + 263, y + GL_ScreenH/2 - 200 + 197 - HUD_MedWhite[0]->h, 0);
    else
        DrawNum(speed, HUD_MedWhite, GL_ScreenMaxX - 320 + 263, y + GL_ScreenH/2 - 200 + 191 - HUD_MedWhite[0]->h, 0);

    if (mode < 2) {
/*
        IS2_DrawHorizontal(HUD_Pos,      8, y+58);
        i = DrawNum(pos, HUD_BigWhite,   3, y+97-HUD_BigWhite[0]->h, 0);
        IS2_DrawHorizontal(HUD_PosBar, i+1, y+97-HUD_PosBar->h);
        i += HUD_PosBar->w+1;
        DrawNum(total, HUD_MedWhite, i, y+97-HUD_MedWhite[0]->h, 0);
*/
        IS2_DrawHorizontal(HUD_Pos,      8, y + GL_ScreenH/2 - 200 + 158);
        i = DrawNum(pos, HUD_BigWhite,   3, y + GL_ScreenH/2 - 200 + 197-HUD_BigWhite[0]->h, 0);
        IS2_DrawHorizontal(HUD_PosBar, i+1, y + GL_ScreenH/2 - 200 + 197-HUD_PosBar->h);
        i += HUD_PosBar->w+1;
        DrawNum(total, HUD_MedWhite, i, y + GL_ScreenH/2 - 200 + 197-HUD_MedWhite[0]->h, 0);
    }

    if (!GL_TimedRace) {
        if (mode < 2) {
            IS2_DrawHorizontal(HUD_Laps,   GL_ScreenCenterX-HUD_Laps->w/2,   y+1);
            IS2_DrawHorizontal(HUD_BigGoldB,   GL_ScreenCenterX-HUD_BigGoldB->w/2, y+10+HUD_BigGold[0]->h-HUD_BigGoldB->h);
            DrawNum(laps, HUD_BigGold, GL_ScreenCenterX-HUD_BigGoldB->w/2, y+10, 2);
            DrawNum(GL_RaceLaps, HUD_MedGold, GL_ScreenCenterX+HUD_BigGoldB->w/2, y+10+HUD_BigGold[0]->h-HUD_MedGold[0]->h, 0);
        } else {
            IS2_DrawHorizontal(HUD_Laps,   GL_ScreenCenterX-HUD_Laps->w/2,   y+1);
            DrawNum(laps, HUD_BigGold, GL_ScreenCenterX, y+10, 1);
        }
    } else {
        IS2_DrawHorizontal(HUD_Time,   GL_ScreenCenterX-HUD_Time->w/2,   y+0);
        DrawNum(time, HUD_BigGold, GL_ScreenCenterX, y+10, 1);
        IS2_DrawHorizontal(HUD_Laps,   GL_ScreenCenterX-HUD_Time->w/2-30-HUD_Laps->w, y + 0);
        DrawNum(laps, HUD_BigGold, GL_ScreenCenterX-HUD_Laps->w-30,                   y + 10, 1);
    }

    if (p->racer->bestlaptime != 0) {
        IS2_Draw(HUD_Best, 4, y+5, HUD_Best->w, HUD_Best->h);
        if (p->status == PLYST_RACING || p->clock & 0x40)
            DrawTime(p->racer->bestlaptime, HUD_LittleWhite, 10, y+15);
    }
    if (p->status == PLYST_RACING) {
        int nl = p->racer->nlap;
        IS2_Draw(HUD_Lap, 4, y+25, HUD_Lap->w, HUD_Lap->h);
        if (nl >= SIZEARRAY(p->racer->laptime))
            nl =  SIZEARRAY(p->racer->laptime) - 1;
        i = 0;
//        while (nl >= 0)
        {
            DrawTime(p->racer->laptime[nl], HUD_LittleGold, 10, y+35+i*10);
            nl--;
            i++;
        }
    }

    {
        sint32 l = 0xC000-FPMultDiv(revo, 0xC000, 13000);
        int cx, cy;
        sint32 r;

        cx = 258;
        cy = y+67;

        if (GL_ScreenW == 320)
            r = 3;
        else {
            cx = GL_ScreenMaxX - (320 - cx);
            cy = y + GL_ScreenH/2 - (100 - (cy-y));
            r = 3;
        }

        POLY_ScrapPoly[0].l = 0xF0000;
        POLY_ScrapPoly[0].x = cx+FPMult(7*r, Cos(l-0*65536/4));
        POLY_ScrapPoly[0].y = cy+FPMult(7*r, Sin(l-0*65536/4));
        POLY_ScrapPoly[1].x = cx+FPMult(  r, Cos(l-1*65536/4));
        POLY_ScrapPoly[1].y = cy+FPMult(  r, Sin(l-1*65536/4));
        POLY_ScrapPoly[2].x = cx+FPMult(  r, Cos(l-2*65536/4));
        POLY_ScrapPoly[2].y = cy+FPMult(  r, Sin(l-2*65536/4));
        POLY_ScrapPoly[3].x = cx+FPMult(  r, Cos(l-3*65536/4));
        POLY_ScrapPoly[3].y = cy+FPMult(  r, Sin(l-3*65536/4));
        if (revo < 10000)
            POLY_SolidDraw(POLY_ScrapPoly, 4, 14);
        else
            POLY_SolidDraw(POLY_ScrapPoly, 4, 4);
    }
}

PUBLIC void HUD_Draw2PlayerLogos(PLY_PPlayer p, dword clock, int mode, int y, THN_PThing first) {
    if (Paused)
//        TEXT_Write(&FONT_Border, GL_ScreenCenterX-160+130, GL_ScreenCenterX-100+50, "PAUSED", 12);
        IS2_Draw(HUD_Pause, GL_ScreenCenterX, y+GL_ScreenH/6, HUD_Pause->w, HUD_Pause->h);

    if (p->racer->clexttime & 0x10)
//        TEXT_Write(&FONT_Border, 130, 65, "EXTENDED TIME", 14);
        IS2_Draw(RExttime, GL_ScreenCenterX, y+GL_ScreenH/4-10, RExttime->w, RExttime->h);
    if (p->racer->clfinallap & 0x10)
//        TEXT_Write(&FONT_Border, 130, 65, "FINAL LAP", 14);
        IS2_Draw(RFinlap, GL_ScreenCenterX, y+GL_ScreenH/4-10, RFinlap->w, RFinlap->h);

    if (p->status == PLYST_WON && (p->clock & 0x20))
//        TEXT_Write(&FONT_Border, 130, y+50, "END OF RACE", 15);
        if (p->thn == first)
            IS2_Draw(RYouWin, GL_ScreenCenterX, y+GL_ScreenH/4-10, RYouWin->w, RYouWin->h);
        else
            IS2_Draw(REndRace, GL_ScreenCenterX, y+GL_ScreenH/4-10, REndRace->w, REndRace->h);
    if (p->status == PLYST_TIMEOUT && (p->clock & 0x20))
//        TEXT_Write(&FONT_Border, 130, y+50, "TIME OUT", 15);
        IS2_Draw(RTimeout, GL_ScreenCenterX, y+GL_ScreenH/4-10, RTimeout->w, RTimeout->h);
}

IS2_PSprite
    HUD_Gear = NULL,
    HUD_Revo, HUD_Pos, HUD_Time, HUD_Laps,
    HUD_MapDot, HUD_Lap, HUD_Best, HUD_Record,

    HUD_BigGold[10], HUD_MedGold[10],
    HUD_BigGoldB, HUD_MedGoldB,
    HUD_BigWhite[10], HUD_MedWhite[10],
    HUD_LittleWhite[12], HUD_LittleGold[12],

    HUD_Gears[7],

    HUD_PosBar, HUD_Pause;



// ------------------------------ HUD.C ----------------------------

