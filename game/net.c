// ------------------------------ NET.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "net.h"

#include <timer.h>
#include <vga.h>
#include <vbl.h>

#include <llkey.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <llscreen.h>
#include <graphics.h>

#include "menus.h"
#include "globals.h"
#include "config.h"

PUBLIC int NET_State = NETS_NONE;
PUBLIC int NET_NumNodes = 0;
NET_TNodeData NET_NodeData[NET_MAXNODES];
int NET_NodeNum[NET_MAXNODES];

PUBLIC bool NET_ModemOn = FALSE;

PUBLIC bool NET_Slave  = FALSE;
PUBLIC int  NET_NMaster = -1;

PUBLIC int NET_MaxNodes = 4;

PUBLIC int NET_MaxTicks = 1;

/*
    All nodes have info of their own, generated at every tick.
    The info that a node will accept from another node is the info
    with a time equal to 1 more than the previous accepted info
    from that node. Therefore, one must send, at least, the info
    at the minimum time that all nodes are expecting.

    One can't generate new info if the most off-time node is too
    far for the buffer to hold them all.

    If a node loses a packet, it will be delayed with respect to all
    other nodes. This can be noticed by the node that sent that packet
    because it has to resend it.

    Options:
      1- Send the same info to every node, i.e., that of the most off-time
        node.
      2- Keep info about the times that every node is expecting, and
        send the appropiate info for each node.

    1 will somehow transmit that delay to all other nodes, because it
    won't update them with the newest info. 2 will leave that node
    behind for one tick with respect to the others. If such a delay
    accumulates, that node would be always off-time (until all other
    nodes eventually lose one packet).

    2 means to send a potentially different packet to every node. If
      a node is delayed with respect to the others, one could feed it
      with more packets than to the others to get it updated.

*/


// -----------------------

PRIVATE void NetShow(FONT_TFont *font, const char *text) {
    int x, y;

    x = LLS_SizeX/2 - 160;
    y = LLS_SizeY/2 - 100;
    GFX_Rectangle(x+20, y+70, 280,  60, 15, -1);
    GFX_Rectangle(x+21, y+71, 278,  58,  7, -1);
    GFX_Rectangle(x+22, y+72, 276,  56,  8, -1);
    GFX_Rectangle(x+23, y+73, 274,  54,  7,  8);

    TEXT_Write(&GL_YFont, x+25, y+ 75, "Contacting players", 15);
    TEXT_Write(&GL_YFont, x+40, y+115, "ESC to Abort", 15);
    if (text != NULL)
        TEXT_Write(font, x+26, y+95, text, 15);
    LLS_Update();
}

