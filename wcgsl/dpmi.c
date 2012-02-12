/* -------------------------- DPMI.C ---------------------------- */
/* DPMI interface code for Watcom C 9.5 and DOS4GW                */
/* Written bye Jare of Iguana in 1994.                            */
/* -------------------------------------------------------------- */
/* Thanks to Yann for supplying all those specs, and also to Tran */
/* of nothing for getting me into this wonderful world of PMode.  */
/* -------------------------------------------------------------- */
/* This code will compile under Watcom C++ 9.5. I think it should */
/* work also with other versions and under other 32 bit compilers */
/* but I haven't bothered to test.                                */
/* -------------------------------------------------------------- */
/* You will find some general purpose DPMI code here. Use it!     */
/* You might need to translate it to assembly, but that should be */
/* easy. After all, it's your job! :)                             */
/* -------------------------------------------------------------- */

#include <stdlib.h>
#include <i86.h>
#include <dpmi.h>

 // -------------------------------------------------------

// ú JC Se necesita?   // Leave some real mode memory free. Just in case. :)
//extern unsigned __near __minreal = 100*1024;


DPMI_TRealModeInfo  DPMI_rmi;

static union  REGS  rmregs;
static struct SREGS rmsregs;

static void DPMI_RealModeCall(void) {
    DPMI_rmi.x.SS = DPMI_rmi.x.SP = 0;// DPMI host will provide its own stack.
    rmregs.h.bh  = 0;                 // Do not reset PIC or A20 gate.
    rmregs.w.cx  = 0;                 // 0 words of parameters on the stack.
    rmregs.x.edi = FP_OFF(&DPMI_rmi); // Real mode info.
    rmsregs.es   = FP_SEG(&DPMI_rmi);
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
}

void PUBLICFUNC DPMI_RealModeInt(int i) {
    rmregs.w.ax = 0x300;
    rmregs.h.bl = i;
    DPMI_RealModeCall();
}

void PUBLICFUNC DPMI_RealModeProc(void *proc, word seg) {
/*
    rmregs.w.ax = 0x301;
    DPMI_rmi.x.CS = seg;
    DPMI_rmi.x.IP = off;
    DPMI_RealModeCall();

    ********************************************************
    WARNING: DOS4GW v1.8 bundled with WATCOM C++ 9.5 doesn't
    seem to suuport the above call (at least I didn't get it
    to work), so we used a wierd scheme to emulate it.
    JCAB - 2/94
    DOS4GW 1.95 doesn't work either. What's going on?
    Jare - 3/94
    ********************************************************
*/

    dword oldvec4 = DPMI_REALVECTORS[4];
    dword oldvec6 = DPMI_REALVECTORS[6];
    dword oldvec7 = DPMI_REALVECTORS[7];
    word  ofs;

    if (seg == 0) {
        seg = DPMI_PTR_SEG(proc);
        ofs = DPMI_PTR_OFS(proc);
    } else
        ofs = DPMI_PTR_OFS_SEG(proc, seg);

//printf("Real mode call: %x:%04x  (p-mode: %x)\n", seg, ofs, proc);

       // None of these interrupts should occur during normal operation.
       // but the way to do would be to allocate Real Mode Memory and place
       // the call there. INT 4 still should be used. I hate this!
    DPMI_REALVECTORS[4] = 6*4;  // Address of INT6 vector.
    DPMI_REALVECTORS[6] =  (long)0x9AFB + ((long)ofs<<16);  // STI + CALL FAR
    DPMI_REALVECTORS[7] =  (long)seg +                      // [seg:off]
                          ((long)0xCF<<16);                 // + IRET   CF
    DPMI_RealModeInt(4);

    DPMI_REALVECTORS[4] = oldvec4;
    DPMI_REALVECTORS[6] = oldvec6;
    DPMI_REALVECTORS[7] = oldvec7;
}


 // -------------------------------------------------------

void * PUBLICFUNC DPMI_DOS_GetMem(dword size)
{
    DPMI_rmi.b.AH = 0x48;
    DPMI_rmi.w.BX = (size+15) >> 4;
    rmregs.w.ax = 0x300;
    rmregs.h.bl = 0x21;
    DPMI_RealModeCall();
    if (DPMI_rmi.x.flags & FLAGS_CARRY)
        return NULL;
    else
        return DPMI_MK_PTR(DPMI_rmi.w.AX, 0);
}

void PUBLICFUNC DPMI_DOS_FreeMem(void *p)
{
    DPMI_rmi.b.AH = 0x49;
    DPMI_rmi.w.ES = DPMI_PTR_SEG(p);
    rmregs.w.ax = 0x300;
    rmregs.h.bl = 0x21;
    DPMI_RealModeCall();
}

dword PUBLICFUNC DPMI_DOS_ResizeMem(void *p, dword size)
{
    DPMI_rmi.b.AH = 0x4A;
    DPMI_rmi.w.ES = DPMI_PTR_SEG(p);
    DPMI_rmi.w.BX = (size+15) >> 4;
    rmregs.w.ax = 0x300;
    rmregs.h.bl = 0x21;
    DPMI_RealModeCall();
    if (DPMI_rmi.x.flags & FLAGS_CARRY) {
        DPMI_rmi.b.AH = 0x4A;
        DPMI_rmi.w.ES = DPMI_PTR_SEG(p);
        size = (dword)DPMI_rmi.w.BX << 4;
        rmregs.w.ax = 0x300;
        rmregs.h.bl = 0x21;
        DPMI_RealModeCall();
    }
    return size;
}

