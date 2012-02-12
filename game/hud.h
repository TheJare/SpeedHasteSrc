// ------------------------------ HUD.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _MARCA_H_
#define _MARCA_H_

#include <is2code.h>
#include "racemap.h"
#include "userctl.h"

PUBLIC bool HUD_Init(void);

PUBLIC void HUD_End(void);

PUBLIC void HUD_DrawSalp(int cy);

PUBLIC void HUD_DrawPlayer(PLY_PPlayer p, dword clock, int mode, bool salp);

PUBLIC void HUD_DrawPlayerLogos(PLY_PPlayer p, dword clock, int mode, THN_PThing first);

PUBLIC void HUD_Draw2Player(PLY_PPlayer p, dword clock, int mode, int y, bool salp);

PUBLIC void HUD_Draw2PlayerLogos(PLY_PPlayer p, dword clock, int mode, int y, THN_PThing first);

PUBLIC void HUD_DrawMap(MAP_PMap map, uint32 x, uint32 y, word angle);

PUBLIC IS2_PSprite
    HUD_Gear,
    HUD_Revo, HUD_Pos, HUD_Time, HUD_Laps,
    HUD_MapDot, HUD_Lap, HUD_Best, HUD_Record,

    HUD_BigGold[10], HUD_MedGold[10],
    HUD_BigGoldB, HUD_MedGoldB,
    HUD_BigWhite[10], HUD_MedWhite[10],
    HUD_LittleWhite[12], HUD_LittleGold[12],

    HUD_Gears[7],

    HUD_PosBar, HUD_Pause;

#endif

// ------------------------------ HUD.H ----------------------------

