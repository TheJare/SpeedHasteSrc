
#include "race.h"

#include <llscreen.h>
#include <graphics.h>
#include <jclib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <sincos.h>
#include <llkey.h>
#include <strparse.h>
#include <text.h>
#include <gamevtal.h>
#include <atan.h>
#include <is2code.h>
#include <vertdraw.h>
#include <object3d.h>
#include <timer.h>
#include <polygon.h>
#include <joystick.h>
#include <lbm.h>

#include <sqrt.h>

#include "3dfloor.h"
#include "globals.h"
#include "userctl.h"
#include "hud.h"
#include "config.h"
#include "models.h"
#include "intro.h"
#include "menus.h"

#include "net.h"

    // fl2.c
extern void LogMemory(const char *text);
extern void GenericTimerFunction(void);

// =============================

PRIVATE F3D_TCamera c = {
    0 << 20,            // radius
    8192,               // h
    0x1CD0000,          // focus        max. == 0x1CD0000
    13,                 // horizon
    TRUE,               // hiDetail
};

// =============================

PRIVATE volatile byte ConsoleInput[MAX_USERCONTROLS];
PRIVATE int           ConsoleClock;

PRIVATE volatile PLY_PPlayer pplayer;

PRIVATE volatile PLY_PPlayer ConsolePlayer1 = NULL,
                             ConsolePlayer2 = NULL;

PRIVATE int                  NHumanPlayers;
PRIVATE volatile PLY_PPlayer HumanPlayers[MAX_HUMAN_PLAYERS];

#define MAXCARS   40

PRIVATE CAR_PCar cars[MAXCARS];
PRIVATE int NCars = 0;
PRIVATE int ViewCar = 0;
PRIVATE int CloseCar0 = 0;
PRIVATE int CloseCar1 = 0;

PRIVATE GAME_PEffect SH_Eff  = NULL;

PUBLIC  GAME_PEffect RACE_EffCrash   = NULL;
PUBLIC  GAME_PEffect RACE_EffDerrape = NULL;

PUBLIC  GAME_PEffect RACE_EffFinalLap  = NULL,
                     RACE_EffLapRecord = NULL,
                     RACE_EffTimeout   = NULL,
                     RACE_EffExtended  = NULL,
                     RACE_EffYouWin    = NULL,
                     RACE_EffGameOver  = NULL;

PRIVATE THN_PThing RacePos[5];
PRIVATE THN_PThing HumanPos[MAX_HUMAN_PLAYERS];

PRIVATE bool ESCPressed = FALSE;

PRIVATE bool CommTrace = FALSE;

PRIVATE bool ViewHud[2];

PRIVATE IS2_PSprite PlayerPosDots[5+1+1+1];

PRIVATE IS2_PSprite HumanSprs[MAX_HUMAN_PLAYERS];

PRIVATE bool DevKeys = FALSE;

PRIVATE int Quitting = 0;

// =============================

PRIVATE void InitScreenParms(void) {
    GL_ScreenW    = LLS_SizeX;
    GL_ScreenMinX = 0;
    if (LLS_SizeY == 480) {
        GL_ScreenH = 400;
        GL_ScreenMinY = 40;
    } else {
        GL_ScreenH = LLS_SizeY;
        GL_ScreenMinY = 0;
    }
    GL_ScreenMaxX = GL_ScreenMinX + GL_ScreenW;
    GL_ScreenMaxY = GL_ScreenMinY + GL_ScreenH;
    GL_ScreenXRatio = GL_ScreenW/320;
    GL_ScreenYRatio = GL_ScreenH/200;
    GL_ScreenCenterX = GL_ScreenMinX + GL_ScreenW/2;
    GL_ScreenCenterY = GL_ScreenMinY + GL_ScreenH/2;

    POLY_MinX = GL_ScreenMinX; POLY_MaxX = GL_ScreenMaxX;
    POLY_MinY = GL_ScreenMinY; POLY_MaxY = GL_ScreenMaxY;
    DRW_SetClipZone(GL_ScreenMinX, GL_ScreenMinY, GL_ScreenMaxX, GL_ScreenMaxY, NULL);
    R3D_FocusX = 256*GL_ScreenW/320;
    R3D_FocusY = 256*GL_ScreenH/240;
    R3D_CenterX = GL_ScreenCenterX;
    R3D_CenterY = GL_ScreenCenterY;
}