PUBLIC int NET_ContactPlayers(int *racemode) {
    int numoknodes = 0;
    int i, j, nlink;
    bool userabort = FALSE;
    char buf[200];

    for (i = 0; i < SIZEARRAY(NET_NodeData); i++)
        NET_NodeData[i].nodenum = 0xFF;

    NET_NodeData[0].kup    = CFG_Config.P1KeyUp;
    NET_NodeData[0].kdn    = CFG_Config.P1KeyDn;
    NET_NodeData[0].klt    = CFG_Config.P1KeyLt;
    NET_NodeData[0].krt    = CFG_Config.P1KeyRt;
    NET_NodeData[0].kge    = CFG_Config.P1KeyGe;

    if (NET_Slave) {
//        printf("Going as slave.\n");
        NET_NodeData[0].nodenum = 0xFF;
        NET_NMaster = -1;
    } else {
//        printf("Going as master.\n");
        NET_NodeData[0].nodenum = 0;
        numoknodes++;
        NET_NMaster = 0;
    }

        // Recognize active nodes around.
    VBL_VSync(0);
    LLK_LastScan = 0;
    j = 70;
    for (;;) {
        NET_TPacket p;

        if (LLK_LastScan == kESC) {
            userabort = TRUE;
            break;
        }

            // If we are master we can start the game.
        if (!NET_Slave && LLK_LastScan != 0) {   // Master starts game.
            if (numoknodes != (COM_NLinks+1)) {
                ;//BASE_Abort("Some nodes are unidentified when forcing start! %d != %d", numoknodes, COM_NLinks+1);
            } else
                break;
        }
        LLK_LastScan = 0;
        if (j >= 30) {  // Send packets every 30 ticks.
            memcpy(p.addr, COM_MyAddress.addr, 6);
            p.command = NETC_SYNC;
            p.len  = sizeof(NET_TSyncPacket);
            p.time = 0;
            p.nodenum = NET_NodeData[0].nodenum;
            p.tracknum = GL_SelCircuit;
            p.cartype  = GL_CarType;
            p.racelaps = (sint8)(GL_RaceLaps & 0x3F) | (0x80*GL_TimedRace);
            p.seed     = GL_Seed;
            p.racemode = *racemode;
            p.kup    = NET_NodeData[0].kup;
            p.kdn    = NET_NodeData[0].kdn;
            p.klt    = NET_NodeData[0].klt;
            p.krt    = NET_NodeData[0].krt;
            p.kge    = NET_NodeData[0].kge;

            COM_SendPacket(0, &p.p);
            if (!NET_Slave) {
                for (i = 1; i <= COM_MaxLinks; i++) {
                    if (COM_Links[i-1].type == COM_Type) {
                        p.command = NETC_NODEID;
                        p.len = sizeof(NET_TNodeIdPacket);
                        p.time = 0;
                        p.yourid = i;
                        COM_SendPacket(i, &p.p);
                    }
                }
            }
            j = 0;
        }
        j += VBL_VSync(1);

        if (!NET_Slave && numoknodes == (COM_NLinks + 1)) {
            sprintf(buf, "Press Enter to accept %d nodes", numoknodes);
        } else {
            sprintf(buf, "%d nodes registered", numoknodes);
        }
        NetShow(&GL_WFont, buf);

        COM_Housekeep();
        nlink = COM_GetPacket(&p.p);

            // Messages from link 0 are broadcast, ignore them and just
            // use them to add new links.
        if (nlink >= 0) {
            if (nlink == 0  && p.command == NETC_SYNC && p.len == sizeof(NET_TSyncPacket)) {
                COM_TAddress a;
                memcpy(a.addr, p.addr, 6);
                nlink = COM_AddLink(&a);
                if (nlink < 0)
                    continue;
                //printf("New link %d.\n", nlink);
            }
            if (nlink > 0 && p.command == NETC_SYNC && p.len == sizeof(NET_TSyncPacket)) {
//                printf("Sync packet from link %d.\n", nlink);
                if (p.nodenum != 0xFF) {
                    if (NET_NodeData[nlink].nodenum == 0xFF)
                        numoknodes++;
                    else if (NET_NodeData[nlink].nodenum != p.nodenum) {
                        //BASE_Abort("Node changed opinion about its nodenum! %d to %d", NET_NodeData[nlink].nodenum, p.nodenum);
                        NetShow(&GL_RFont, "Only one master allowed");
                        VBL_VSync(70);
                        userabort = TRUE;
                        break;
                    }
                    NET_NodeData[nlink].nodenum = p.nodenum;
                    NET_NodeNum[p.nodenum] = nlink;
                    //printf("Says he's node number %d.\n", p.nodenum);
                }
                    // Packet from master?
                if (p.nodenum == 0) {
                    if (!NET_Slave) {
                        //BASE_Abort("Two master nodes!");
                        NetShow(&GL_RFont, "Only one master allowed");
                        VBL_VSync(70);
                        userabort = TRUE;
                        break;
                    } else {
                        NET_NMaster = nlink;
                        NET_NodeData[nlink].master = TRUE;
                        GL_SelCircuit = p.tracknum;
                        GL_RaceLaps = (sint8)(p.racelaps & ~0x80);
                        GL_TimedRace = (p.racelaps & 0x80) != 0;
                        GL_CarType  = p.cartype;
                        GL_Seed = p.seed;
                        *racemode  = p.racemode;
                    }
                }
                NET_NodeData[nlink].kup = p.kup;
                NET_NodeData[nlink].kdn = p.kdn;
                NET_NodeData[nlink].klt = p.klt;
                NET_NodeData[nlink].krt = p.krt;
                NET_NodeData[nlink].kge = p.kge;

                if (!NET_Slave) { // I am the master, I give node numbers.
                    //printf("I say he's node number %d.\n", nlink);
                }
            } else if (nlink > 0 && p.command == NETC_NODEID && p.len == sizeof(NET_TNodeIdPacket)) {
                if (!NET_NodeData[nlink].master) {
                    //BASE_Abort("Non-master node is giving node IDs");
                    NetShow(&GL_RFont, "Only one master allowed");
                    VBL_VSync(70);
                    userabort = TRUE;
                    break;
                }
                if (NET_NodeData[0].nodenum != 0xFF && NET_NodeData[0].nodenum != p.yourid) {
                    //BASE_Abort("I've been assigned different node numbers");
                    NetShow(&GL_RFont, "Only one master allowed");
                    VBL_VSync(70);
                    userabort = TRUE;
                    break;
                }
                //printf("He says I'm node number %d.\n", p.yourid);
                if (NET_NodeData[0].nodenum == 0xFF)
                    numoknodes++;
                NET_NodeData[0].nodenum = p.yourid;
            } else if (nlink > 0 && p.command == NETC_START && p.len == sizeof(NET_TStartPacket)) {
                if (!NET_NodeData[nlink].master) {
                    //BASE_Abort("Non-master node ordered to start!");
                    NetShow(&GL_RFont, "Only one master allowed");
                    VBL_VSync(70);
                    userabort = TRUE;
                    break;
                }
                if (numoknodes != (COM_NLinks+1)) {
                    //BASE_Abort("Some nodes are unidentified when starting! %d != %d", numoknodes, COM_NLinks+1);
                    NetShow(&GL_RFont, "Only one master allowed");
                    VBL_VSync(70);
                    userabort = TRUE;
                    break;
                }
                    // GO!!
                //printf("He says GO!!.\n");
                break;
            }
        } else if (nlink == -2) {
        }
    }
    if (userabort)
        return -1;
    if (!NET_Slave) {   // Tell others that the network identification is ok.
        NET_TPacket p;
        p.command = NETC_START;
        p.len = sizeof(NET_TStartPacket);
        p.time = 1;
        COM_SendPacket(0, &p.p);
    }

    NET_NumNodes = COM_NLinks+1;
    LLK_LastScan = 0;
    return GL_SelCircuit;
}

