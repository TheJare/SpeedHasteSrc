// ------------------------------ COMMS.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "comms.h"
#include <llipx.h>
#include <serial.h>

#include <string.h>
#include <stdio.h>

/*
    Packet description: ????????????

    byte - piggyback info (using rings MOD 4)
       and packet type: in-game, initialization, exiting, check, etc.
    byte - Player number, or 0xFF for player with unassigned number.

    packet data, n bytes.

    byte - simple checksum


    For the comm protocol, a simple bidirectional protocol with ordering
        and discarding should suffice.

*/

// -------------------------------------
/*
    Broadcasted packets are those that come from an address that is
    not registered as a link. Therefore, if there is no link for a
    serial game, anything that comes from there is treated as
    "broadcast". For IPX, the packet source address must be checked
    against addresses of the active links (and our own, since our
    broadcasted packets are received by us too).
*/

    // -------- Variables

    // --- Private

#define NUMINPACKETS 10

PRIVATE int         NumIPXPackets = 0;
PRIVATE IPX_PPacket IPXPackets = NULL;
PRIVATE IPX_TAddress IPXAddress;

PRIVATE SER_TStream SerChannel;

    // --- Public

    // Active and maximum number of links.
PUBLIC int  COM_NLinks = 0,
            COM_MaxLinks = 0;

    // Array of "MaxLinks" links.
PUBLIC COM_PLink COM_Links = NULL;

    // One of COMT_xxx
PUBLIC int COM_Type = COMT_NONE;

PUBLIC bool COM_IPXPresent = FALSE;

    // Own address.
PUBLIC COM_TAddress COM_MyAddress = {
    {0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF},
};

    // -------- Functions

PRIVATE void __interrupt __far __loadds SerISR(void) {
    SER_StreamHandle(&SerChannel);
}

PRIVATE byte CalcChecksum(COM_PPacket p) {
    byte *s, c;
    int i;

    s = (byte*)p;
//    c = s[0] + s[1] + s[2] + s[3] + p->len;
    c = 0;
    for (i = 6; i < p->len; i++)
        c += s[i]+0x37;
    return c;
}

    // Init the global thing, get machine address, IPX existance, etc.
PUBLIC bool COM_Init(void) {
//    printf("Checking network interface:\n");
    COM_IPXPresent = (IPX_Init() != 0);
    if (COM_IPXPresent) {
        IPX_PAddress p = IPX_GetAddress(&IPXAddress);
        if (p != NULL) {
            memcpy(COM_MyAddress.addr, p->node, 6);
/*
            printf("  IPX Detected, local address is %02X:%02X:%02X:%02X:%02X:%02X\n",
                p->node[0],p->node[1],p->node[2],p->node[3],p->node[4],p->node[5]);
*/
        }
    } else {

    }
    return TRUE;
}

    // Init structures for an IPX game.
PUBLIC bool COM_InitIpxGame(int maxlink, int socket) {
    int i;
    if (COM_Links != NULL || !COM_IPXPresent || maxlink <= 0)
        return FALSE;

    COM_Type = COMT_IPX;
        // Init links.
    COM_Links = NEW(maxlink*sizeof(*COM_Links));
    if (COM_Links == NULL)
        return FALSE;
    memset(COM_Links, 0, maxlink*sizeof(*COM_Links));
    COM_MaxLinks = maxlink;
    COM_NLinks = 0;

        // Init packets.
    NumIPXPackets = NUMINPACKETS + maxlink + 1;
    IPXPackets = IPX_AllocPackets(NumIPXPackets, MAXPACKETSIZE);
    if (IPXPackets == NULL) {
        DISPOSE(COM_Links);
        return FALSE;
    }
        // Assign out packets to links.
    for (i = 0; i < maxlink; i++) {
        COM_Links[i].ipx = IPX_PACKET(IPXPackets, NUMINPACKETS + i);
    }
        // Open socket.
    IPXAddress.socket = IPX_FlipWord(socket);
    IPX_OpenSocket(0, &IPXAddress.socket);

        // Init in packets.
    for (i = 0; i < NUMINPACKETS; i++) {
            // Setup receive socket.
        IPX_InitInPacket(IPX_PACKET(IPXPackets,i), IPXAddress.socket);
            // And listen on it.
        IPX_ListenForPacket(&IPX_PACKET(IPXPackets,i)->ecb);
    }
    return TRUE;
}

    // Init structures for an serial/modem game.