PRIVATE bool InitData(int mode, int addPlayers) {
    int i;
    THN_PThing p;
    char buf[30];

    Quitting = 0;
    DevKeys = (BASE_CheckArg("devkeys") > 0);

    MDL_InitCircuits();
    MDL_InitCars();

    GL_RaceTime[0] = (MDL_Circuits[GL_SelCircuit].record+15)*70;
    GL_RaceTime[1] = (MDL_Circuits[GL_SelCircuit].record- 0)*70;
    GL_RaceTime[2] = (MDL_Circuits[GL_SelCircuit].record- 2)*70;
    GL_RaceTime[3] = (MDL_Circuits[GL_SelCircuit].record- 4)*70;
    GL_RaceTime[4] = (MDL_Circuits[GL_SelCircuit].record- 5)*70;


//    LogMemory("InitData: Probando SEC");
    sprintf(buf, "map%02d.sec", GL_SelCircuit);
    if (!SEC_LoadMap(&Map.sec, buf))
        BASE_Abort("Can't load sectormap %s!", buf);
/*
    LogMemory("InitData: Entre Probando SEC");
    SEC_EndMap(&Map.sec);
    LogMemory("InitData: Fin de Probando SEC");
    sprintf(buf, "map%02d.sec", GL_SelCircuit);
    REQUIRE(SEC_LoadMap(&Map.sec, buf));
*/

//    LogMemory("InitData: Probando Map");
    sprintf(buf, "map%02d.dat", GL_SelCircuit);
    if (!MAP_Load(&Map, buf))
        BASE_Abort("Can't load map %s!", buf);
/*
    LogMemory("InitData: Entre Probando Map");
    MAP_Free(&Map);
    LogMemory("InitData: Fin Probando Map");
    sprintf(buf, "map%02d.dat", GL_SelCircuit);
    REQUIRE(MAP_Load(&Map, buf));
    LogMemory("InitData: Map cargado");
*/

    if (!RCS_Init(40))
        BASE_Abort("Out of memory for racers");

//printf("Reading map things...\n");
//    LogMemory("InitData: Probando Things");
    if (!MAP_LoadThings(&Map, buf))
        BASE_Abort("Can't load map things for map %s!", buf);
/*
    LogMemory("InitData: Entre probando Things");
    {
        THN_PThing p, g;
        p = THN_ThingList;
        while (p != NULL) {
            g = p->global.next;
            THN_DeleteThing(p);
            p = g;
        }
        p = THN_ThinkerList;
        while (p != NULL) {
            g = p->global.next;
            THN_DeleteThing(p);
            p = g;
        }
        MAP_EndStaticCameras(&Map);
        MAP_EndStartPos(&Map);
        FS3_End();
    }
    LogMemory("InitData: Fin de Probando Things");
    REQUIRE(MAP_LoadThings(&Map, buf));
    LogMemory("InitData: Things cargadas");
*/

//    LogMemory("InitData: Probando HUD");
    if (!HUD_Init())
        BASE_Abort("Missing file(s) for HUD initialization");
/*
    LogMemory("InitData: Entre probando HUD");
    HUD_End();
    LogMemory("InitData: Fin de Probando HUD");
    REQUIRE(HUD_Init());
*/

//    LogMemory("InitData: Probando Path");
    sprintf(buf, "map%02d.pth", GL_SelCircuit);
    PATH_Init(&Map.path, buf);
/*
    LogMemory("InitData: Entre Probando Path");
    PATH_Done(&Map.path);
    LogMemory("InitData: Fin de Probando Path");
    PATH_Init(&Map.path, buf);
*/

    NHumanPlayers = 0;
        // Setup human players.
    pplayer = ConsolePlayer1 = ConsolePlayer2 = NULL;
    for (i = 0; i < MAX_HUMAN_PLAYERS; i++) {
        HumanPlayers[i] = NULL;
    }

//    printf("Creating %d net players:\n", addPlayers*NET_NumNodes);

    if (addPlayers > 0) {
        int nnode;
        nnode = 0;
        do {
            bool isconsole;
            PLY_PPlayer pp;
            int npos;

            isconsole = (NET_State == NETS_NONE || NET_NodeNum[nnode] == 0);

/*
            printf("Creating a%s player at netnode %d, nodenum %d\n",
                    isconsole?" console":"", nnode, NET_NodeNum[nnode]);
*/
            npos = MAP_UseStartPos(&Map);
            if (npos >= 0)
                p = THN_AddThing(Map.startPos[npos].x,
                                 Map.startPos[npos].y,
                                 0x40000,
                                 THNT_PLAYER | ((GL_SelCar[2*NET_NodeNum[nnode]+0] % 6) << 4),
                                 Map.startPos[npos].angle, PLYF_ISCONSOLE*isconsole);
            else
                p = THN_AddThing(1 << 30, 1 << 30, 0x40000,
                                 THNT_PLAYER | ((GL_SelCar[2*NET_NodeNum[nnode]+0] % 6) << 4),
                                 0x6000, PLYF_ISCONSOLE*isconsole);
            if (p == NULL)
                BASE_Abort("Out of memory for human racer %d", NHumanPlayers+1);

            HumanPlayers[NHumanPlayers] = p->data;
            HumanPlayers[NHumanPlayers]->net = NET_NodeData + NET_NodeNum[nnode];
            NET_NodeData[NET_NodeNum[nnode]].p[0] = p->data;
            pp = p->data;
            if (isconsole) {
                ConsolePlayer1 = p->data;
                pplayer = ConsolePlayer1;
                pp->cfg.kup    = CFG_Config.P1KeyUp;
                pp->cfg.kdn    = CFG_Config.P1KeyDn;
                pp->cfg.kleft  = CFG_Config.P1KeyLt;
                pp->cfg.kright = CFG_Config.P1KeyRt;
                pp->cfg.kgear  = CFG_Config.P1KeyGe;
                pp->cfg.source = CFG_Config.P1Control;
            } else {
                pp->cfg.kup    = NET_NodeData[NET_NodeNum[nnode]].kup;
                pp->cfg.kdn    = NET_NodeData[NET_NodeNum[nnode]].kdn;
                pp->cfg.kleft  = NET_NodeData[NET_NodeNum[nnode]].klt;
                pp->cfg.kright = NET_NodeData[NET_NodeNum[nnode]].krt;
                pp->cfg.kgear  = NET_NodeData[NET_NodeNum[nnode]].kge;
                pp->cfg.source = UCTS_KEYBOARD;
            }

            pp->maxspeed = MDL_Cars[GL_SelCar[2*NET_NodeNum[nnode]+0]].maxSpeed;
            pp->automatic = !MDL_Cars[GL_SelCar[2*NET_NodeNum[nnode]+0]].automatic;
            pp->cartype = GL_CarType;
            pp->damage = 0;

/*
            printf("Car type %d, model %d, maxspeed %d, automatic %d\n",
                    pp->cartype, GL_SelCar[2*NET_NodeNum[nnode]+0],
                    pp->maxspeed, pp->automatic);
*/

            NHumanPlayers++;

            if (addPlayers > 1) {
                npos = MAP_UseStartPos(&Map);
                if (npos >= 0)
                    p = THN_AddThing(Map.startPos[npos].x,
                                     Map.startPos[npos].y,
                                     0x40000,
                                     THNT_PLAYER | ((GL_SelCar[2*NET_NodeNum[nnode]+1] % 6) << 4),
                                     Map.startPos[npos].angle, PLYF_ISCONSOLE*isconsole);
                else
                    p = THN_AddThing(1 << 30, 1 << 30, 0x40000,
                                     THNT_PLAYER | ((GL_SelCar[2*NET_NodeNum[nnode]+1] % 6) << 4),
                                     0x6000, PLYF_ISCONSOLE*isconsole);
                if (p == NULL)
                    BASE_Abort("Out of memory for human racer %d", NHumanPlayers+1);
                HumanPlayers[NHumanPlayers] = p->data;
                HumanPlayers[NHumanPlayers]->net = NET_NodeData + NET_NodeNum[nnode];
                NET_NodeData[NET_NodeNum[nnode]].p[1] = p->data;
                pp = p->data;
                if (isconsole) {
                    ConsolePlayer2 = p->data;
                    pp->cfg.kup    = CFG_Config.P2KeyUp;
                    pp->cfg.kdn    = CFG_Config.P2KeyDn;
                    pp->cfg.kleft  = CFG_Config.P2KeyLt;
                    pp->cfg.kright = CFG_Config.P2KeyRt;
                    pp->cfg.kgear  = CFG_Config.P2KeyGe;
                    pp->cfg.source = CFG_Config.P2Control;
                } else {
                    pp->cfg.kup    = NET_NodeData[NET_NodeNum[nnode]].kup;
                    pp->cfg.kdn    = NET_NodeData[NET_NodeNum[nnode]].kdn;
                    pp->cfg.kleft  = NET_NodeData[NET_NodeNum[nnode]].klt;
                    pp->cfg.kright = NET_NodeData[NET_NodeNum[nnode]].krt;
                    pp->cfg.kgear  = NET_NodeData[NET_NodeNum[nnode]].kge;
                    pp->cfg.source = UCTS_KEYBOARD;
                }
                pp->cartype = GL_CarType;
                pp->maxspeed = MDL_Cars[GL_SelCar[2*NET_NodeNum[nnode]+1]].maxSpeed;
                pp->automatic = !MDL_Cars[GL_SelCar[2*NET_NodeNum[nnode]+1]].automatic;
                pp->damage = 0;
                NHumanPlayers++;
            }
            nnode++;
        } while (NET_State != NETS_NONE && nnode < NET_NumNodes);
    }

    GL_UseJoysticks = 0;
    GL_JoyAX = 0; GL_JoyAY = 0; GL_JoyABut = 0; GL_JoyALastBut = 0;
    GL_JoyBX = 0; GL_JoyBY = 0; GL_JoyBBut = 0; GL_JoyBLastBut = 0;
    if (ConsolePlayer1 != NULL) {
        if (ConsolePlayer1->cfg.source == UCTS_JOYA)
            GL_UseJoysticks |= 1;
        else if (ConsolePlayer1->cfg.source == UCTS_JOYB)
            GL_UseJoysticks |= 2;
    }
    if (ConsolePlayer2 != NULL) {
        if (ConsolePlayer2->cfg.source == UCTS_JOYA)
            GL_UseJoysticks |= 1;
        else if (ConsolePlayer2->cfg.source == UCTS_JOYB)
            GL_UseJoysticks |= 2;
    }

        // Create things for race position.
    if (mode < 2) {
        for (i = 0; i < SIZEARRAY(RacePos); i++)
            RacePos[i] = THN_AddThing(0,0, 0x800000, THNT_RACEPOS + i, 0, 0);
    } else {
        for (i = 0; i < SIZEARRAY(RacePos); i++)
            RacePos[i] = NULL;
    }

        // Create things for human position.
    for (i = 0; i < NHumanPlayers; i++)
        HumanPos[i] = THN_AddThing(0,0, 0x800000, THNT_HUMANPOS + i, 0, 0);

        // Fill up the rest of car start points with robots.
    if (mode == 2)
        NCars = 0;
    else
        NCars = Map.nStartPos - NHumanPlayers;
    for (i = 0; i < NCars; i++) {
        int npos;
        npos = MAP_UseStartPos(&Map);
        if (npos < 0)
            break;
        assert(Map.startPos != NULL);
        p = THN_AddThing(Map.startPos[npos].x,
                         Map.startPos[npos].y,
                         0x40000,
                         THNT_CAR | ((RND_GetNum() % 6) << 4),
                         Map.startPos[npos].angle, 0);
        if (p == NULL)
            BASE_Abort("Out of memory for computer racer %d", i+1);
        cars[i] = p->data;
        cars[i]->carSpeed = (Map.nStartPos-npos);
        cars[i]->v = 0;
        cars[i]->state = 0;
    }
    NCars = i;

    Map.nracers = RCS_NRacers;
    if (!( (Map.racers = NEW(Map.nracers*sizeof(*Map.racers))) != NULL || Map.nracers <= 0))
        BASE_Abort("Out of memory for racer info");
    for (i = 0; i < Map.nracers; i++)
        Map.racers[i] = RCS_Racers + i;

        // Load additional game sprites: sparks, explosions, etc.
    for (i = 0; i < SIZEARRAY(Smoke); i++) {
        char buf[20];

        if (i < SIZEARRAY(Sparks)) {
            sprintf(buf, "sprk%02daa", i+1);
            if (!((Sparks[i] = FS3_New(buf)) != NULL))
                BASE_Abort("Reading sprite %s", buf);
        }
        sprintf(buf, "gnd%1d%02daa", i/6, (i%6)+1);
        if (!((Smoke[i] = FS3_New(buf)) != NULL))
            BASE_Abort("Reading sprite %s", buf);
    }

//    LogMemory("InitData: Antes de los efectos");

    GAME_EFF_UnlockChannel(SH_Vtal, 0);
    GAME_EFF_UnlockChannel(SH_Vtal, 1);
    GAME_EFF_UnlockChannel(SH_Vtal, 2);
    GAME_EFF_UnlockChannel(SH_Vtal, 3);
    GAME_EFF_StopAll(SH_Vtal);
    GAME_EFF_UnloadAll(SH_Vtal);
    sprintf(buf, "motor%d.raw", GL_CarType);
    SH_Eff   = GL_LoadEffect(buf, 11025, TRUE);
    RACE_EffCrash   = GL_LoadEffect("crash.raw", 13000, FALSE);
    RACE_EffDerrape = GL_LoadEffect("derrape.raw", 11000, TRUE);

    if (GL_TimedRace) {
        RACE_EffExtended = GL_LoadEffect("exttime.raw", 13000, FALSE);
        RACE_EffTimeout  = GL_LoadEffect("timeover.raw", 13000, FALSE);
    } else {
        RACE_EffLapRecord = GL_LoadEffect("laprec.raw", 13000, FALSE);
    }
    if (mode != 2)
        RACE_EffYouWin   = GL_LoadEffect("unbeli.raw", 13000, FALSE);
    else
        RACE_EffYouWin   = NULL;
    RACE_EffGameOver = GL_LoadEffect("gameover.raw", 13000, FALSE);

    TSTangle = 0;
    LLK_LastScan = 0;
    ESCPressed = FALSE;
    Paused = RaceStopped = FALSE;

    CommTrace = DevKeys && (BASE_CheckArg("commtrace") > 0);

    InitScreenParms();

    ViewHud[0] = ViewHud[1] = TRUE;

    return TRUE;
}

PRIVATE void EndData(void) {
    int i;
    THN_PThing p, g;

    GAME_EFF_UnlockChannel(SH_Vtal, 0);
    GAME_EFF_UnlockChannel(SH_Vtal, 1);
    GAME_EFF_UnlockChannel(SH_Vtal, 2);
    GAME_EFF_UnlockChannel(SH_Vtal, 3);
    GAME_EFF_StopAll(SH_Vtal);
    GAME_EFF_UnloadAll(SH_Vtal);
    SH_Eff = NULL;
    RACE_EffCrash = NULL;
    RACE_EffDerrape = NULL;
    RACE_EffFinalLap = NULL;
    RACE_EffLapRecord= NULL;
    RACE_EffTimeout  = NULL;
    RACE_EffYouWin   = NULL;
    RACE_EffExtended = NULL;
    RACE_EffGameOver = NULL;

//    LogMemory("EndData: Tras los efectos");

    p = THN_ThingList;
    while (p != NULL) {
        g = p->global.next;
        THN_DeleteThing(p);
        p = g;
    }
    p = THN_ThinkerList;
    while (p != NULL) {
        g = p->global.next;
        THN_DeleteThing(p);
        p = g;
    }
        // Check.
    for (i = 0; i < 64*64; i++)
        if (THN_ThingMap[i] != NULL)
            BASE_Abort("Thing map should be empty!");
    MAP_EndStaticCameras(&Map);
    MAP_EndStartPos(&Map);

    HUD_End();
    PATH_Done(&Map.path);
    FS3_End();
    DISPOSE(Map.racers);
    RCS_End();
    MAP_Free(&Map);
    SEC_EndMap(&Map.sec);

    POLY_MinX = 0; POLY_MaxX = 320;
    POLY_MinY = 0; POLY_MaxY = 200;
    DRW_SetClipZone(0, 0, 320, 200, NULL);
}

PRIVATE void InitView(GL_PViewInfo v, THN_PThing thn) {
    assert(thn != NULL);
    v->thn    = thn;
    v->c.data = F3D_StdCams[STDCAM_HIGH];
    v->CamTo  = F3D_StdCams[STDCAM_NORMAL];
    v->CamChangePos = 0;
    v->c.tilt  = 0;
    v->c.angle = thn->angle;
    v->c.dx    = thn->x;
    v->c.dy    = thn->y;
    v->c.sec   = thn->sec;
}