PUBLIC int NET_GetCars(void) {
    int numoknodes = 1;
    int i, j, nlink;
    int nodeok[NET_MAXNODES], numok = 1;
    char buf[200];
    bool userabort = FALSE;

    for (i = 0; i < SIZEARRAY(nodeok); i++)
        nodeok[i] = 0;

        // Recognize active nodes around.
    VBL_VSync(0);
    LLK_LastScan = 0;
    j = 70;
    for (;;) {
        NET_TPacket p;

        if (LLK_LastScan == kESC) {
            userabort = TRUE;
            break;
        }

        if (j >= 30) {  // Send packets every 30 ticks.
            p.command = NETC_GAMEINFO;
            p.len  = sizeof(NET_TGameInfoPacket);
            p.time = 0;
            p.carmodel[0] = GL_SelCar[0];
            p.carmodel[1] = GL_SelCar[1];
            p.nnodes = numoknodes;
            COM_SendPacket(0, &p.p);
            if (numok == NET_NumNodes)
                break;
            j = 0;
        }

        j += VBL_VSync(1);
        sprintf(buf, "%d nodes contacted", numok);
        NetShow(&GL_WFont, buf);
        COM_Housekeep();
        nlink = COM_GetPacket(&p.p);
        if (nlink > 0) {
            if (p.command == NETC_GAMEINFO && p.len == sizeof(NET_TGameInfoPacket)) {
                if (nodeok[nlink] == 0) {
                    nodeok[nlink]++;
                    numoknodes++;
                }
                if (p.nnodes == NET_NumNodes) {
                    if (nodeok[nlink] == 1)
                        numok++;
                    nodeok[nlink]++;
                }
                GL_SelCar[nlink*2+0] = p.carmodel[0];
                GL_SelCar[nlink*2+1] = p.carmodel[1];
//                j = 70;
            }
        } else if (nlink == -2) {
        }
    }
    if (userabort || numok != NET_NumNodes)
        return -1;
    LLK_LastScan = 0;
    return 0;
}

