// ----------------------------- LLIPX.C ---------------------------
// For use with Watcom C.
// (C) Copyright 1995 by Jare & JCAB of Iguana.
// Based on IPXSTART by David Welch.

#include <llipx.h>
#include <string.h>
#include <dpmi.h>

PRIVATE dword IPX_Entry = NULL;
PRIVATE IPX_PAddress IPX_RealModeMem = NULL;

    // Returns ipxentry. if zero, ipx is not installed
PUBLIC dword IPX_Init(void) {
    if (IPX_Entry != 0)
        return IPX_Entry;
    DPMI_rmi.w.AX = 0x7A00;
    DPMI_RealModeInt(0x2F);
    IPX_Entry = DPMI_MK_RFP(DPMI_rmi.w.ES,DPMI_rmi.w.DI);
    if (DPMI_rmi.b.AL != 0xFF || IPX_Entry == NULL)
        return 0;
    IPX_RealModeMem = DPMI_DOS_GetMem(sizeof(*IPX_RealModeMem));
    if (IPX_RealModeMem == NULL)
        return 0;
    return IPX_Entry;
}

PUBLIC void IPX_End(void) {
    if (IPX_RealModeMem == NULL)
        return;
    DPMI_DOS_FreeMem(IPX_RealModeMem);
    IPX_RealModeMem = NULL;
    IPX_Entry = 0;
}

PRIVATE volatile int InIPXCall = 0;

PRIVATE void CallIPX(void) {
    InIPXCall++;
    DPMI_RealModeProc(DPMI_RFP2PTR(IPX_Entry), DPMI_RFP_SEG(IPX_Entry));
    InIPXCall--;
}

PUBLIC IPX_PAddress IPX_GetAddress(IPX_PAddress b) {
    if (IPX_Entry == 0)
        return NULL;
    memset(IPX_RealModeMem, -1, sizeof(*IPX_RealModeMem));
    DPMI_rmi.w.BX = 0x0009;
    DPMI_rmi.w.ES = DPMI_PTR_SEG(IPX_RealModeMem);
    DPMI_rmi.w.SI = DPMI_PTR_OFS(IPX_RealModeMem);
    CallIPX();
    if (b != NULL) {
        memcpy(b, IPX_RealModeMem, sizeof(*b));
        return b;
    }
    return IPX_RealModeMem;
}

PUBLIC word IPX_GetMaxPacketSize(void) {
    if (IPX_Entry == 0)
        return 0;
    DPMI_rmi.w.BX = 0x000D; // 1A?? Both are internal.
    CallIPX();
    return DPMI_rmi.w.AX;
}

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
PUBLIC int IPX_OpenSocket(byte longevity, word *sock) {
    if (IPX_Entry == 0 || sock == NULL)
        return -3;
    DPMI_rmi.b.AL = longevity;
    DPMI_rmi.w.DX = *sock;
    DPMI_rmi.w.BX = 0x0000;
    CallIPX();
    if (DPMI_rmi.b.AL != 0)
        return (sint8)DPMI_rmi.b.AL;
    *sock = DPMI_rmi.w.DX;
    return 0;
}

PUBLIC void IPX_CloseSocket(word s) {
    if (IPX_Entry == 0)
        return;
    DPMI_rmi.w.DX = s;
    DPMI_rmi.w.BX = 0x0001;
    CallIPX();
}

    // returns >= 0 Success, -1 -> listening socket does not exist
PUBLIC int IPX_ListenForPacket(IPX_Tecb *ecb) {
    if (IPX_Entry == 0 || ecb == NULL || ((dword)ecb) >= 0x100000)
        return -2;
    while (InIPXCall > 0);
    DPMI_rmi.w.ES = DPMI_PTR_SEG(ecb);
    DPMI_rmi.w.SI = DPMI_PTR_OFS(ecb);
    DPMI_rmi.w.BX = 0x0004;
    CallIPX();
    return (sint8)DPMI_rmi.b.AL;
}

PUBLIC void IPX_SendPacket(IPX_Tecb *ecb) {
    if (IPX_Entry == 0 || ecb == NULL || ((dword)ecb) >= 0x100000)
        return;
    while (InIPXCall > 0);
    DPMI_rmi.w.ES = DPMI_PTR_SEG(ecb);
    DPMI_rmi.w.SI = DPMI_PTR_OFS(ecb);
    DPMI_rmi.w.BX = 0x0003;
    CallIPX();
}