PRIVATE void HandleView(GL_PViewInfo v, int h) {

    if ( (v->thn->type & 0xFF00) == THNT_PLAYER &&
        ((PLY_PPlayer)v->thn->data)->status != PLYST_RACING) {
        v->CamChangePos = 0;
        v->CamTo = F3D_StdCams[STDCAM_STATIC];
    }

    if (v->CamChangePos >= 0) {
        sint32  sin2;

        if ((((v->c.data.flags & F3DCF_STATIC) || (v->CamTo.flags & F3DCF_STATIC))
            && v->CamChangePos == 0)
           || v->CamChangePos > 16384) {
            v->c.data = v->CamTo;
            v->CamChangePos = -1;
            v->c.tilt = 0;
            v->c.data.focus = v->c.data.focus*GL_ScreenXRatio;
            v->c.data.horizon = v->c.data.horizon*GL_ScreenYRatio;
        } else if (v->CamChangePos == 0) {
            v->CamFrom = v->c.data;
            v->CamFrom.focus = v->CamFrom.focus/GL_ScreenXRatio;
            v->CamFrom.horizon = v->CamFrom.horizon/GL_ScreenYRatio;
            v->CamChangePos++;
        } else {
            sin2 = FPMult(Sin(v->CamChangePos),Sin(v->CamChangePos));
            v->c.data.horizon = v->CamFrom.horizon + FPMult(sin2, (v->CamTo.horizon - v->CamFrom.horizon));
            v->c.data.h       = v->CamFrom.h       + FPMult(sin2, (v->CamTo.h       - v->CamFrom.h));
            v->c.data.focus   = v->CamFrom.focus   + FPMult(sin2, (v->CamTo.focus   - v->CamFrom.focus ));
            v->c.data.radius  = v->CamFrom.radius  + FPMult(sin2, (v->CamTo.radius  - v->CamFrom.radius ));
            v->CamChangePos += 111;
            v->c.data.focus = v->c.data.focus*GL_ScreenXRatio;
            v->c.data.horizon = v->c.data.horizon*GL_ScreenYRatio;
        }
    }
    if (DevKeys) {
        if (LLK_Keys[kINS])
            v->c.data.focus += 32;
        if (LLK_Keys[kDEL])
            v->c.data.focus -= 32;

        if (LLK_Keys[kHOME])
            if (v->c.data.horizon > 1)
                v->c.data.horizon--;
        if (LLK_Keys[kEND])
            v->c.data.horizon++;

        if (LLK_Keys[kPGUP])
            v->c.data.h += 32;
        if (LLK_Keys[kPGDN])
            v->c.data.h -= 32;

        if (LLK_Keys[kQ])
            v->c.data.radius += 1 << 20;
        if (LLK_Keys[kA])
            v->c.data.radius -= 1 << 20;

        if (LLK_Keys[kKEYPADMINUS])
            c.data.focus += 11 << 16;
        if (LLK_Keys[kKEYPADPLUS])
            c.data.focus -= 11 << 16;

        if (LLK_Keys[kKEYPAD9])
            v->c.tilt--;
        if (LLK_Keys[kKEYPAD3])
            v->c.tilt++;

        if (LLK_Keys[kCOMMA])
            TSTangle += 223;
        if (LLK_Keys[kPERIOD])
            TSTangle -= 223;
    }

    if (v->c.data.flags & F3DCF_STATIC) {
        MAP_PStaticCamera pc;
        pc = MAP_FindStaticCamera(&Map, v->thn->x, v->thn->y, NULL);
        if (pc != NULL) {
            sint32 dx, dy, r;
            dx = (v->thn->x >> 12) - (pc->x >> 12);
            dy = (v->thn->y >> 12) - (pc->y >> 12);
            v->c.angle = GetAngle(dx, -dy);
            v->c.dx = pc->x;
            v->c.dy = pc->y;
            v->c.data.h = pc->z;
            v->c.data.horizon = pc->z/2000 + 1;
            r = SQR_Sqrt(Pow2(dx>>8) + Pow2(dy>>8));
            v->c.data.focus = 0xA00 + FPMultDiv(0x1000, r, 64*4);
            if (r > 32) {
                sint32 y;
                y = FPMultDiv((v->c.data.h << 2) ,// - (v->thn->z >> 10),
                              v->c.data.focus, r << 14)
                   + v->c.data.horizon;
                if (y > h/3)
                    v->c.tilt = y-h/3;
                else
                    v->c.tilt = 0;
            } else
                ; // Too near: leave tilt as it is.
            v->c.data.horizon = v->c.data.horizon*GL_ScreenYRatio;
            v->c.data.focus = v->c.data.focus*GL_ScreenXRatio;
        }
    } else {
        word angle;

        if ((v->thn->type & 0xFF00) == THNT_PLAYER) {
            PLY_PPlayer p;
            p = v->thn->data;
            angle = p->movangle; //p->ma;
        } else
            angle = v->thn->angle;

        if (v->c.data.radius == 0)
            v->c.angle = v->thn->angle;
        else if (((v->c.angle - angle) / 16) == 0)
            v->c.angle = angle;
        else {
            sint32 k;
            k = (sint16)(angle - v->c.angle) / 16;
/*
            k = (sint16)(angle - v->c.angle) / 16;
            if (Abs32(k) > 0x800/16)
                k = Sgn(k)*0x800/16;
*/
            v->c.angle += k;
        }
        v->c.dx = v->thn->x;
        v->c.dy = v->thn->y;
//        v->c.tilt = 0;
        if (v->c.data.radius == 0 && v->CamChangePos <= 0)
            v->c.data.h = v->CamTo.h + (((sint32)(v->thn->z - 0x20000)) >> 12);
    }

    LLK_BIOSFlush();

    if (v->c.data.horizon < 1)
        v->c.data.horizon = 1;
}

PRIVATE dword CommsClock;

//PRIVATE UCT_TUserControl NetCheckData[NET_NTIMES];
//PRIVATE dword NetCRCData[NET_NTIMES];
//PRIVATE dword NetCRC = 0;
PRIVATE bool  UpPressed = FALSE;

PUBLIC void RACE_HandleComms(bool ints) {
        // Handle comms stuff.
/*
    if ((ints && NET_State == NETS_SERIAL)
     || (!ints && NET_State == NETS_IPX)) {
*/
    if (!ints) {
        int nlink, i, j, mintime;

        if (CommTrace) VGA_SetBorder(63,0,0);

        //----  Retrieve information from all nodes.

            // Find minimum time that may be pending to be sent.
        mintime = NET_NodeData[0].sendtime;
        for (j = 1; j < NET_NumNodes; j++)
            if (mintime > NET_NodeData[j].sendtime)
                mintime = NET_NodeData[j].sendtime;

            // First, from ourselves.
            // Only if there is room for the info. We can not afford to lose
            // info from any node, including ourselves.
        if ((NET_NodeData[0].rectime+NET_NCTL) <= (NET_NodeData[0].p[0]->clock)
         && (NET_NodeData[0].rectime+NET_NCTL) <= (mintime+NET_NTIMES)) {
                // Fetch data from console player.
            if (ConsolePlayer1 != NULL) {
                for (i = 0; i < NET_NCTL; i++) {
/*
if (NET_NumNodes == 2) if (NET_NodeData[0].ctlnum[(NET_NodeData[0].rectime+i)%NET_NTIMES] != 0)
    BASE_Abort("Overwriting packet %d, contains %d, GlobalClock = %d, rectime = %d, pclock = %d",
                NET_NodeData[0].rectime+i,
                NET_NodeData[0].ctlnum[(NET_NodeData[0].rectime+i)%NET_NTIMES],
                GlobalClock,
                NET_NodeData[0].rectime,
                NET_NodeData[0].p[0]->clock
                );
*/
                    NET_NodeData[0].ctl[(NET_NodeData[0].rectime+i)%NET_NTIMES][0]
                        = ConsolePlayer1->UserControl[(NET_NodeData[0].rectime+i)%MAX_USERCONTROLS];
/*
                    NetCheckData[(NET_NodeData[0].rectime+i)%NET_NTIMES]
                        = ConsolePlayer1->UserControl[(NET_NodeData[0].rectime+i)%MAX_USERCONTROLS];
*/
//                    NetCRCData[(NET_NodeData[0].rectime+i)%NET_NTIMES]

/*
if (NET_NumNodes == 2) NET_NodeData[0].ctlnum[(NET_NodeData[0].rectime+i)%NET_NTIMES]
    = NET_NodeData[0].rectime+i;
*/
                }
            }
                // Mark as received.
            NET_NodeData[0].rectime += NET_NCTL;
                // The info is sent to ourselves and automatically confirmed.
            NET_NodeData[0].sendtime += NET_NCTL;
        }

            // Receive incoming packets.
        for(;;) {
            byte lc, pc;
            NET_TPacket InP;
            int newst;

            if (CommTrace) VGA_SetBorder(0,63,0);
            InP.len = sizeof(NET_TGamePacket);
            nlink = COM_GetPacket(&InP.p);
            if (nlink == -1)
                break;
            if (nlink == -2) {
                continue;
            }

//if (!ints) {printf("packet: %d =?= %d\r", InP.thisclock, NET_NodeData[nlink].rectime); fflush(stdout);}

                // Ignore broadcast packets.
            if (nlink <= 0 || nlink > COM_MaxLinks || COM_Links[nlink-1].type != COM_Type)
                continue;

                 // Appropiate size
            if (!(InP.command == NETC_INFO && InP.len == sizeof(NET_TGamePacket))
             && !(InP.command == NETC_RESEND && InP.len == sizeof(NET_TResendPacket))) {
                continue;
            }

            if (InP.command == NETC_RESEND) {
//if (!ints) {printf("resend: %d\r", InP.resendfrom); fflush(stdout);}
                NET_NodeData[nlink].ringtime = InP.resendfrom;
                continue;
            }
                // INFO command

            pc = InP.thisclock;
            lc = NET_NodeData[nlink].rectime & 0xFF;
                // Update this node's wanted packet number.
            newst = NET_NodeData[nlink].sendtime;
            if ((newst & 0xFF) > (256-NET_NTIMES)
             && InP.sendclock < NET_NTIMES)
                newst += 256;
            newst = (newst & ~0xFF) + InP.sendclock;

/*
if (NET_NumNodes == 2) for (i = NET_NodeData[nlink].sendtime; i < newst; i++) {
    if (NET_NodeData[0].ctlnum[i%NET_NTIMES] != i)
        BASE_Abort("Packet %d has been overwritten", i);
    NET_NodeData[0].ctlnum[i%NET_NTIMES] = 0;
}
*/
            NET_NodeData[nlink].sendtime = newst;

                // Correct time.
            if (lc != pc) {
                dword packettime;

                    // Estimate packet time.
                packettime = NET_NodeData[nlink].rectime;
                if ((packettime & 0xFF) > (256-NET_NTIMES)
                 && pc < NET_NTIMES)
                    packettime += 256;
                packettime = (packettime & ~0xFF) + pc;

//                if (packettime != InP.thisFullClock)
//                    BASE_Abort("Times differ! %d != %d", packettime, InP.thisFullClock);

                    // If packet is the next to the one we expected, RESEND
                if (packettime > NET_NodeData[nlink].rectime) {
                    NET_TPacket OutP;

//if (!ints) {printf("resend %d, not %d\r", NET_NodeData[nlink].rectime, packettime); fflush(stdout);}
                    OutP.time = 0;
                    OutP.len = sizeof(NET_TResendPacket);
                    OutP.command = NETC_RESEND;
                    OutP.resendfrom = NET_NodeData[nlink].rectime;
                    if (CommTrace) VGA_SetBorder(63,0,63);
                    COM_SendPacket(nlink, &OutP.p);
                    if (CommTrace) VGA_SetBorder(63,63,0);
                } else {
//if (!ints) {printf("ignoring %d, not %d\r", packettime, NET_NodeData[nlink].rectime); fflush(stdout);}
                }
                continue;
            }

                // Won't overwrite an unprocessed packet.
            if ((NET_NodeData[nlink].rectime+NET_NCTL) > (GlobalClock+NET_NTIMES)) {
                continue;
            }

            if (CommTrace) VGA_SetBorder(63,63,63);
            // OK with this packet, retrieve info.
                // Store packet data.
            for (i = 0; i < NET_NCTL; i++) {
/*
if (NET_NumNodes == 2) if (NET_NodeData[nlink].ctlnum[(NET_NodeData[nlink].rectime+i)%NET_NTIMES] != 0)
    BASE_Abort("Overwriting packet from link %d", nlink);
*/
                NET_NodeData[nlink].ctl[(NET_NodeData[nlink].rectime+i)%NET_NTIMES][0]
                     = InP.ctl[i];
/*
if (NET_NumNodes == 2) NET_NodeData[nlink].ctlnum[(NET_NodeData[nlink].rectime+i)%NET_NTIMES]
    = NET_NodeData[nlink].rectime+i;
*/
            }
            if (NET_NodeData[nlink].p[0] != NULL) {
                PLY_PPlayer p;
                p = NET_NodeData[nlink].p[0];
                for (i = 0; i < NET_NCTL; i++) {
/*
if (NET_NumNodes == 2) if ((p->clock+i) != NET_NodeData[nlink].ctlnum[(NET_NodeData[nlink].rectime+i)%NET_NTIMES])
    BASE_Abort("clock Out of sequence: %i is %d",
               p->clock+i,
               NET_NodeData[nlink].ctlnum[(NET_NodeData[nlink].rectime+i)%NET_NTIMES]);
*/
                    p->UserControl[(p->clock+i)%MAX_USERCONTROLS] =
                        NET_NodeData[nlink].ctl[(NET_NodeData[nlink].rectime+i)%NET_NTIMES][0];
//if (NET_NumNodes == 2) NET_NodeData[nlink].ctlnum[(NET_NodeData[nlink].rectime+i)%NET_NTIMES] = 0;
                }
                p->clock += NET_NCTL;
            }
                // Update this node's expected packet number.
            NET_NodeData[nlink].rectime += NET_NCTL;
        }

        while (CommsClock < TIMER_Clock) {
            NET_TPacket OutP;

            CommsClock += NET_NCTL;
                // Send outgoing packets.
            if (CommTrace) VGA_SetBorder(0,0,63);
                // Up to N consecutive packets to allow faster resyncing.
            for (i = 0; i < NET_MaxTicks; i++) {
                for (j = 1; j < NET_NumNodes; j++) {
                        // ringtime can't go too far from sendtime.
                    if ((NET_NodeData[j].ringtime+NET_NCTL) > (NET_NodeData[j].sendtime+NET_NTIMES))
                        if (i == 0)
                            NET_NodeData[j].ringtime = NET_NodeData[j].sendtime+NET_NTIMES-NET_NCTL;
                        else
                            continue;
                        // ringtime can't go further than the already acquired input.
                    if ((NET_NodeData[j].ringtime+NET_NCTL) > NET_NodeData[0].rectime)
                        continue;
                        // ringtime can't be below the sent and confirmed input.
                    if (NET_NodeData[j].ringtime < NET_NodeData[j].sendtime)
                        NET_NodeData[j].ringtime = NET_NodeData[j].sendtime;
                        // ringtime can't go further than the already acquired input.
                    if ((NET_NodeData[j].ringtime+NET_NCTL) > NET_NodeData[0].rectime)
                        continue;

                        // Send my info at this node's time.
                    OutP.time = 100 + NET_NodeData[j].ringtime;
                    OutP.len = sizeof(NET_TGamePacket);
                    OutP.command = NETC_INFO;
                    for (i = 0; i < NET_NCTL; i++) {
                        OutP.ctl[i] = NET_NodeData[0].ctl[(NET_NodeData[j].ringtime+i)%NET_NTIMES][0];
/*
                        if (memcmp(NetCheckData+(NET_NodeData[j].ringtime+i)%NET_NTIMES,
                                   NET_NodeData[0].ctl[(NET_NodeData[j].ringtime+i)%MAX_USERCONTROLS]+0,
                                   4) != 0)
                            BASE_Abort("Trying to send different data! 0x%X != 0x%X",
                                *(dword*)NetCheckData + (NET_NodeData[j].ringtime+i)%NET_NTIMES,
                                *(dword*)NET_NodeData[0].ctl[(NET_NodeData[j].ringtime+i)%MAX_USERCONTROLS]+0
                            );
*/
                    }
                    OutP.thisclock = NET_NodeData[j].ringtime & 0xFF;
                    OutP.sendclock = NET_NodeData[j].rectime & 0xFF;
//                    OutP.thisFullClock = NET_NodeData[j].ringtime;
                    if (CommTrace) VGA_SetBorder(0,63,63);
                    COM_SendPacket(j, &OutP.p);
                    if (CommTrace) VGA_SetBorder(0,0,63);
                    NET_NodeData[j].ringtime += NET_NCTL;
                }
            }
        }
        if (CommTrace) VGA_SetBorder(0,0,0);
    }
}

    // This gets called during interrupts by the timer handler TimerUserControl.