PUBLIC int NET_SyncStart(void) {
    int numoknodes = 1;
    int i, j, nlink;
    int nodeok[NET_MAXNODES], numok = 1;
    char buf[200];

    for (i = 0; i < SIZEARRAY(nodeok); i++)
        nodeok[i] = 0;

        // Recognize active nodes around.
    LLK_LastScan = 0;
    j = 70;
    for (;;) {
        NET_TPacket p;

        if (LLK_LastScan == kESC)
            break;

        if (j >= 70) {  // Send packets every 70 ticks.
            p.command = NETC_START;
            p.len  = sizeof(NET_TStartPacket);
            p.time = 0;
            p.oknodes = numoknodes;
            COM_SendPacket(0, &p.p);
            if (numok == NET_NumNodes)
                break;
            j = 0;
        }

        j += 1;
        VGA_VSync();
        sprintf(buf, "%d nodes contacted", numok);
        NetShow(&GL_WFont, buf);
        COM_Housekeep();
        nlink = COM_GetPacket(&p.p);
        if (nlink > 0) {
            if (p.command == NETC_START && p.len == sizeof(NET_TStartPacket)) {
                if (nodeok[nlink] == 0) {
                    nodeok[nlink]++;
                    numoknodes++;
                }
                if (p.oknodes == NET_NumNodes) {
                    if (nodeok[nlink] == 1)
                        numok++;
                    nodeok[nlink]++;
                }
  //              j = 70;
            }
        } else if (nlink == -2) {
        }
    }
    if (numok != NET_NumNodes)
        return -1;
    LLK_LastScan = 0;
    return 0;
}


// -----------------------

PRIVATE int StartOrJoinMenu(void) {
    static int opt = 0;     // So it stays from call to call.
    char *menu[] = {
        "Start Multiplayer Game",
        "Join Multiplayer Game",
        "Exit",
    };

    if (BASE_CheckArg("master") > 0) {
        NET_Slave = FALSE;
        return NET_Slave;
    } else if (BASE_CheckArg("slave") > 0) {
        NET_Slave = TRUE;
        return NET_Slave;
    }

    for (;;) {
        switch (MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt)) {
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                switch (opt) {
                    case SIZEARRAY(menu)-1:
                        return -1;
                    default:
                        NET_Slave = opt;
                        return opt;
                }
            case kESC:
                return -1;
        }
    }
}


// -----------------------

PRIVATE const char *ChoosePhoneMenu(void) {
    static int opt = 0;     // So it stays from call to call.
    char *menu[6+1];
    int nitems = 1;
    char (*pnum)[CFG_STRSIZE];

    for (pnum = &CFG_Config.phone1; pnum <= CFG_Config.phone6; pnum++) {
        if (isdigit(pnum[0][0])) {
            menu[nitems-1] = pnum[0];
            nitems++;
        }
    }
    menu[nitems-1] = "Exit";
    if (opt >= nitems)
        opt = 0;

    for (;;) {
        switch (MENU_DoControlMenu(menu, nitems, &opt)) {
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                if (opt == nitems-1)
                    return NULL;
                return menu[opt];
            case kESC:
                return NULL;
        }
    }
}