PUBLIC bool COM_InitSerGame(int com, int port, int irq){
    if (COM_Links != NULL)
        return FALSE;
    COM_Type = COMT_SERIAL;
        // Init links.
    COM_Links = NEW(1*sizeof(*COM_Links));
    if (COM_Links == NULL)
        return FALSE;
    memset(COM_Links, 0, 1*sizeof(*COM_Links));
    COM_MaxLinks = 1;
    COM_NLinks = 0;

        // Open stream.
    SER_InitComInfo(&SerChannel.port, com, port, irq);
    if (SER_InitComPort(&SerChannel.port)) {
/*
        printf("Com %d, port 0x%X, IRQ %d, UART type %s\n",
                SerChannel.port.com+1, SerChannel.port.port, SerChannel.port.irq,
                (SerChannel.port.flags & SERPF_IS16550)? "16550" : "8250");
*/
        SER_InitStream(&SerChannel, &SerISR);
    } else {
        //printf("Failed to initialize com port %d\n", SerChannel.port.com+1);
        return FALSE;
    }

    return TRUE;
}

    // Add a link to the specified machine address (IPX address, or anything
    // for serial game). Return number of link. Link 0 is a broadcast, so
    // link numbers go from 1 to maxlink.
PUBLIC int  COM_AddLink(COM_PAddress addr) {
    int i;
    if (addr == NULL || COM_Links == NULL)
        return -1;
    for (i = 0; i < COM_MaxLinks; i++) {
        if (COM_Links[i].type == COM_Type) {    // Free link.
            if (memcmp(COM_Links[i].addr.addr,
                       addr->addr, sizeof(addr->addr)) == 0)
                return i + 1;
        }
    }
    for (i = 0; i < COM_MaxLinks; i++) {
        if (COM_Links[i].type == COMT_NONE) {    // Free link.
            COM_Links[i].type = COM_Type;
            memcpy(COM_Links[i].addr.addr, addr->addr, sizeof(addr->addr));
            if (COM_Type == COMT_IPX) {
                memcpy(COM_Links[i].ipxadr.node, addr->addr, sizeof(addr->addr));
                COM_Links[i].ipxadr.socket = IPXAddress.socket;
            }
            COM_Links[i].nrec  = 0;
            COM_Links[i].nsend = 0;
            COM_NLinks++;
            break;
        }
    }
    if (i >= COM_MaxLinks)
        return -1;
    return i+1;
}

    // End the given link.
PUBLIC void COM_EndLink(int nlink) {
    if (COM_Links == NULL || nlink <= 0 || nlink > COM_MaxLinks
     || COM_Links[nlink-1].type == COMT_NONE)
        return;
    COM_Links[nlink-1].type = COMT_NONE;
    COM_NLinks--;
}

    // Check for incoming packets, and if there is any, copy it and
    // return number of link it came from (0 for a broadcasted packet).