PRIVATE void TimerUserControlFunc(void) {
    static volatile int TUCsema = 0;
    int i;
    dword minnettime = 0x3FFFFFFF, maxcontime = 0;

    if (TUCsema) return;
    TUCsema = TRUE;

    //VGA_SetBorder(63, 0, 0);

        // GlobalClock is the first element not yet interpreted.
        // player->clock is the place where to leave new input
        // Therefore, the >= means the new input would overwrite
        // input not yet interpreted (both have the same % MAX_USERC..).

        // We can fetch new data if and only if the place where we
        // would leave it is not needed anymore:
        //  - it has been processed:
        //      clock < GlobalClock+MAX_USERCONTROLS
        //  - all nodes have received it:
        //      sendtime >

        // If console player has exhausted its buffer completely, or
        // network players are way out of time, don't get input.
    for (i = 0; i < NHumanPlayers; i++)
        if (HumanPlayers[i] != NULL)
            if (HumanPlayers[i]->flags & PLYF_ISCONSOLE) {
                if (HumanPlayers[i]->clock >= (GlobalClock+ MAX_USERCONTROLS))
                    break;
                if (maxcontime < HumanPlayers[i]->clock)
                    maxcontime = HumanPlayers[i]->clock;
            } else {
                if (minnettime > HumanPlayers[i]->clock)
                    minnettime = HumanPlayers[i]->clock;
            }

    if (i >= NHumanPlayers && ConsoleClock < (GlobalClock+MAX_USERCONTROLS)
        && (NET_State == NETS_NONE
         || ((NET_NodeData[0].rectime+MAX_USERCONTROLS) > NET_NodeData[0].p[0]->clock
//          && (minnettime+10) > mincontime))
//          && (minnettime+10) > mincontime))
          && minnettime+10 > maxcontime))
        ) {
        dword k;

            // Get input from console players.
        if (ConsolePlayer1 != NULL) {
            k = ConsolePlayer1->clock % MAX_USERCONTROLS;
            UCT_GetUserControl(ConsolePlayer1->UserControl + k,
                               k, &ConsolePlayer1->cfg);
            ConsolePlayer1->clock++;
        }
        if (ConsolePlayer2 != NULL) {
            k = ConsolePlayer2->clock % MAX_USERCONTROLS;
            UCT_GetUserControl(ConsolePlayer2->UserControl + k,
                               k, &ConsolePlayer2->cfg);
            ConsolePlayer2->clock++;
        }
        k = ConsoleClock % MAX_USERCONTROLS;
        ConsoleInput[k] = LLK_LastScan;
        ConsoleClock++;
        LLK_LastScan = 0;
        GL_JoyABut = 0;
        GL_JoyBBut = 0;

    }

    //RACE_HandleComms(TRUE);

    GL_PollMusic();
    TUCsema = FALSE;
    //VGA_SetBorder(0, 0, 0);
}

PRIVATE void TimerUserControl(void) {
    XTRN_StackExec(&SH_Stack, TimerUserControlFunc);
}


/*
PRIVATE void DmpTrans(byte *dest, const byte *org, int nb) {
    while (nb-- > 0) {
        if (*org != 0)
            *dest = *org;
        dest++;
        org++;
    }
}
*/
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

extern void Dmp640(byte *dest, const byte *org, int nb);
#pragma aux Dmp640 modify [EAX] parm [EDI] [ESI] [ECX] = \
    "drl:             "  \
    "    MOV AL,[ESI] "  \
    "    INC ESI      "  \
    "    MOV AH,AL    "  \
    "    MOV [EDI],AX "  \
    "    ADD EDI,2    "  \
    "    DEC ECX      "  \
    "    JNZ drl      "

PRIVATE void RenderView(F3D_PCamera cam, int y, int h) {
    int i;
    int hsk, hfl;
    int camh;

    ExtraDetail = (cam->data.flags & F3DCF_STATIC) != 0;

    camh = cam->data.horizon;
    if (cam->tilt < 0)
        cam->tilt = 0;
    if (cam->tilt > Map.hbackg) {
        cam->data.horizon += cam->tilt - Map.hbackg;
        hsk = 0;
    } else
        hsk = Map.hbackg - cam->tilt;

    cam->angle += TSTangle;
    cam->x = cam->dx - FPMult(cam->data.radius, Cos(cam->angle));
    cam->y = cam->dy - FPMult(cam->data.radius, Sin(cam->angle));
    cam->data.hiDetail = (Detail & 1) != 0;

    {
        dword o, d;
        byte *p;
        int hmnt;
        int cy;

        cy = y*LLS_SizeX;

        p = Map.backg + 320*cam->tilt;
        o = (3*cam->angle*320/65536)%320;
        d = LLS_SizeX-o;
        for (i = 0; i < h && i < hsk; i++) {
            int to, td;
            to = o; td = d;

            if (to > 0)
/*
                if (GL_ScreenXRatio > 1)
                    Dump640(LLS_Screen[0]+cy, p+td, to);
                else
*/
                    RepMovsb(LLS_Screen[0]+cy, p+td%320, to);
            while (td > 0) {
                if (td > 320)
/*
                    if (GL_ScreenXRatio > 1)
                        Dump640(LLS_Screen[0]+cy+to, p, 320);
                    else
*/
                        RepMovsb(LLS_Screen[0]+cy+to, p, 320);
                else
/*
                    if (GL_ScreenXRatio > 1)
                        Dump640(LLS_Screen[0]+cy+to, p, td);
                    else
*/
                        RepMovsb(LLS_Screen[0]+cy+to, p, td);
//                to += GL_ScreenXRatio*320;
                to += 320;
                td -= 320;
            }
            if (GL_ScreenYRatio > 1) {
                cy += LLS_SizeX;
            }
            p += 320;
            cy += LLS_SizeX;
        }

        /*if (BackgroundDetail) */{
            if (cam->tilt > (Map.hbackg - Map.hbackg2)) {
                p = Map.backg2 + 320*(cam->tilt - (Map.hbackg - Map.hbackg2));
                i = 0;
                hmnt = Map.hbackg - cam->tilt;
            } else {
                i = (Map.hbackg - Map.hbackg2) - cam->tilt;
                p = Map.backg2;
                hmnt = i + Map.hbackg2;
            }

            cy = (i*GL_ScreenYRatio+y)*LLS_SizeX;
            o = (4*cam->angle*320/65536)%320;
            d = LLS_SizeX-o;
            for (; i < h && i < hmnt; i++) {
                int to, td;
                to = o; td = d;
                if (to > 0)
                    DmpTrans(LLS_Screen[0]+cy, p+td%320, to);
                while (td > 0) {
                    if (td > 320)
                        DmpTrans(LLS_Screen[0]+cy+to, p, 320);
                    else
                        DmpTrans(LLS_Screen[0]+cy+to, p, td);
                    to += 320;
                    td -= 320;
                }
                if (GL_ScreenYRatio > 1) {
                    cy += LLS_SizeX;
                }
                p += 320;
                cy += LLS_SizeX;
            }
        }

        if (GL_ScreenYRatio > 1) {
            cy = y*LLS_SizeX;
            for (i = 0; i < hsk; i++) {
                RepMovsb(LLS_Screen[0]+cy+LLS_SizeX, LLS_Screen[0]+cy, LLS_SizeX);
                cy += 2*LLS_SizeX;
            }
        }
    }
    if (Trace) VGA_PutColor(0, 0, 63, 0);

        // Render floor.
    hfl = h - GL_ScreenYRatio*hsk;
    if (hfl > 0)
        F3D_Draw3D(LLS_Screen[0]+LLS_SizeX*(GL_ScreenYRatio*hsk+y)+GL_ScreenMinX,
                   Map.map, GL_ScreenW, hfl, cam, Map.trans);
    if (Trace) VGA_PutColor(0, 0, 0, 63);

        // Render objects.

//    if (BASE_CheckArg("nocalc") <= 0 && (Detail & 2))
    {
        SEC_Render(&Map.sec, cam->sec, GL_ScreenCenterX, GL_ScreenYRatio*hsk+y, cam);
    }
/*
    else {
        FSP_ClearObjs(160, hsk+y, cam);
        if (ConsolePlayer1 != NULL)
            FSP_AddObj(ConsolePlayer1->thn->x, ConsolePlayer1->thn->y,
                       ConsolePlayer1->thn->z, ConsolePlayer1->thn->angle,
                       0, ConsolePlayer1->thn->spr);
        if (ConsolePlayer2 != NULL)
            FSP_AddObj(ConsolePlayer2->thn->x, ConsolePlayer2->thn->y,
                       ConsolePlayer2->thn->z, ConsolePlayer2->thn->angle,
                       0, ConsolePlayer2->thn->spr);
        if (Trace) VGA_PutColor(0, 63, 0, 63);
        FSP_DumpObjs(cam, Map.trans, 160, hsk+y);
    }
*/
    cam->angle -= TSTangle;
    cam->data.horizon = camh;
}