PRIVATE void DelayVBL(int mill) {
    VBL_VSync(0);
    while (LLK_LastScan == 0 && mill > 0) {
        VBL_VSync(1);
        mill -= 1000/70;    // Assume 70Hz
    }
}

PRIVATE void UpdateModemLine(const char *msg) {
        // Copy background.
    if (LLS_Mode < 0) {
        puts(msg);
        return;
    }
    RepMovsb(LLS_Screen[0], MENU_Back, LLS_Size);
    TEXT_Write(&MENU_FontRed, 50, 90, "MODEM STATUS", 15);
    if (msg != NULL)
        TEXT_Write(&MENU_FontYellow, 60, 120, msg, 15);
    LLS_Update();
}

PRIVATE void UpdateModemStatus(int com) {
    static const char *modemstrs[] = {
        "OK",
        "RING",
        "BUSY",
        "NO CARRIER",
        "NO DIALTONE",
        "CONNECT",
    };
    if (com == -1)
        UpdateModemLine("Key Pressed");
    else if (com > 0 && com <= SIZEARRAY(modemstrs))
        UpdateModemLine(modemstrs[com-1]);
}

    // Should return SERMS_OK
PRIVATE int InitModem(SER_PStream s, SER_PModemCfg cfg) {
    int ret;

    UpdateModemLine("Initializing modem");
    SER_WriteStream(s, cfg->initstr, strlen(cfg->initstr));
    SER_WriteStreamChar(s, '\r');
    SER_StreamTransmit(s);
	for(;;) {
	    ret = SER_WaitModem(s, NULL);
	    UpdateModemStatus(ret);
	    if (ret == SERMS_OK || ret < 0)
            break;
    }
    return ret;
}

    // Should return SERMS_OK
PRIVATE int Hangup(SER_PStream s, SER_PModemCfg cfg) {
    LLK_LastScan = 0;
    UpdateModemLine("Hanging up");
    SER_WriteStreamChar(s, 27);
    SER_StreamTransmit(s);
    if (cfg->delayfunc == NULL) delay (750);
    else                        cfg->delayfunc(750);
    UpdateModemLine("Dropping DTR");
    outp(s->port.port + SERR_MCR, inp(s->port.port+SERR_MCR) & ~SERR_MCR_DTR);
    if (cfg->delayfunc == NULL) delay (1250);
    else                        cfg->delayfunc(1250);
    outp(s->port.port + SERR_MCR, inp(s->port.port+SERR_MCR) | SERR_MCR_DTR);
    UpdateModemLine("Entering command mode");
    SER_WriteStream(s, "+++", strlen("+++"));
    SER_StreamTransmit(s);
    if (cfg->delayfunc == NULL) delay (1250);
    else                        cfg->delayfunc(1250);
    UpdateModemLine("Sending hangup string");
    SER_WriteStream(s, cfg->hangstr, strlen(cfg->hangstr));
    SER_WriteStreamChar(s, '\r');
    SER_StreamTransmit(s);
    return SER_WaitModem(s, NULL);
}

    // Should return SERMS_CONNECT
PRIVATE int Dial(SER_PStream s, SER_PModemCfg cfg, const char *number) {
    char buf[200];
    int ret;
    UpdateModemLine("Dialing");
    sprintf(buf, "%s%s\r", cfg->dialstr, number);
    SER_WriteStream(s, buf, strlen(buf));
    SER_StreamTransmit(s);
	for (;;) {
	    ret = SER_WaitModem(s, NULL);
        UpdateModemStatus(ret);
	    if (ret == SERMS_CONNECT)
            break;
        if (ret < 0 || ret == SERMS_NODIALTONE) {
            return Hangup(s, cfg);
        }
    }
    return ret;
}

    // Should return SERMS_CONNECT
