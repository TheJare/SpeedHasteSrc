// ------------------------------ COMMS.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

// Comunications module.

#ifndef _COMMS_H_
#define _COMMS_H_

#include <base.h>
#include <serial.h>
#include <llipx.h>

#define RINGSIZE        1
#define MAXPACKETSIZE  32

#pragma pack(1)

typedef struct {
    byte addr[6];
} COM_TAddress, *COM_PAddress;

typedef struct {
    dword time;
    byte  len;
    byte  checksum;
} COM_TPacketHeader, *COM_PPacketHeader;

typedef struct {
    COM_TPacketHeader;
    byte  data[MAXPACKETSIZE-sizeof(COM_TPacketHeader)];
} COM_TPacket, *COM_PPacket;

#pragma pack()

typedef struct {
    COM_TAddress addr;
    byte type;                  // One of COMT_xxxx
    union {
        SER_PComPort serial;
        struct {
            IPX_PPacket  ipx;
            IPX_TAddress ipxadr;
        };
    };
    dword flags;                        // COMLF_xxxx
    dword nrec;                         // Last packet number received.
    dword nsend;                        // Number of packet to send.
    COM_TPacket spackets[RINGSIZE];     // Space for sent packets.
//    COM_TPacket rpacket;
} COM_TLink, *COM_PLink;

enum {
    COMT_NONE,          // Link is uninitialized.
    COMT_IPX,           // Link is IPX-type link.
    COMT_SERIAL,        // Link is serial-type link.

    COMLF_PIGGYBACK,    // Enable packet number checking and discarding.
};

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

    // Active and maximum number of links.
PUBLIC int  COM_NLinks,
            COM_MaxLinks;

    // Array of "MaxLinks" links.
PUBLIC COM_PLink COM_Links;

PUBLIC int COM_Type;            // One of COMT_xxx

PUBLIC bool COM_IPXPresent;

    // Own address.
PUBLIC COM_TAddress COM_MyAddress;

    // -------- Functions

    // Init the global thing, get machine address, IPX existance, etc.
PUBLIC bool COM_Init(void);

    // Init structures for an IPX game.
PUBLIC bool COM_InitIpxGame(int maxlink, int socket);

    // Init structures for an serial/modem game.
PUBLIC bool COM_InitSerGame(int com, int port, int irq);

    // Add a link to the specified machine address (IPX address, or anything
    // for serial game). Return number of link. Link 0 is a broadcast, so
    // link numbers go from 1 to maxlink.
PUBLIC int  COM_AddLink(COM_PAddress addr);

    // End the given link.
PUBLIC void COM_EndLink(int nlink);

    // Check for incoming packets, and if there is any, copy it and
    // return number of link it came from (0 for a broadcasted packet).
PUBLIC int  COM_GetPacket(COM_PPacket p);

    // Send packet to specified link (0 for broadcast).
PUBLIC bool COM_SendPacket(int nlink, COM_PPacket p);

    // In case the layer needs some spare time sometimes. IPX does.
PUBLIC void COM_Housekeep(void);

    // End current game structures.
PUBLIC void COM_EndGame(void);

    // End the module.
PUBLIC void COM_End(void);

#endif

// ------------------------------ COMMS.H ----------------------------
