// ------------------------------ NET.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _NET_H_
#define _NET_H_

#include <comms.h>
    // Use the fancy VTAL stacks during interrupts.
#include <vtal.h>

#include "userctl.h"

    // Max. packets to send ahead of time before resending
#define NET_NTIMES 32 //32

    // Max. number of nodes including ourselves.
#define NET_MAXNODES 4

#define NET_NCTL 4

enum {
    NETC_SYNC,
    NETC_NODEID,
    NETC_START,
    NETC_GAMEINFO,
    NETC_READY,
    NETC_INFO,
    NETC_BYE,
    NETC_RESEND,
};

typedef struct {
    COM_TPacketHeader;

    byte  command;      // NETC_SYNC
    byte  addr[6];
    byte  nodenum;      // Assigned by the master. If == 255 means yet unassigned.
                        // Master will always be 0.
    byte  tracknum;
    byte  racelaps;
    byte  cartype;
    byte  racemode;
    dword seed;
    byte  kup;
    byte  kdn;
    byte  klt;
    byte  krt;
    byte  kge;
} NET_TSyncPacket, *NET_PSyncPacket;

typedef struct {
    COM_TPacketHeader;

    byte  command;      // NETC_NODEID
    byte  yourid;
} NET_TNodeIdPacket, *NET_PNodeIdPacket;

typedef struct {
    COM_TPacketHeader;

    byte  command;      // NETC_START || NETC_BYE
    byte  oknodes;
} NET_TStartPacket, *NET_PStartPacket;

typedef struct {
    COM_TPacketHeader;

    byte  command;      // NETC_GAMEINFO
    byte  carmodel[2];
    byte  nnodes;       // Number of nodes whose car models I know.
} NET_TGameInfoPacket, *NET_PGameInfoPacket;

typedef struct {
    COM_TPacketHeader;

    byte  command;      // NETC_READY
} NET_TReadyPacket, *NET_PReadyPacket;

typedef struct {
    COM_TPacketHeader;

    byte  command;      // NETC_INFO
    byte thisclock;     // clock of this packet info.
    byte sendclock;     // clock they are expecting to receive.
    UCT_TUserControl ctl[NET_NCTL];    // Actual info data.
//    dword thisFullClock;
} NET_TGamePacket, *NET_PGamePacket;

typedef struct {
    COM_TPacketHeader;

    byte  command;      // NETC_RESEND
    int   resendfrom;
} NET_TResendPacket, *NET_PResendPacket;

typedef union {
    COM_TPacket p;
    NET_TSyncPacket;
    NET_TNodeIdPacket;
    NET_TStartPacket;
    NET_TGameInfoPacket;
    NET_TReadyPacket;
    NET_TGamePacket;
    NET_TResendPacket;
} NET_TPacket, *NET_PPacket;

// ----------------------------

    // Nodes are aligned with COM_Links, therefore we are node 0.
    // But we also have a netgame node number assigned by the master.
typedef struct NET_SNodeData {
    dword sendtime, rectime, ringtime;
    UCT_TUserControl    ctl[NET_NTIMES][2]; // Up to two players per node.
    PLY_PPlayer         p[2];
    bool                master;
    byte                nodenum;
    byte  kup;
    byte  kdn;
    byte  klt;
    byte  krt;
    byte  kge;
    int                 ctlnum[NET_NTIMES]; // Check
} NET_TNodeData, *NET_PNodeData;

// ----------------------------

enum {
    NETS_NONE,
    NETS_IPX,
    NETS_SERIAL,
};

    // Usable stack for calling a net routine.
PUBLIC XTRN_TStack NET_ISRStack;
PUBLIC int NET_State;           // NETS_xxxx
PUBLIC int NET_NumNodes;        // Number of active nodes.

    // Nodes are aligned with COM_Links. Node 0 is ourselves.
PUBLIC NET_TNodeData NET_NodeData[NET_MAXNODES];

    // This is based on gamenode numbers (master == 0), gives number of link.
PUBLIC int NET_NodeNum[NET_MAXNODES];

PUBLIC bool NET_ModemOn;    // TRUE if modem call is established.

PUBLIC bool NET_Slave;      // Am I the slave?
PUBLIC int  NET_NMaster;    // NodeData index of race master.

PUBLIC int NET_MaxNodes;

PUBLIC int NET_MaxTicks;

// ----------------------------

PUBLIC void NET_End(void);

PUBLIC int NET_GameMenu(void);

PUBLIC int NET_EditPhones(void);

PUBLIC int NET_ContactPlayers(int *racemode);

PUBLIC int NET_GetCars(void);

PUBLIC int NET_SyncStart(void);

#endif

// ------------------------------ NET.H ----------------------------