PRIVATE int Answer(SER_PStream s, SER_PModemCfg cfg) {
    int ret;
    UpdateModemLine("Listening");
	for (;;) {
	    ret = SER_WaitModem(s, NULL);
        UpdateModemStatus(ret);
	    if (ret == SERMS_RING)
	        break;
	    if (ret < 0)
    	    return ret;
    }
	SER_WriteStream(s, "ATA\r", strlen("ATA\r"));
    SER_StreamTransmit(s);
	for (;;) {
	    ret = SER_WaitModem(s, NULL);
        UpdateModemStatus(ret);
	    if (ret == SERMS_CONNECT)
            break;
        if (ret < 0) {
            return Hangup(s, cfg);
        }
    }
    return ret;
}

PUBLIC void NET_End(void) {
    COM_End();
    if (NET_ModemOn) {
        SER_TModemCfg cfg;
        cfg.initstr = CFG_Config.modeminit;
        cfg.dialstr = CFG_Config.modemdial;
        cfg.hangstr = CFG_Config.modemhang;
        cfg.delayfunc = NULL;
        SER_InitComInfo(&SER_ScrapStream.port,
                        CFG_Config.comnum-1,
                        CFG_Config.comaddr,
                        CFG_Config.comirq);
        SER_InitComPort(&SER_ScrapStream.port);
        SER_InitStream(&SER_ScrapStream, SER_ScrapISR);
        if (Hangup(&SER_ScrapStream, &cfg) != SERMS_OK) {
            int x, y;

            x = LLS_SizeX/2 - 160;
            y = LLS_SizeY/2 - 100;
            GFX_Rectangle(x+20, y+70, 280,  60, 15, -1);
            GFX_Rectangle(x+21, y+71, 278,  58,  7, -1);
            GFX_Rectangle(x+22, y+72, 276,  56,  8, -1);
            GFX_Rectangle(x+23, y+73, 274,  54,  7,  8);

            TEXT_Write(&GL_RFont, x+25, y+ 75, "WARNING", 15);
            TEXT_Write(&GL_YFont, x+26, y+ 95, "HANGUP FAILED!", 15);
            TEXT_Write(&GL_RFont, x+40, y+115, "Call may still be active", 15);
            LLS_Update();
            LLK_LastScan = 0;
            while (LLK_LastScan == 0);
        }
        SER_EndComPort(&SER_ScrapStream.port);
    }
}