PUBLIC void IPX_RelinquishControl(void) {
    if (IPX_Entry == 0 || InIPXCall > 0)
        return;
    DPMI_rmi.w.BX = 0x000A;
    CallIPX();
}

// ------------------------------------
// Helper functions for allocating ECBs from DOS real mode memory.

PUBLIC IPX_Tecb *IPX_NewECB(int n, int extra) {
    IPX_Pecb ecb;

    if (n < 0 || extra < 0)
        return NULL;
    if (n == 0 && extra == 0)
        return NULL;
    ecb = DPMI_DOS_GetMem(n*sizeof(*ecb)+extra);
    return ecb;
}

PUBLIC void IPX_FreeECB(IPX_Tecb *ecb) {
    if (ecb == NULL || ((dword)ecb) >= 0x100000)
        return;
    DPMI_DOS_FreeMem(ecb);
}

// ----------------------------------------------
// Higher level stuff.

PUBLIC IPX_PPacket IPX_AllocPackets(int n, int bufsize) {
    IPX_PPacket p;
    int i;

/*
    if (sizeof(IPX_TPacket) != (sizeof(IPX_THeader)+sizeof(IPX_Tecb)+sizeof(int)))
        BASE_Abort("%d != %d+%d+%d", sizeof(IPX_TPacket), sizeof(IPX_THeader),sizeof(IPX_Tecb),sizeof(int));
*/
    if (n <= 0 || bufsize <= 0)
        return NULL;
    p = DPMI_DOS_GetMem(n*(sizeof(*p)+bufsize));
    if (p == NULL)
        return NULL;
    memset(p, 0, n*(sizeof(*p)+bufsize));
    p->bufsize = bufsize;   // Init first so that IPX_PACKET macro works.
    for (i = 1; i < n; i++)
        IPX_PACKET(p,i)->bufsize = bufsize;
    return p;
}

PUBLIC void IPX_FreePackets(IPX_PPacket p) {
    if (p == NULL || ((dword)p) >= 0x100000)
        return;
    DPMI_DOS_FreeMem(p);
}

    // Fill up appropiate fields. If src == NULL will use own memory area.
void IPX_InitOutPacket(IPX_PPacket p,
                       IPX_PAddress src, IPX_PAddress dest, int sock) {
    int i;
    if (p == NULL)
        return;

    if (src == NULL)
        src = IPX_RealModeMem;

    if (dest == NULL) {
        for (i = 0; i < 4; i++)
            p->h.dest.net[i] = 0x00;
        for (i = 0; i < 6; i++) {
            p->h.dest.node[i]   = 0xFF;
            p->ecb.immedaddr[i] = 0xFF;
        }
    } else {
        for (i = 0; i < 4; i++)
            p->h.dest.net[i] = dest->net[i];
        for (i = 0; i < 6; i++) {
            p->h.dest.node[i]   = dest->node[i];
            p->ecb.immedaddr[i] = dest->node[i];
        }
    }
    for (i = 0; i < 4; i++)
        p->h.source.net[i] = src->net[i];
    for (i = 0; i < 6; i++)
        p->h.source.node[i] = dest->node[i];

    p->h.dest.socket = sock;
    p->h.type = 4;

    p->ecb.socket = sock;
    p->ecb.esraddress = NULL;
    p->ecb.fragcount = 2;
    p->ecb.fragaddr1 = DPMI_PTR2RFP(&p->h);
    p->ecb.fragsize1 = sizeof(p->h);
    p->ecb.fragaddr2 = DPMI_PTR2RFP(&p->buf);
    p->ecb.fragsize2 = p->bufsize;
    p->ecb.inuse = 0;
}

void IPX_InitInPacket(IPX_PPacket p, int sock) {
    p->ecb.socket = sock;
    p->ecb.esraddress = NULL;
    p->ecb.fragcount = 2;
    p->ecb.fragaddr1 = DPMI_PTR2RFP(&p->h);
    p->ecb.fragsize1 = sizeof(p->h);
    p->ecb.fragaddr2 = DPMI_PTR2RFP(&p->buf);
    p->ecb.fragsize2 = p->bufsize;
}


// ----------------------------- LLIPX.C ---------------------------