PRIVATE void DrawMap(PLY_PPlayer p, int x, int y, int w, int h, sint32 focus, int mode) {
    int i;

    c.data.focus = focus;
    c.x = p->thn->x;
    c.y = p->thn->y;
    c.angle = p->thn->angle;
    F3D_Draw2D(LLS_Screen[0] + y*LLS_SizeX + x, Map.map, w, h, &c, Map.trans);
    for (i = Map.nracers-1; i >= 0; i--) {
        sint32 ca, sa;
        ca = FPMult(Cos(16384-c.angle), (1 << 30) / (c.data.focus >> 8));
        sa = FPMult(Sin(16384-c.angle), (1 << 30) / (c.data.focus >> 8));
        /*if (RacePos[i] != NULL) */{
            sint32 rx, ry, px, py;
            rx = (Map.racers[i]->thn->x >> 8) - (c.x >> 8);
            ry = (Map.racers[i]->thn->y >> 8) - (c.y >> 8);
            px = w/2 + FPMult(rx, ca) - FPMult(ry, sa);
            py = h/2 + FPMult(ry, ca) + FPMult(rx, sa);
            if (px >= 0 && px < w && py >= 0 && py < h) {
                int nsp;
                if (i > 4 || mode == 2) {
                    if ((Map.racers[i]->thn->type & 0xFF00) == THNT_PLAYER)
                        nsp = 7;
                    else
                        nsp = 6;
                } else
                nsp = i;
                IS2_DrawHorizontal(PlayerPosDots[nsp], x + px, y + py);
            }
        }
    }
    IS2_DrawHorizontal(PlayerPosDots[5], x + w/2, y + h/2);
}

PRIVATE void ConsoleRoutine(dword clock) {
    int k, i;
    bool is;

    k = clock % MAX_USERCONTROLS;

    is = ConsoleInput[k] == kPAUSE;
    for (i = 0; !is && i < NHumanPlayers; i++)
        if (HumanPlayers[i] != NULL)
            is = HumanPlayers[i]->UserControl[k].scancode == kPAUSE;
    if (is) {
        if (Paused) {
            GAME_EFF_ChangeVolume(SH_Vtal, CFG_Config.FXVolume);
        } else {
            GAME_EFF_ChangeVolume(SH_Vtal, 0);
        }
        Paused = !Paused;
    }

    is = ConsoleInput[k] != 0 && ConsoleInput[k] != kESC;
    for (i = 0; !is && i < NHumanPlayers; i++)
        if (HumanPlayers[i] != NULL)
            is = HumanPlayers[i]->UserControl[k].scancode != 0
              && HumanPlayers[i]->UserControl[k].scancode != kESC;
    if (Quitting == 1 && is) {
        Quitting = 0;
    }

    is = ConsoleInput[k] == kESC;
    for (i = 0; !is && i < NHumanPlayers; i++)
        if (HumanPlayers[i] != NULL)
            is = HumanPlayers[i]->UserControl[k].scancode == kESC;
    if (is) {
        Quitting++;
    }

/*
    if (ConsoleInput[k] == kT)
        Trace = !Trace;
    if (ConsoleInput[k] == kV)
        O3DM_MaxDetail = (O3DM_MaxDetail+1)%(O3DD_TEXGOURAUD+1);
    if (ConsoleInput[k] == kS && ConsolePlayer2 != NULL)
        SplitScreenMode = !SplitScreenMode;
    if (ConsoleInput[k] == kN)
        AdvanceTarget = 1;
    if (ConsoleInput[k] == kP)
        AdvanceTarget = -1;
    if (ConsoleInput[k] == kD)
        Detail++;
    if (ConsoleInput[k] == kC) {
        if (ConsolePlayer1 != NULL)
            ConsolePlayer1->thn->flags = ConsolePlayer1->thn->flags ^ THNF_SOLID;
        if (ConsolePlayer2 != NULL)
            ConsolePlayer2->thn->flags = ConsolePlayer2->thn->flags ^ THNF_SOLID;
    }
*/
    GL_Capture(ConsoleInput[k]);

    if (ConsoleInput[k] == kF1 || ConsoleInput[k] == kF2
     || ConsoleInput[k] == kF3 || ConsoleInput[k] == kF4) {
        if (ConsoleInput[k] == kF1)
            Views[0].CamTo   = F3D_StdCams[STDCAM_NORMAL];
        if (ConsoleInput[k] == kF2)
            Views[0].CamTo   = F3D_StdCams[STDCAM_LOW];
        if (ConsoleInput[k] == kF3)
            Views[0].CamTo   = F3D_StdCams[STDCAM_HIGH];
        if (ConsoleInput[k] == kF4)
            Views[0].CamTo   = F3D_StdCams[STDCAM_STATIC];
        Views[0].CamChangePos = 0;
    }
    if (ConsoleInput[k] == kF5 || ConsoleInput[k] == kF6
     || ConsoleInput[k] == kF7 || ConsoleInput[k] == kF8) {
        if (ConsoleInput[k] == kF5)
            Views[1].CamTo   = F3D_StdCams[STDCAM_NORMAL];
        if (ConsoleInput[k] == kF6)
            Views[1].CamTo   = F3D_StdCams[STDCAM_LOW];
        if (ConsoleInput[k] == kF7)
            Views[1].CamTo   = F3D_StdCams[STDCAM_HIGH];
        if (ConsoleInput[k] == kF8)
            Views[1].CamTo   = F3D_StdCams[STDCAM_STATIC];
        Views[1].CamChangePos = 0;
    }
    if (ConsoleInput[k] == kF9)
        ViewHud[0] = !ViewHud[0];
    if (ConsoleInput[k] == kF10)
        ViewHud[1] = !ViewHud[1];

    if (ConsoleInput[k] == kF11)
        FloorDetail = !FloorDetail;

    if (ConsoleInput[k] == kF12) {
        SVGAOn = !SVGAOn;
        if (!SVGAOn
         || (!LLS_Init(LLSM_VIRTUAL, LLSVM_640x400x256)
          && !LLS_Init(LLSM_VIRTUAL, LLSVM_640x480x256))) {
            SVGAOn = FALSE;
            LLS_Init(LLSM_VIRTUAL, LLSVM_MODE13);
        }
        InitScreenParms();
        VGA_DumpPalette(GamePal, 0, 256);
        Views[0].c.data = Views[0].CamTo;
        Views[0].CamChangePos = -1;
        Views[0].c.tilt = 0;
        Views[0].c.data.focus   = Views[0].c.data.focus*GL_ScreenXRatio;
        Views[0].c.data.horizon = Views[0].c.data.horizon*GL_ScreenYRatio;
        Views[1].c.data = Views[1].CamTo;
        Views[1].CamChangePos = -1;
        Views[1].c.tilt = 0;
        Views[1].c.data.focus   = Views[1].c.data.focus*GL_ScreenXRatio;
        Views[1].c.data.horizon = Views[1].c.data.horizon*GL_ScreenYRatio;
    }
}

PRIVATE void UpdateJoystick(void) {
    uint ab = 0, bb = 0;
    switch (GL_UseJoysticks) {
        case 0:
            break;
        case 1:
            JOY_CalGet(16, 16, &GL_JoyAX, &GL_JoyAY, NULL, NULL, &ab, NULL); break;
        case 2:
            JOY_CalGet(16, 16, NULL, NULL, &GL_JoyBX, &GL_JoyBY, NULL, &bb); break;
        case 3:
            JOY_CalGet(16, 16, &GL_JoyAX, &GL_JoyAY, &GL_JoyBX, &GL_JoyBY, &ab, &bb); break;
    }
    GL_JoyABut |= ab & (ab ^ GL_JoyALastBut);
    GL_JoyBBut |= bb & (bb ^ GL_JoyBLastBut);
    GL_JoyALastBut = ab;
    GL_JoyBLastBut = bb;
}


// =============================

PRIVATE void RemoveTargetPos(GL_PViewInfo v, FS3_PSprite cspr[2], THN_PThing cthn[2]) {
    int i;

    cthn[0] = NULL;
    cthn[1] = NULL;
    for (i = 0; (i < SIZEARRAY(RacePos) || i < NHumanPlayers)
                 && i < Map.nracers; i++) {
        if (Map.racers[i]->thn == v->thn) {
            if (i < SIZEARRAY(RacePos)) {
                cspr[0] = RacePos[i]->spr;
                RacePos[i]->spr = NULL;
                cthn[0] = RacePos[i];
            }
        }
        if (i < NHumanPlayers) {
            if (HumanPlayers[i]->thn == v->thn) {
                cspr[1] = HumanPos[i]->spr;
                HumanPos[i]->spr = NULL;
                cthn[1] = HumanPos[i];
            }
        }
    }
}

PRIVATE void RestoreTargetPos(FS3_PSprite cspr[2], THN_PThing cthn[2]) {
    if (cthn[0] != NULL)
        cthn[0]->spr = cspr[0];
    if (cthn[1] != NULL)
        cthn[1]->spr = cspr[1];
}

