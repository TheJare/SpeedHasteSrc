// ----------------------------- LLIPX.H ---------------------------
// For use with Watcom C.
// (C) Copyright 1995 by Jare & JCAB of Iguana.
// Based on IPXSTART by David Welch.

#ifndef _LLIPX_H_
#define _LLIPX_H_

#ifndef _BASE_H_
#include <base.h>
#endif

#pragma pack(1)

typedef struct {
    byte net[4];
    byte node[6];
    word socket;
} IPX_TAddress, *IPX_PAddress;

typedef struct {
    word            checksum;
    word            len;
    byte            tc;
    byte            type;
    IPX_TAddress    dest;
    IPX_TAddress    source;
} IPX_THeader, *IPX_PHeader;

typedef struct {
    dword   linkaddress;
    void   *esraddress;     // B careful, it's a real mode proc!
    byte    inuse;
    byte    cc;
    word    socket;
    byte    workspace[16];
    byte    immedaddr[6];
    word    fragcount;
    dword   fragaddr1;      // Pointer, Use DPMI_PTR2RFP
    word    fragsize1;
    dword   fragaddr2;      // Pointer, Use DPMI_PTR2RFP
    word    fragsize2;
} IPX_Tecb, *IPX_Pecb;

#pragma pack()

// ------------------------------------

PUBLIC word IPX_FlipWord(word a);
#pragma aux IPX_FlipWord parm [EAX] = "XCHG AL,AH"

// ------------------------------------

    // Returns ipxentry. if zero, ipx is not installed
PUBLIC dword IPX_Init(void);

PUBLIC void IPX_End(void);

// ------------------------------------

    // If b == NULL will return own memory area, else b.
PUBLIC IPX_PAddress IPX_GetAddress(IPX_PAddress b);

PUBLIC word IPX_GetMaxPacketSize(void);

    // first argument:
    //      socket longevity flag
    //          00h the socket will remain open until a Close Socket call or the
    //              application terminates
    //          01h the socket will remain open until a Close Socket call
    // second argument:
    //      address to the socket value (little-endian word)
    // returns:
    //      >= 0 Success
    //          the socket value will reflect the open socket (little-endian word)
    //      -1 Socket Already Open
    //      -2 Socket Table Is Full
PUBLIC int IPX_OpenSocket(byte longevity, word *sock);

PUBLIC void IPX_CloseSocket(word s);

    // returns >= 0 Success, -1 -> listening socket does not exist
PUBLIC int IPX_ListenForPacket(IPX_Tecb *ecb);

PUBLIC void IPX_SendPacket(IPX_Tecb *ecb);

PUBLIC void IPX_RelinquishControl(void);

// ------------------------------------
// Helper functions for allocating ECBs from DOS real mode memory.

    // Alloc an array of ECBs. Also can alloc some more space, usually
    // for data buffers and such.
PUBLIC IPX_Tecb *IPX_NewECB(int n, int extra);

PUBLIC void IPX_FreeECB(IPX_Tecb *ecb);

#define IPX_DisposeECB(ecb) {IPX_FreeECB((ecb)); (ecb) = NULL;}

// ----------------------------------------------
// Higher level stuff.

typedef struct {
    IPX_THeader h;
    IPX_Tecb    ecb;
    int         bufsize;
    byte        buf[];
} IPX_TPacket, *IPX_PPacket;

PUBLIC IPX_PPacket IPX_AllocPackets(int n, int bufsize);

PUBLIC void IPX_FreePackets(IPX_PPacket p);

#define IPX_DisposePackets(p) (IPX_FreePackets((p)),(void)((p)=NULL))

#define IPX_PACKET(p,n) ((volatile IPX_TPacket*)(((byte*)(p))+(n)*(sizeof(*p)+((p)->bufsize))))

    // Fill up appropiate fields. If src == NULL will use own memory area.
    // If dest == NULL, broadcast.
PUBLIC void IPX_InitOutPacket(IPX_PPacket p,
                              IPX_PAddress src, IPX_PAddress dest, int sock);

PUBLIC void IPX_InitInPacket(IPX_PPacket p, int sock);

#endif

// ----------------------------- LLIPX.H ---------------------------
