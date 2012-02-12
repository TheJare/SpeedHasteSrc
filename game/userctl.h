// ------------------------------ USERCTL.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _USERCTL_H_
#define _USERCTL_H_

#include "things.h"
#include "racers.h"

#define MAX_USERCONTROLS 32
#define MAX_PLAYERS 4

typedef struct {
    byte scancode;
    byte control;
    sint8 analogX;
    sint8 analogY;
} UCT_TUserControl, *UCT_PUserControl;

typedef struct {
    byte source;        // UCTS_xxxx
    byte kleft, kright, kup, kdn, kgear;    // kXXX constants.
} UCT_TInputConfig, *UCT_PInputConfig;

enum {
        // Input sources. Keep them aligned to the CFG_xx constants.
    UCTS_KEYBOARD = 0,
    UCTS_JOYA     = 1,
    UCTS_JOYB     = 2,
    UCTS_MOUSE    = 3,

        // Usercontrol bits.
    UCTL_LEFT  = 0x01,
    UCTL_RIGHT = 0x02,
    UCTL_UP    = 0x04,
    UCTL_DOWN  = 0x08,
    UCTL_GEAR  = 0x10,
    UCTL_GEARUP = 0x20,
    UCTL_GEARDN = 0x40,
};

struct NET_SNodeData;

typedef struct PLY_SPlayer {
    THN_PThing       thn;
    word             flags;
    word             gear;
    word             ma;
    sint32           revo;
    sint32           va, v;
    dword            clock;
    RCS_PRacer       racer;
    UCT_TInputConfig cfg;
    UCT_TUserControl UserControl[MAX_USERCONTROLS];
    sint32           status;
    sint32           slid, slidcounter, slidspeed, slidva;
    sint32           tires, movangle;
    sint32           cartype, maxspeed, damage, automatic;
    sint32           reverse;
    sint32           fill[3];

        // Link to node info.
    struct NET_SNodeData *net;
} PLY_TPlayer, *PLY_PPlayer;

enum {
    PLYF_PRESENT   = 0x0001,
    PLYF_ISCONSOLE = 0x0002,

    PLYF_SLIDING   = 0x8000,
    PLYF_TRACTING  = 0x4000,

    PLYST_RACING   = 0,
    PLYST_BROKEN   = 1,
    PLYST_WON      = 2,
    PLYST_TIMEOUT  = 3,
};

PUBLIC bool UCT_New(THN_PThing t, int type, word flags);

PUBLIC void UCT_GetUserControl(UCT_PUserControl ctl,
                               dword clock, UCT_PInputConfig cfg);

PUBLIC void UCT_DoControl(PLY_PPlayer p);

#endif

// ------------------------------ USERCTL.H ----------------------------