dword PUBLICFUNC DPMI_DOS_MaxMem(void)
{
    DPMI_DOS_GetMem(0xFFFF0);
    return ((dword)DPMI_rmi.w.BX) << 4;
}

dword PUBLICFUNC DPMI_DOS_GetVector(int v)
{
    rmregs.x.eax = 0x0200;   /* DPMI get real mode vector */
    rmregs.h.bl  = v;
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
    return DPMI_MK_RFP(rmregs.w.cx, rmregs.w.dx);
}

void PUBLICFUNC DPMI_DOS_SetVector(int v, dword h)
{
    rmregs.x.eax = 0x0201;
    rmregs.h.bl  = v;
    /* CX:DX == real mode &handler */
    rmregs.w.cx = DPMI_RFP_SEG(h);
    rmregs.w.dx = DPMI_RFP_OFS(h);
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
}

extern DPMI_ExcHandlerFunc PUBLICFUNC DPMI_GetExceptionHandler(int v)
{
    rmregs.x.eax = 0x0202;   /* DPMI get exception handler vector */
    rmregs.h.bl  = v;
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
    return MK_FP(rmregs.w.cx, rmregs.x.edx);
}

extern void PUBLICFUNC DPMI_SetExceptionHandler(int v, DPMI_ExcHandlerFunc h)
{
    rmregs.x.eax = 0x0203;
    rmregs.h.bl  = v;
    /* CX:EDX == exception handler */
    rmregs.w.cx  = FP_SEG(h);
    rmregs.x.edx = FP_OFF(h);
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
}

extern dword PUBLICFUNC DPMI_NewCallback(DPMI_TCallbackFunc func)
{
    rmregs.x.eax = 0x0303;   /* DPMI Alloc Callback */
    rmregs.x.esi = FP_OFF(func);
    rmsregs.ds   = FP_SEG(func);
    rmregs.x.edi = FP_OFF(&DPMI_rmi); // Real mode info.
    rmsregs.es   = FP_SEG(&DPMI_rmi);
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
    return DPMI_MK_RFP(rmregs.w.cx, rmregs.w.dx);
}

void PUBLICFUNC DPMI_EndCallback(dword h)
{
    rmregs.x.eax = 0x0304;  /* DPMI free callback */
    /* CX:DX == real mode callback */
    rmregs.w.cx = DPMI_RFP_SEG(h);
    rmregs.w.dx = DPMI_RFP_OFS(h);
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
}

 // -------------------------------------------------------

void PUBLICFUNC DPMI_GetMemInfo(DPMI_PMemInfo p)
{
    rmsregs.es   = FP_SEG(p);
    rmregs.x.edi = FP_OFF(p);
    rmregs.x.eax = 0x00000500;
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
}

 // -------------------------------------------------------

word PUBLICFUNC DPMI_NewSelector(void)
{
    rmregs.x.eax = 0x0000;
    rmregs.x.ecx = 1;
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
    return rmregs.w.ax;
}

void PUBLICFUNC DPMI_FreeSelector(word sel)
{
    if (sel == 0)
        return;
    rmregs.x.eax = 0x0001;
    rmregs.x.ebx = sel;
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
}

dword PUBLICFUNC DPMI_GetBaseAddress(word sel)
{
    if (sel == 0)
        return 0;
    rmregs.x.eax = 0x0006;
    rmregs.w.bx  = sel;
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
    return DPMI_MK_RFP(rmregs.w.cx, rmregs.w.dx);
}

dword PUBLICFUNC DPMI_GetLimit(word sel)
{
extern dword lsl(dword sel);
#pragma aux  lsl modify nomemory parm [EAX] value [EAX] = \
    "LSL EAX,AX"    \
    "JZ @@l1"       \
    "XOR EAX,EAX"   \
    "@@l1:"

    if (sel == 0)
        return 0;
    return lsl(sel);
}

void PUBLICFUNC DPMI_SetBaseAddress(word sel, dword linbase)
{
    if (sel == 0)
        return;
    rmregs.x.eax = 0x0007;
    rmregs.w.bx  = sel;
    rmregs.w.cx  = DPMI_RFP_SEG(linbase);
    rmregs.w.dx  = DPMI_RFP_OFS(linbase);
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
}

void PUBLICFUNC DPMI_SetLimit(word sel, dword limit)
{
    if (sel == 0)
        return;
    rmregs.x.eax = 0x0008;
    rmregs.w.bx  = sel;
    if (limit >= (1 << 20))         // Must be made page-granular.
        limit |= (1 << 20) - 1;     // and therefore reach the end of the page.
    rmregs.w.cx  = DPMI_RFP_SEG(limit);
    rmregs.w.dx  = DPMI_RFP_OFS(limit);
    int386x(0x31, &rmregs, &rmregs, &rmsregs);
}