PUBLIC int  COM_GetPacket(COM_PPacket p) {
    int i, j;
    if (COM_Links == NULL || p == NULL)
        return -1;
    if (COM_Type == COMT_IPX) {
        while (TRUE) {
            IPX_PPacket q = NULL;
            dword mintime = 0xFFFFFFFFUL;
                // Search for the most ancient packet.
            for (i = 0; i < NUMINPACKETS; i++)
                if (!IPX_PACKET(IPXPackets,i)->ecb.inuse) {
                    if (((COM_PPacket)IPX_PACKET(IPXPackets,i)->buf)->time < mintime) {
                        mintime = ((COM_PPacket)(IPX_PACKET(IPXPackets,i)->buf))->time;
                        q = IPX_PACKET(IPXPackets,i);
                    }
                }
                // Was there any inbound packet?
            if (q == NULL)
                return -1;
                // Does it come from ourselves?
            if (memcmp(IPXAddress.node, q->h.source.node, sizeof(IPXAddress.node)) == 0) {
                    // Discard it and search again.
                IPX_ListenForPacket(&q->ecb);
                continue;
            }
                // Accept it
            memcpy(p, q->buf, sizeof(*p));
                // Search source address in active links.
            for (j = 0; j < COM_MaxLinks; j++) {
                if (COM_Links[j].type == COM_Type
                 && memcmp(COM_Links[j].addr.addr, q->h.source.node,
                           sizeof(COM_Links[j].addr.addr)) == 0)
                    break;
            }
            IPX_ListenForPacket(&q->ecb);
            if (j >= COM_MaxLinks) return 0;   // Unkown source.
            else                   return j + 1;
        }
    } else if (COM_Type == COMT_SERIAL) {
        if (SER_ReadBlock(&SerChannel, (void*)&p->len, sizeof(*p)-sizeof(p->time)) <= 0)
//        if (SER_ReadBlock(&SerChannel, (void*)&p->len, p->len-sizeof(p->time)) <= 0)
//        if (SER_ReadBlock(&SerChannel, (void*)&p->time, p->len) <= 0)
            return -1;
        if (CalcChecksum(p) != p->checksum)
            return -2;
//        p->time = COM_Links[0].nrec++;
        return COM_NLinks;
    }
    return -1;
}

    // Send packet to specified link (0 for broadcast).
PUBLIC bool COM_SendPacket(int nlink, COM_PPacket p) {
    if (COM_Links == NULL || p == NULL || nlink < 0 || nlink > COM_MaxLinks)
        return FALSE;
    if (nlink > 0 && COM_Links[nlink-1].type == COMT_NONE)
        return FALSE;
    if (COM_Type == COMT_IPX) {
        IPX_PPacket q;
        if (nlink == 0) {
            q = IPX_PACKET(IPXPackets, NumIPXPackets-1);
            while (IPX_PACKET(q,0)->ecb.inuse);
            IPX_InitOutPacket(q, NULL, NULL, IPXAddress.socket);
        } else {
            q = COM_Links[nlink-1].ipx;
            while (IPX_PACKET(q,0)->ecb.inuse);
            IPX_InitOutPacket(q, NULL, &COM_Links[nlink-1].ipxadr, IPXAddress.socket);
//            p->time = COM_Links[nlink-1].nsend++;
        }
        memcpy(q->buf, p, p->len);
        IPX_SendPacket(&IPX_PACKET(q,0)->ecb);
    } else if (COM_Type == COMT_SERIAL) {
        p->checksum = CalcChecksum(p);
        SER_WriteBlock(&SerChannel, (void*)&p->len, p->len-sizeof(p->time));
//        SER_WriteBlock(&SerChannel, (void*)&p->time, p->len);
//        if (p->time &1)
            SER_StreamTransmit(&SerChannel);
    }
    return TRUE;
}

    // In case the layer needs some spare time sometimes. IPX does.
PUBLIC void COM_Housekeep(void) {
    if (COM_Type == COMT_IPX)
        IPX_RelinquishControl();
}

    // End current game structures.
PUBLIC void COM_EndGame(void) {
    if (COM_Links == NULL)
        return;
    if (COM_Type == COMT_IPX) {
        IPX_CloseSocket(IPXAddress.socket);
        IPX_DisposePackets(IPXPackets);
    } else if (COM_Type == COMT_SERIAL) {
        SER_EndComPort(&SerChannel.port);
    }
    DISPOSE(COM_Links);
    COM_MaxLinks = COM_NLinks = 0;
    COM_Type = COMT_NONE;
}

    // End the module.
PUBLIC void COM_End(void) {
    COM_EndGame();
    if (COM_IPXPresent)
        IPX_End();
    memset(COM_MyAddress.addr, 0xFF, sizeof(COM_MyAddress.addr));
}


// ------------------------------ COMMS.H ----------------------------