PUBLIC int NET_GameMenu(void) {
    bool leave = FALSE;
    int ret = -1;
    static int opt = 0;     // So it stays from call to call.
    static char seropt[] = "Serial Link";
    char *menu[] = {
        "IPX Network",
        seropt,
        "Dial Modem",
        "Answer Modem",
        "Hang Up Modem",
        "Exit",
    };

    memset(NET_NodeData, 0, sizeof(NET_NodeData));

    do {
        if (NET_ModemOn) strcpy(seropt, "Modem link");
        else             strcpy(seropt, "Serial link");
        switch (MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt)) {
            case kENTER:
            case kSPACE:
            case kKEYPADENTER: {
                const char *nph;
                SER_TModemCfg cfg;
                switch (opt) {
                    case 0:
                        if (!COM_IPXPresent) {
                                // Copy background.
                            RepMovsb(LLS_Screen[0], MENU_Back, LLS_Size);
                            TEXT_Write(&MENU_FontYellow, 50, 110, "No IPX driver", 15);
                            LLS_Update();
                            LLK_LastScan = 0;
                            while (LLK_LastScan == 0);
                            break;
                        }
                    case 1:
                        ret = opt; leave = TRUE; break;
                    case 2:
                        nph = ChoosePhoneMenu();
                        if (nph == NULL)
                            break;
                    case 3:
                    case 4:
                        cfg.initstr = CFG_Config.modeminit;
                        cfg.dialstr = CFG_Config.modemdial;
                        cfg.hangstr = CFG_Config.modemhang;
                        cfg.delayfunc = DelayVBL;
                        SER_InitComInfo(&SER_ScrapStream.port,
                                        CFG_Config.comnum-1,
                                        CFG_Config.comaddr,
                                        CFG_Config.comirq);
                        SER_InitComPort(&SER_ScrapStream.port);
                        SER_InitStream(&SER_ScrapStream, SER_ScrapISR);
                        if (opt == 2) {
                            InitModem(&SER_ScrapStream, &cfg);
                            if (Dial(&SER_ScrapStream, &cfg, nph) == SERMS_CONNECT)
                                NET_ModemOn = TRUE;
                        } else if (opt == 3) {
                            InitModem(&SER_ScrapStream, &cfg);
                            if (Answer(&SER_ScrapStream, &cfg) == SERMS_CONNECT)
                                NET_ModemOn = TRUE;
                        } else if (opt == 4) {
                            if (Hangup(&SER_ScrapStream, &cfg) == SERMS_OK)
                                NET_ModemOn = FALSE;
                        }
                        SER_EndComPort(&SER_ScrapStream.port);
                        break;

                    case SIZEARRAY(menu)-1:
                        ret = -1; leave = TRUE; break;
                }
                break;
            }
            case kESC:
                ret = -1; leave = TRUE;
                break;
        }
    } while (!leave);
    MENU_Return();
    if (ret == 0 || ret == 1) {
        int st;
        st = StartOrJoinMenu();
        if (st < 0)
            return -1;

            // Clear net structures from previous race or aborted menu.
        NET_NumNodes = 0;
        COM_EndGame();
        NET_State = NETS_NONE;
        NET_NumNodes = 0;
        memset(NET_NodeData, 0, sizeof(NET_NodeData));
        memset(NET_NodeNum,  0, sizeof(NET_NodeNum));
        if (ret == 0) {
            NET_State = NETS_IPX;
            if (!COM_InitIpxGame(3, 10000+CFG_Config.ipxsocket))
                BASE_Abort("Unable to initialize IPX interface");
        } else {
            NET_State = NETS_SERIAL;
            if (!COM_InitSerGame(CFG_Config.comnum-1, CFG_Config.comaddr, CFG_Config.comirq))
                BASE_Abort("Unable to initialize Serial interface");
        }
//        MENU_Return();  // Quit from main menu
        if (st == 0) {
//            MENU_Enter(MENU_MAIN);      // In case the user ESCs from racetype.
            MENU_Enter(MENU_RACETYPE);
        } else
            MENU_Return();      // Quit from main menu.
    }
    return ret;
}

// -----------------------

PUBLIC int NET_EditPhones(void) {
    static int opt = 0;     // So it stays from call to call.
    char *menu[6+1];
    int nitems = 1;
    char (*pnum)[CFG_STRSIZE];
    int i;

    for (pnum = &CFG_Config.phone1; pnum <= CFG_Config.phone6; pnum++) {
        menu[nitems-1] = pnum[0];
        nitems++;
    }
    menu[nitems-1] = "Exit";
    if (opt >= nitems)
        opt = 0;

    for (;;) {
        for (i = 0; i < nitems-1; i++) {
            if (menu[i][0] == '\0')
                strcpy(menu[i], "NONE");
        }
        switch (MENU_DoControlMenu(menu, nitems, &opt)) {
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                if (opt < nitems-1) {
                    char buf[20];
                    if (isdigit(menu[opt][0]))
                        strcpy(buf, menu[opt]);
                    else
                        buf[0] = '\0';
                    if (MENU_DoInput("Enter phone number", buf, sizeof(buf), "0123456789PT"))
                        strcpy(menu[opt], buf);
                } else {
                    MENU_Return();
                    return 0;
                }
                break;
            case kESC:
                MENU_Return();
                return 0;
        }
    }
}

// ------------------------------ NET.C ----------------------------