// =============================

            extern int FL_StatNPolys,
                       FL_StatNSprs,
                       FL_StatNObjs,
                       FL_StatNFrontObjs,
                       FL_StatNVecs;

PUBLIC int RACE_DoRace(int mode, RACE_TResult result[NET_MAXNODES]) {
    dword startDelay, nFrames;
    int  i, j, k;
    sint efftime, lefftime;
    IS2_PSprite countdown[6];
    IS2_PSprite gameover;
    GAME_PEffect countsmp[4];
    int countsmpch = -1, countsmpsec = -1;
    int countgover = -1;
    bool isderrape = FALSE;
    dword LastFrameTimer;

//    memset(NetCheckData, 0, sizeof(NetCheckData));
//    memset(NetCRCData, 0, sizeof(NetCRCData));
//    NetCRC = 0;

    for (i = 0; i < SIZEARRAY(NET_NodeData); i++) {
        NET_NodeData[i].sendtime = 0;
        NET_NodeData[i].rectime  = 0;
        NET_NodeData[i].ringtime = 0;
    }

//    LogMemory("RACE_DoRace");

    if (O3DM_MaxDetail > O3DD_TEXGOURAUD)
        O3DM_MaxDetail = O3DD_TEXGOURAUD;
//    TEXT_Write(&FONT_Border, 140, 170, "LOADING", 14);

    RND_Randomize(GL_Seed);

//    LogMemory("RACE_DoRace: Probando InitData");
     if (!InitData(mode, GL_ConsolePlayers))
        BASE_Abort("Out of memory initializing race");
/*
    LogMemory("RACE_DoRace: Entre Probando InitData");
    EndData();
    LogMemory("RACE_DoRace: Fin de Probando InitData");
    if (!InitData(mode, GL_ConsolePlayers))
        return;
*/
//    LogMemory("RACE_DoRace: Tras InitData");

    for (i = 0; i < SIZEARRAY(countdown); i++) {
        static const char *smp[] = {
            "go.raw",
            "one.raw",
            "two.raw",
            "three.raw"
        };
        char buf[200];
        sprintf(buf, "race_%d.is2", i);
        countdown[i] = IS2_Load(buf);
        if (i < SIZEARRAY(countsmp)) {
            if (i < SIZEARRAY(smp)) {
                countsmp[i] = GL_LoadEffect(smp[i], 13000, FALSE);
            } else {
                countsmp[i] = NULL;
            }
        }
    }
    for (i = 0; i < SIZEARRAY(PlayerPosDots); i++) {
        char buf[200];
        sprintf(buf, "carpos%d.is2", i);
        PlayerPosDots[i] = IS2_Load(buf);
    }
    for (i = 0; i < SIZEARRAY(HumanSprs); i++) {
        char buf[200];
        sprintf(buf, "humansp%d.is2", i+1);
        HumanSprs[i] = IS2_Load(buf);
    }

    gameover = IS2_Load("gameover.is2");

#define MOTORMIN 100
#define MOTORRAT 700


    if (SH_Vtal != NULL) lefftime = SH_Vtal->EffTickCount;

    if (ConsolePlayer2 == NULL)
        SplitScreenMode = FALSE;
    else
        SplitScreenMode = TRUE;

        // Adjust changed parameters.
    if (SplitScreenMode)
        F3D_StdCams = F3D_StdCams2Player;
    else
        F3D_StdCams = F3D_StdCams1Player;

    InitView(Views + 0, pplayer->thn);
    Views[0].c.data = F3D_StdCams[STDCAM_NORMAL];
    if (mode < 2)
        Views[0].c.data.radius = 32 << 23;

    if (ConsolePlayer2 != NULL) {
        InitView(Views + 1, ConsolePlayer2->thn);
        Views[1].c.data = F3D_StdCams[STDCAM_NORMAL];
        if (mode < 2)
            Views[1].c.data.radius = 32 << 23;
    }

//    LogMemory("RACE_DoRace: Antes del bucle");

    GameClock = 0;
    RaceStarted = FALSE;
    Paused = RaceStopped = FALSE;
    if (mode == 2)
        startDelay = 0;
    else
        startDelay = 3*70 + 69;
    nFrames = 0;
    HandleView(Views+0, 200);
    if (ConsolePlayer2 != NULL)
        HandleView(Views+1, 200);
    GlobalClock = 0;
    for (i = 0; i < NHumanPlayers; i++)
        if (HumanPlayers[i] != NULL)
            HumanPlayers[i]->clock = 0;
    ConsoleClock = 0;

    memset(LLS_Screen[0], 0, LLS_Size);
    LLS_Update();
    VGA_DumpPalette(GamePal, 0, 256);

    if (NET_State != NETS_NONE && NET_SyncStart() < 0)
        goto endrace;
    GAME_EFF_Start(SH_Vtal, 0, MOTORMIN,  ((GL_CarType==1)*2+1)*64, 0, SH_Eff);
    if (ConsolePlayer2 == NULL && NCars > 0) {
        GAME_EFF_Start(SH_Vtal, 1, MOTORMIN,   0, 0, SH_Eff);
        GAME_EFF_Start(SH_Vtal, 2, MOTORMIN,   0, 0, SH_Eff);
    } else if (ConsolePlayer2 != NULL)
        GAME_EFF_Start(SH_Vtal, 1, MOTORMIN,  ((GL_CarType==1)+1)*64, 0, SH_Eff);

    CommsClock = TIMER_Clock;
    LastFrameTimer = TIMER_Clock;
    TIMER_HookFunction = &TimerUserControl;

    do {
        int i, s, nsims;

        if (Trace) VGA_VSync();
        if (Trace) VGA_PutColor(0, 63, 0, 0);

        nsims = 0;
            // Run simulation of world.
        while (/*!LLK_Keys[kBACKSPACE] && */TRUE) {
                // Check if we have input from all human players.
                // For that, all clocks must be greater than the
                // simulation clock, GlobalClock.
            for (i = 0; i < NHumanPlayers; i++)
                if (HumanPlayers[i] != NULL)
                    if (HumanPlayers[i]->clock <= GlobalClock)
                        break;
            if (i < NHumanPlayers)
                break;
                // Run controllers and things.
            if ((Detail & 2) && !Paused) {
/*
                if (LLK_Keys[kRIGHTSHIFT]) {
                        // Just run human players.
                    for (i = 0; i < NHumanPlayers; i++)
                        if (HumanPlayers[i] != NULL)
                            HumanPlayers[i]->thn->routine(HumanPlayers[i]->thn->data);
                } else  // Run everything.
*/
                    THN_RunThings();
            } else {
                    // Just run human players.
                for (i = 0; i < NHumanPlayers; i++)
                    if (HumanPlayers[i] != NULL)
                        HumanPlayers[i]->thn->routine(HumanPlayers[i]->thn->data);
            }
                // Run console
            ConsoleRoutine(GlobalClock);

                // World simulated, increase world time.
            GlobalClock++;

                // Control views
            if (!SplitScreenMode) {
                HandleView(Views+0, 200);
            } else {
                HandleView(Views+0, 100);
                HandleView(Views+1, 100);
            }
                // Initial startup delay, GameClock updating.
            if (!Paused && !RaceStopped) {
                if (startDelay > 0)
                    startDelay--;
                if (startDelay < 70) {
                    RaceStarted = TRUE;
                    GameClock++;
                }
            }
                // Keep racers' position.
            if (countgover < 0) {
                bool didchg;

                do {
                    long dist, dist1;
                    didchg = FALSE;

                    i = 0;
                    REQUIRE(Map.racers[i] != NULL);
                    dist = Map.racers[i]->nlap*10000 + Map.racers[i]->npoint;
                    for (i = 1; i < Map.nracers; i++) {
                        dist1 = Map.racers[i]->nlap*10000 + Map.racers[i]->npoint;
                            // Don't move a finished racer's position.
                        if (!Map.racers[i-1]->finished && dist1 > dist) {
                            RCS_PRacer r;
                            r = Map.racers[i-1];
                            Map.racers[i-1] = Map.racers[i];
                            Map.racers[i] = r;
                            didchg = TRUE;
                        }
                        dist = dist1;
                    }
                } while (didchg);
            }

                // Check for game over
            if (countgover > 0) {
                countgover--;
            } else if (countgover < 0) {
                for (i = 0; i < NHumanPlayers; i++)
                    if (HumanPlayers[i]->status == PLYST_RACING)
                        break;
                if (i >= NHumanPlayers) {
                    GAME_EFF_Start(SH_Vtal, 4, 256, 400, 0, RACE_EffGameOver);
                    countgover = 70*3;
                }
            }
            nsims++;
        }

            // Snoop wires.
        RACE_HandleComms(FALSE);

        if (nsims <= 0)
            goto getinfo; //continue;

            // Read joysticks if appropiate.
        UpdateJoystick();

        if (ConsolePlayer2 != NULL) {
            if (AdvanceTarget != 0) {
                THN_PThing thn;
                ViewCar = ViewCar + AdvanceTarget;
                if (ViewCar < 0)
                    ViewCar = NCars-1;
                else if (ViewCar > NCars-1)
                    ViewCar = 0;
                AdvanceTarget = 0;
                thn = cars[ViewCar]->thn;
                Views[1].thn = thn;
                Views[1].c.angle = thn->angle;
                Views[1].c.dx    = thn->x;
                Views[1].c.dy    = thn->y;
            }
        }

            // Place race position things in their appropiate place.
        for (i = 0; i < SIZEARRAY(RacePos) && i < Map.nracers; i++)
            if (RacePos[i] != NULL)
                THN_MoveThing(RacePos[i],
                              Map.racers[i]->thn->x,
                              Map.racers[i]->thn->y);

            // Place human position things in their appropiate place.
        for (i = 0; i < NHumanPlayers; i++)
            if (HumanPos[i] != NULL)
                THN_MoveThing(HumanPos[i],
                              HumanPlayers[i]->thn->x,
                              HumanPlayers[i]->thn->y);

            // Adjust changed parameters.
        if (SplitScreenMode)
            F3D_StdCams = F3D_StdCams2Player;
        else
            F3D_StdCams = F3D_StdCams1Player;

            // -------- Render view.

        FL_StatNPolys = 0;
        FL_StatNSprs = 0;
        FL_StatNObjs = 0;
        FL_StatNFrontObjs = 0;
        FL_StatNVecs = 0;

            // Snoop wires.
        RACE_HandleComms(FALSE);

        if (ConsolePlayer1 != NULL && ConsolePlayer1->status != PLYST_RACING)
            ViewHud[0] = TRUE;
        if (ConsolePlayer2 != NULL && ConsolePlayer2->status != PLYST_RACING)
            ViewHud[1] = TRUE;
        if (!SplitScreenMode) {
            FS3_PSprite cspr[2];
            THN_PThing  cthn[2];

            if (!(Views[0].c.data.flags & F3DCF_STATIC))
                RemoveTargetPos(&Views[0], cspr, cthn);
            RenderView(&Views[0].c, GL_ScreenMinY, GL_ScreenH);
            if (!(Views[0].c.data.flags & F3DCF_STATIC))
                RestoreTargetPos(cspr, cthn);
        } else {
            FS3_PSprite cspr[2];
            THN_PThing  cthn[2];

            if (!(Views[0].c.data.flags & F3DCF_STATIC))
                RemoveTargetPos(&Views[0], cspr, cthn);
            POLY_MinY = GL_ScreenMinY; POLY_MaxY = GL_ScreenMinY+GL_ScreenH/2;
            DRW_SetClipZone(POLY_MinX, POLY_MinY, POLY_MaxX, POLY_MaxY, NULL);
            Views[0].c.tilt += 20;
            RenderView(&Views[0].c, GL_ScreenMinY, GL_ScreenH/2);
            Views[0].c.tilt -= 20;
            if (!(Views[0].c.data.flags & F3DCF_STATIC))
                RestoreTargetPos(cspr, cthn);

                // Snoop wires.
            RACE_HandleComms(FALSE);

                // Read joysticks if appropiate.
            UpdateJoystick();

            if (!(Views[1].c.data.flags & F3DCF_STATIC))
                RemoveTargetPos(&Views[1], cspr, cthn);
            POLY_MinY = GL_ScreenMinY+GL_ScreenH/2; POLY_MaxY = GL_ScreenMinY+GL_ScreenH;
            DRW_SetClipZone(POLY_MinX, POLY_MinY, POLY_MaxX, POLY_MaxY, NULL);
            Views[1].c.tilt += 20;
            RenderView(&Views[1].c, POLY_MinY, GL_ScreenH/2);
            Views[1].c.tilt -= 20;
            POLY_MinY = GL_ScreenMinY; POLY_MaxY = GL_ScreenMaxY;
            DRW_SetClipZone(GL_ScreenMinX, GL_ScreenMinY, GL_ScreenMaxX, GL_ScreenMaxY, NULL);
            if (!(Views[1].c.data.flags & F3DCF_STATIC))
                RestoreTargetPos(cspr, cthn);
        }

            // Snoop wires.
        RACE_HandleComms(FALSE);

            // Read joysticks if appropiate.
        UpdateJoystick();

            // Cars sounds

        if ((ConsolePlayer1 != NULL && (ConsolePlayer1->flags & PLYF_SLIDING) && Abs32((sint16)(ConsolePlayer1->thn->angle - ConsolePlayer1->ma)) > 0x600)
         || (ConsolePlayer2 != NULL && (ConsolePlayer2->flags & PLYF_SLIDING) && Abs32((sint16)(ConsolePlayer2->thn->angle - ConsolePlayer2->ma)) > 0x600)) {
            if (!isderrape) {
//                GAME_EFF_UnlockChannel(SH_Vtal, 3);
                GAME_EFF_Start(SH_Vtal, 3, 256, 100, 0, RACE_EffDerrape);
//                GAME_EFF_LockChannel(SH_Vtal, 3);
                isderrape = TRUE;
            }
        } else if (isderrape) {
            isderrape = FALSE;
//            GAME_EFF_UnlockChannel(SH_Vtal, 3);
            GAME_EFF_Stop(SH_Vtal, 3);
        }

        if (SH_Vtal != NULL) efftime  = SH_Vtal->EffTickCount - lefftime;
        if (SH_Vtal != NULL) lefftime = SH_Vtal->EffTickCount;

        GAME_EFF_SetChannelFreq(SH_Vtal, 0, (((pplayer->revo >> 16) * MOTORRAT) >> 6) + MOTORMIN, efftime);
            // One player
        if (ConsolePlayer2 == NULL && NCars > 0) {
            int c0, c1, p0 = 0, p1 = 0, d0 = 0x7FFFFFFF, d1 = 0x7FFFFFFF;
            int dx, dy, d, e;
            uint16 a0, a1;

            for (i = 0; i < NCars; i++) {
//                dx = (cars[i]->thn->x >> 20) - (pplayer->thn->x >> 20);
//                dy = (cars[i]->thn->y >> 20) - (pplayer->thn->y >> 20);
                dx = (cars[i]->thn->x >> 20) - (Views[0].c.dx >> 20);
                dy = (cars[i]->thn->y >> 20) - (Views[0].c.dy >> 20);
                d = dx*dx + dy*dy;

                if (d < d1) {
                    if (d < d0) {
                        d1 = d0;
                        p1 = p0;
                        d0 = d;
                        p0 = i;
                    } else {
                        d1 = d;
                        p1 = i;
                    }
                }
            }
            if (p0 == CloseCar0 || p0 == CloseCar1) {
                if (p0 == CloseCar0) {
                    c0 = 1;
                    c1 = 2;
                } else {
                    c0 = 2;
                    c1 = 1;
                }
            } else {
                if (p1 == CloseCar1) {
                    c0 = 1;
                    c1 = 2;
                } else {
                    c0 = 2;
                    c1 = 1;
                }
            }
            CloseCar0 = p0;
            CloseCar1 = p1;
            e = (4+6)*2;
//            a0 = pplayer->thn->angle - GetAngle((sint32)(cars[p0]->thn->x - pplayer->thn->x) >> 8,
            a0 = Views[0].c.angle - GetAngle((sint32)(cars[p0]->thn->x - pplayer->thn->x) >> 8,
                                               -(sint32)(cars[p0]->thn->y - pplayer->thn->y) >> 8);
            GAME_EFF_SetChannelFreq   (SH_Vtal, c0, (((cars[p0]->v >> 16) * MOTORRAT) >> 6) + MOTORMIN, efftime);
            e = ((1<<(e+8))/15) / (((1<<e)/15)+d0)/4;
            if (e > 64) e = 64;
            if (GL_CarType == 1)
                e = e*2;
            GAME_EFF_SetChannelVolume (SH_Vtal, c0, e, efftime);
            GAME_EFF_SetChannelPanning(SH_Vtal, c0, a0 >> 8, efftime);
            e = (4+6)*2;
//            a1 = pplayer->thn->angle - GetAngle((sint32)(cars[p1]->thn->x - pplayer->thn->x) >> 8,
            a1 = Views[0].c.angle - GetAngle((sint32)(cars[p1]->thn->x - pplayer->thn->x) >> 8,
                                               -(sint32)(cars[p1]->thn->y - pplayer->thn->y) >> 8);
            GAME_EFF_SetChannelFreq   (SH_Vtal, c1, (((cars[p1]->v >> 16) * MOTORRAT) >> 6) + MOTORMIN, efftime);
            e = ((1<<(e+8))/15) / (((1<<e)/15)+d1)/4;
            if (e > 64) e = 64;
            if (GL_CarType == 1)
                e = e*2;
            GAME_EFF_SetChannelVolume (SH_Vtal, c1, e, efftime);
            GAME_EFF_SetChannelPanning(SH_Vtal, c1, a1 >> 8, efftime);
        } else if (ConsolePlayer2 != NULL) {
            // Two players
            GAME_EFF_SetChannelFreq(SH_Vtal, 1, (((ConsolePlayer2->revo >> 16) * MOTORRAT) >> 6) + MOTORMIN, efftime);
        }


        if (!SplitScreenMode) {
            bool salp;
            if (ViewHud[0]) {
                salp = !(Views[0].c.data.flags & F3DCF_STATIC) && Views[0].c.data.radius == 0;
                if (salp)
                    HUD_DrawSalp(DRW_MaxY);
                HUD_DrawPlayer(pplayer, GameClock, mode, salp);
                DrawMap(ConsolePlayer1, LLS_SizeX - 76, GL_ScreenMinY + 4,
                        68, 68, 0x1CD0000, mode);
            }
            HUD_DrawPlayerLogos(pplayer, GameClock, mode,
                                (mode != 2)?Map.racers[0]->thn:NULL);
        } else {
            if (ViewHud[0]) {
                POLY_MinY = GL_ScreenMinY; POLY_MaxY = GL_ScreenMinY+GL_ScreenH/2;
                HUD_Draw2Player(ConsolePlayer1, GameClock, mode, GL_ScreenMinY, FALSE);
                DrawMap(ConsolePlayer1, LLS_SizeX - 92, GL_ScreenMinY + 2,
                        36, 36, 0x1CD0000*3/2, mode);
            }
            HUD_Draw2PlayerLogos(ConsolePlayer1, GameClock, mode, GL_ScreenMinY,
                                 (mode != 2)?Map.racers[0]->thn:NULL);
            if (ViewHud[1]) {
                POLY_MinY = GL_ScreenCenterY; POLY_MaxY = GL_ScreenCenterY+GL_ScreenH/2;
                HUD_Draw2Player(ConsolePlayer2, GameClock, mode, GL_ScreenMinY+GL_ScreenH/2, FALSE);
                DrawMap(ConsolePlayer2, LLS_SizeX - 92, GL_ScreenCenterY + 2,
                        36, 36, 0x1CD0000*3/2, mode);
            }
            HUD_Draw2PlayerLogos(ConsolePlayer2, GameClock, mode, GL_ScreenMinY+GL_ScreenH/2,
                                 (mode != 2)?Map.racers[0]->thn:NULL);
        }

        if (Trace) VGA_PutColor(0, 0, 63, 63);

            // Controls.
        if (startDelay > 0) {
            IS2_PSprite sp;
            int sec = startDelay / 70;

            if (sec != countsmpsec && sec < SIZEARRAY(countsmp)) {
                GAME_EFF_UnlockChannel(SH_Vtal, countsmpch);
                countsmpch = GAME_EFF_Start(SH_Vtal, 3, 256, 400, 0, countsmp[sec]);
                countsmpsec = sec;
                GAME_EFF_LockChannel(SH_Vtal, countsmpch);
            }
            sp = countdown[sec];
            DRW_TranslatePtr = GL_ClrTable + 256*16;
            IS2_Draw(sp, GL_ScreenCenterX, GL_ScreenCenterY,
                     GL_ScreenXRatio*sp->w*30/(startDelay%70 + 5),
                     GL_ScreenYRatio*sp->h*30/(startDelay%70 + 5));
        } else if (countsmpch != -1) {
            GAME_EFF_UnlockChannel(SH_Vtal, countsmpch);
            countsmpch = -1;
        }

        if (countgover >= 0) {
            DRW_TranslatePtr = GL_ClrTable + 256*16;
            IS2_Draw(gameover, LLS_SizeX/2, LLS_SizeY/2, gameover->w*30/(countgover + 30), gameover->h*30/(countgover + 30));
        }

            // Snoop wires.
        RACE_HandleComms(FALSE);

        LastFrameTimer = TIMER_Clock;

    getinfo:

        if (Quitting == 1) {
            // Prompt for exit.
            int x, y;

            x = LLS_SizeX/2 - 160;
            y = LLS_SizeY/2 - 100;
            GFX_Rectangle(x+20, y+70, 280,  60, 15, -1);
            GFX_Rectangle(x+21, y+71, 278,  58,  7, -1);
            GFX_Rectangle(x+22, y+72, 276,  56,  8, -1);
            GFX_Rectangle(x+23, y+73, 274,  54,  7,  8);

            TEXT_Write(&GL_YFont, x+35, y+  80, "Abort race", 15);
            TEXT_Write(&GL_RFont, x+25, y+ 105, "Press  ESC  to confirm.", 15);
        }

        if ((sint32)(TIMER_Clock-LastFrameTimer) > 4*70) {
            // Prompt for exit.
            int x, y;

            x = LLS_SizeX/2 - 160;
            y = LLS_SizeY/2 - 100;
            GFX_Rectangle(x+20, y+70, 280,  60, 15, -1);
            GFX_Rectangle(x+21, y+71, 278,  58,  7, -1);
            GFX_Rectangle(x+22, y+72, 276,  56,  8, -1);
            GFX_Rectangle(x+23, y+73, 274,  54,  7,  8);

            TEXT_Write(&GL_YFont, x+35, y+  75, "Network Jammed", 15);
            TEXT_Write(&GL_WFont, x+25, y+  90, "Some node is not responding", 15);
            TEXT_Write(&GL_RFont, x+25, y+ 115, "Press  CTRL-ESC  to quit.", 15);
        }

        if (DevKeys && LLK_Keys[kI]) {
            char buf[200];
            if (nsims > 0) {
                sprintf(buf, "framerate: %lf", (double)70.0/(double)nsims);
                TEXT_Write(&FONT_Border, 0, GL_ScreenMinY, buf, 12);
            }
#define P(a,y) \
            sprintf(buf, #a ": %d", (a)); \
            TEXT_Write(&FONT_Border, 0, (GL_ScreenMinY+y), buf, 12);

            P(Views[0].c.data.radius, 10);
            P(Views[0].c.data.h, 20);
            P(Views[0].c.data.focus, 30);
            P(Views[0].c.data.horizon, 40);
            {
                int c1, c2;
                c1 = HumanPlayers[0]->clock-GlobalClock;
                P(c1, 60);
                if (HumanPlayers[1] != NULL) {
                    c2 = HumanPlayers[1]->clock-GlobalClock;
                    P(c2, 80);
                }
            }
#undef P
        }
        if (Trace) VGA_PutColor(0, 63, 63, 0);
        LLS_UpdateMinY = GL_ScreenMinY;
        LLS_UpdateMaxY = GL_ScreenMaxY;
        LLS_Update();
        nFrames++;
        VGA_PutColor(0, 0, 0, 0);

            // Snoop wires.
        RACE_HandleComms(FALSE);

    } while (countgover != 0
          && (Quitting < 2)
          && !(LLK_Keys[kESC]
           && (LLK_Keys[kLEFTCTRL] || LLK_Keys[kRIGHTCTRL])));// Always a way to ESC.

  endrace:

    LLS_UpdateMinY = 0;
    LLS_UpdateMaxY = 200;


    TIMER_HookFunction = &GenericTimerFunction;

//    LogMemory("RACE_DoRace: Despues del bucle");

    j = 0;
    for (i = 0; i < NET_MAXNODES && i < NHumanPlayers; i++) {
        if (HumanPlayers[i] != NULL) {
            result[j].nlaps = HumanPlayers[i]->racer->nlap + (HumanPlayers[i]->status == PLYST_WON);
            result[j].time  = HumanPlayers[i]->racer->totaltime;
            result[j].best  = HumanPlayers[i]->racer->bestlaptime;
            result[j].flags = RACERF_ISCONSOLE*((HumanPlayers[i]->flags & PLYF_ISCONSOLE) != 0);
            result[j].pos   = Map.nracers-1;
            for (k = 0; k < Map.nracers; k++) {
                if (Map.racers[k]->thn == HumanPlayers[i]->thn) {
                    result[j].pos   = k;
                    break;
                }
            }
            j++;
        }
    }
    while (j < NET_MAXNODES) {
        result[j].flags = RACERF_NOTPRESENT;
        j++;
    }
    GAME_EFF_StopAll(SH_Vtal);
    //LogMemory();
    DISPOSE(gameover);

    for (i = 0; i < SIZEARRAY(HumanSprs); i++) {
        DISPOSE(HumanSprs[i]);
    }
    for (i = 0; i < SIZEARRAY(PlayerPosDots); i++) {
        DISPOSE(PlayerPosDots[i]);
    }
    for (i = 0; i < SIZEARRAY(countdown); i++) {
        DISPOSE(countdown[i]);
        if (i < SIZEARRAY(countsmp)) {
            GAME_EFF_Unload(SH_Vtal, countsmp[i]);
        }
    }
//    LogMemory("RACE_DoRace: Antes de EndData");

    EndData();
    return NHumanPlayers;
}


// =============================

PRIVATE bool ConsoleDemoRoutine(dword clock) {
    int k;

    k = clock % MAX_USERCONTROLS;
    if (ConsoleInput[k] == kSYSREQ || ConsoleInput[k] == kPRTSC) {
        GL_Capture(ConsoleInput[k]);
        return FALSE;
    }
    return (ConsoleInput[k] != 0);
}

PUBLIC bool RACE_DoDemo(void) {
    dword nFrames;
    int  i;
    bool leave = FALSE;
    IS2_PSprite demologo;
    FONT_TFont demofont;
    int lastcamera = STDCAM_STATIC;

//    LogMemory("RACE_DoDemo");

    if (O3DM_MaxDetail > O3DD_TEXGOURAUD)
        O3DM_MaxDetail = O3DD_TEXGOURAUD;
    GL_SelCircuit = 0;
    if (!InitData(3, FALSE))
        return FALSE;

    demologo = IS2_Load("demologo.is2");
    REQUIRE(demologo != NULL);
    FONT_Load(&demofont, "mfontyel.fnt");

    SplitScreenMode = FALSE;

        // Adjust changed parameters.
    if (SplitScreenMode)
        F3D_StdCams = F3D_StdCams2Player;
    else
        F3D_StdCams = F3D_StdCams1Player;

    InitView(Views + 0, cars[RND_GetNum()%10]->thn);
    Views[0].CamTo  = F3D_StdCams[lastcamera];
    Views[0].CamChangePos = 0;

    //LogMemory();

    GlobalClock = 0;
    ConsoleClock = 0;
    GameClock = 0;
    RaceStarted = TRUE;
    Paused = RaceStopped = FALSE;
    nFrames = 0;
    HandleView(Views+0, 200);
    TIMER_HookFunction = &TimerUserControl;

    {
        int i;
        for (i = 0; i < 2000; i++)
            THN_RunThings();
    }
    memset(LLS_Screen[0], 0, LLS_Size);
    LLS_Update();
    VGA_DumpPalette(GamePal, 0, 256);

    do {
        int i, s, nsims;

        if (Trace) VGA_VSync();
        if (Trace) VGA_PutColor(0, 63, 0, 0);

        nsims = 0;
            // Run simulation of world.
        while (TRUE) {
            if (ConsoleClock <= GlobalClock)
                break;
            leave |= ConsoleDemoRoutine(GlobalClock);
            THN_RunThings();
            GlobalClock++;
            HandleView(Views+0, 200);
            GameClock++;

                // Randomily change camera.
            if ((GameClock & 0xFF) == 0x7F && RND_GetNum()%4 == 0) {
                int c;
                do c = RND_GetNum()%6; while (c == lastcamera);
                if (c < 4) {
                    Views[0].CamTo  = F3D_StdCams[c];
                    Views[0].c.tilt = 0;
                    Views[0].CamChangePos = 0;
                    TSTangle = 0;
                    if (lastcamera >= 4) {
                        Views[0].c.data = Views[0].CamTo;   // Instant change!
                        Views[0].CamChangePos = -1;
                        Views[0].c.data.focus = Views[0].c.data.focus*GL_ScreenXRatio;
                        Views[0].c.data.horizon = Views[0].c.data.horizon*GL_ScreenYRatio;
                    }
                } else {
                    Views[0].CamTo = F3D_StdCams[0];
                    Views[0].c.data = Views[0].CamTo;   // Instant change!
                    Views[0].c.data.h = 200+300*(RND_GetNum()%10);
                    Views[0].c.tilt = (Views[0].c.data.h-200)*80/3000;
                    Views[0].c.data.radius = (20 << 20);
                    Views[0].c.data.focus = 0x1000*GL_ScreenXRatio;
                    Views[0].c.data.horizon = Views[0].c.data.horizon*GL_ScreenYRatio;
                    TSTangle = -0x6000;
                    Views[0].CamChangePos = -1;
                }
                lastcamera = c;
            }
                // Randomily change car.
            if ((GameClock & 0xFF) == 0x40 && RND_GetNum()%4 == 0) {
                int c;
                do c = RND_GetNum()%6; while (c == lastcamera);
                AdvanceTarget = 1;
                    // Change camera too.
                if (c < 4) {
                    Views[0].CamTo  = F3D_StdCams[c];
                    Views[0].c.tilt = 0;
                    Views[0].c.data = Views[0].CamTo;   // Instant change!
                    Views[0].c.data.focus = Views[0].c.data.focus*GL_ScreenXRatio;
                    Views[0].c.data.horizon = Views[0].c.data.horizon*GL_ScreenYRatio;
                    TSTangle = 0;
                } else {
                    Views[0].CamTo = F3D_StdCams[0];
                    Views[0].c.data = Views[0].CamTo;   // Instant change!
                    Views[0].c.data.h = 200+300*(RND_GetNum()%10);
                    Views[0].c.tilt = (Views[0].c.data.h-200)*80/3000;
                    Views[0].c.data.radius = (20 << 20);
                    Views[0].c.data.focus = 0x1000*GL_ScreenXRatio;
                    Views[0].c.data.horizon = Views[0].c.data.horizon*GL_ScreenYRatio;
                    TSTangle = 0x6000;
                }
                lastcamera = c;
                Views[0].CamChangePos = -1;
            }
            if (lastcamera == 4)
                TSTangle += 128;
            else if (lastcamera == 5)
                TSTangle -= 128;
            nsims++;
        }

        {
            if (AdvanceTarget != 0) {
                THN_PThing thn;
                ViewCar = ViewCar + AdvanceTarget;
                if (ViewCar < 0)
                    ViewCar = NCars-1;
                else if (ViewCar > NCars-1)
                    ViewCar = 0;
                AdvanceTarget = 0;
                thn = cars[ViewCar]->thn;
                Views[0].thn = thn;
                Views[0].c.angle = thn->angle;
                Views[0].c.dx    = thn->x;
                Views[0].c.dy    = thn->y;
            }
        }

            // -------- Render view.

        POLY_MinY = GL_ScreenMinY; POLY_MaxY = GL_ScreenMaxY;
        DRW_SetClipZone(GL_ScreenMinX, GL_ScreenMinY, GL_ScreenMaxX, GL_ScreenMaxY, NULL);
        RenderView(&Views[0].c, GL_ScreenMinY, GL_ScreenH);
        IS2_DrawHorizontal(demologo, 10, 10);
        if ((GlobalClock & 0x40) == 0) {
            int w;
            TEXT_GetExtent(&demofont, 0, 0, "Press Any Key", &w, 0);
            TEXT_Write(&demofont, GL_ScreenCenterX-w/2, GL_ScreenMaxY - 30, "Press Any Key", 15);
        }

        if (Trace) VGA_PutColor(0, 63, 63, 0);
        LLS_Update();
        nFrames++;

        if (GameClock > 70*60)
            break;
        if (Trace) VGA_PutColor(0, 0, 0, 0);

        if (LLK_Keys[kESC] && (LLK_Keys[kLEFTCTRL] || LLK_Keys[kRIGHTCTRL]))
            break;
    } while (!leave);          // Always a way to ESC.
    TIMER_HookFunction = &GenericTimerFunction;

    //LogMemory();
    DISPOSE(demologo);
    FONT_End(&demofont);
    EndData();
    return leave;
}
