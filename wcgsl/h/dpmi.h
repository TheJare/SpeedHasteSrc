/* -------------------------- DPMI.H ---------------------------- */
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

#ifndef _DPMI_H_
#define _DPMI_H_
                                             
#include <base.h>




// ------------------------------------------------------------------------
// ---------- DPMI DOS interface definitions.

#define DPMI_REALVECTORS ((dword *)0)   // Real mode vector table.
                                                                  
typedef struct DPMI_SXrminfo {      // Real mode registers for the DPMI
    dword EDI;                      // translation API.
    dword ESI;                      // 32 bit registers.
    dword EBP;
    dword _reserved;
    dword EBX;
    dword EDX;
    dword ECX;
    dword EAX;
    word  flags;
    word  ES, DS, FS, GS;
    word  IP, CS, SP, SS;
} DPMI_TXRealModeInfo;

typedef struct DPMI_SWrminfo {      // Real mode registers for the DPMI
    word DI, HDI;                   // translation API.
    word SI, HSI;                   // 16 bit registers.
    word BP, HBP;
    dword _reserved;
    word BX, HBX;
    word DX, HDX;
    word CX, HCX;
    word AX, HAX;
    word flags;
    word ES, DS, FS, GS;
    word IP, CS, SP, SS;
} DPMI_TWRealModeInfo;

typedef struct DPMI_SBrminfo {      // Real mode registers for the DPMI
    word DI, HDI;                   // translation API.
    word SI, HSI;                   // 8 bit registers
    word BP, HBP;
    dword _reserved;
    byte BL, BH, HBL, HBH;
    byte DL, DH, HDL, HDH;
    byte CL, CH, HCL, HCH;
    byte AL, AH, HAL, HAH;
    byte flagsl, flagsh;
    word ES, DS, FS, GS;
    word IP, CS, SP, SS;
} DPMI_TBRealModeInfo;

typedef union DPMI_Urminfo {       // Real mode registers for the DPMI
    // translation API.
    DPMI_TBRealModeInfo b;
    DPMI_TWRealModeInfo w;
    DPMI_TXRealModeInfo x;
} DPMI_TRealModeInfo;

enum {
    FLAGS_CARRY  = 0x0001,         // To check flags above.
    FLAGS_PARITY = 0x0004,
    FLAGS_AUX    = 0x0010,
    FLAGS_ZERO   = 0x0040,
    FLAGS_SIGN   = 0x0080,
    FLAGS_TRACE  = 0x0100,
    FLAGS_INTR   = 0x0200,
    FLAGS_DIRECC = 0x0400,
    FLAGS_OVERF  = 0x0800
};

#define DPMI_MK_RFP(s,o) (((dword)(s) << 16) + (word)(o))
#define DPMI_MK_PTR(s,o) ((void *)(((dword)(s) << 4) + (word)(o)))

#define DPMI_RFP_SEG(p)  ((word)((dword)(p) >> 16))
#define DPMI_RFP_OFS(p)  ((word)(p))

#define DPMI_PTR_SEG(p)  ((word)(((dword)(p)) >> 4))
#define DPMI_PTR_OFS(p)  (((word)(p)) & 0x000F)

#define DPMI_PTR2RFP(p)  (((((dword)(p)&0xFFFF0))    << 12) + ((word)(p)&0xF))
#define DPMI_RFP2PTR(p)  ((void *)(((((dword)(p)&0xFFFF0000)) >> 12) + (word)(p)))

#define DPMI_PTR_OFS_SEG(p,s)  (((dword)(p))-((dword)(s)<<4))
#define DPMI_PTR2RFP_SEG(p,s) ((((dword)(p))-((dword)(s)<<4))+((dword)(s)<<16))


// ------------------------------------------------------------------------
// ---------- DPMI interface definitions.

typedef struct DPMI_SMemInfo {
    unsigned LargestBlockAvail;
    unsigned MaxUnlockedPage;
    unsigned LargestLockablePage;
    unsigned LinAddrSpace;
    unsigned NumFreePagesAvail;
    unsigned NumPhysicalPagesFree;
    unsigned TotalPhysicalPages;
    unsigned FreeLinAddrSpace;
    unsigned SizeOfPageFile;
    unsigned Reserved[3];
} DPMI_TMemInfo, *DPMI_PMemInfo;


// ------------------------------------------------------------------------
// ---------- DPMI DOS interface data.

extern DPMI_TRealModeInfo DPMI_rmi; // Place real mode register values
                                    // here when calling the next two
                                    // functions.


// ------------------------------------------------------------------------
// ---------- DPMI DOS interface routines.

extern void   PUBLICFUNC DPMI_RealModeInt(int i);
  //                    ----------------
  // Calls a real mode interrupt. For example, DOS' INT 2Fh.


extern void   PUBLICFUNC DPMI_RealModeProc(void *proc, word seg);
  //                    -----------------
  // Calls a real mode procedure.


extern byte * PUBLICFUNC DPMI_MapMemory(dword baseaddr, dword len);
  //                     --------------
  // Maps a linear memory area into a far pointer. Remember that this
  // uses up the descriptor that the DPMI host provides, so you
  // should plan carefully when to call this function, and most of
  // the time you will rather mess with the pointers (and selectors)
  // obtained here.

extern void * PUBLICFUNC DPMI_DOS_GetMem(dword size);
  //                     ---------------
  // Allocates DOS memory.

extern void   PUBLICFUNC DPMI_DOS_FreeMem(void *p);
  //                    ----------------
  // Frees DOS memory.

extern dword  PUBLICFUNC DPMI_DOS_ResizeMem(void *p, dword size);
  //                    ------------------
  // Resizes a previously allocated DOS memory block.
  // Returns the size of the new block.

extern dword  PUBLICFUNC DPMI_DOS_MaxMem(void);
  //                    ---------------
  // Returns size of biggest free DOS memory block.

extern dword  PUBLICFUNC DPMI_DOS_GetVector(int v);
  //                    ------------------
  // Returns the real pointer to the interrupt handler.

extern void   PUBLICFUNC DPMI_DOS_SetVector(int v, dword h);
  //                    ---------------
  // Changes the real pointer of the interrupt handler.

// ------------------------------------------------------------------------
// ---------- DPMI exception handler routines.

typedef void (__far * DPMI_ExcHandlerFunc)(void);

extern DPMI_ExcHandlerFunc PUBLICFUNC DPMI_GetExceptionHandler(int v);

extern void PUBLICFUNC DPMI_SetExceptionHandler(int v, DPMI_ExcHandlerFunc h);

// ------------------------------------------------------------------------
// ---------- DPMI memory interface routines.

extern void   PUBLICFUNC  DPMI_GetMemInfo(DPMI_PMemInfo p);
//                       ---------------
// Retrieves memory info from the DPMI server.

// ------------------------------------------------------------------------
// ---------- DPMI selector handling routines.

extern word   PUBLICFUNC DPMI_NewSelector(void);

extern void   PUBLICFUNC DPMI_FreeSelector(word sel);

extern dword  PUBLICFUNC DPMI_GetBaseAddress(word sel);

extern dword  PUBLICFUNC DPMI_GetLimit(word sel);

extern void   PUBLICFUNC DPMI_SetBaseAddress(word sel, dword linbase);

extern void   PUBLICFUNC DPMI_SetLimit(word sel, dword limit);

// ------------------------------------------------------------------------
// ---------- DPMI interrupt interface routines.

typedef void (__far *DPMI_TCallbackFunc)(void);

extern dword PUBLICFUNC DPMI_NewCallback(DPMI_TCallbackFunc func);

extern void  PUBLICFUNC DPMI_EndCallback(dword h);


extern void   far *DPMI_InterruptFix(void);
//                 -----------------
// Does some fixups with stack and segments, at the beginning of an
// interrupt handler routine. Returns a far pointer that must be saved.

extern void        DPMI_InterruptRestore(void far *);
//                 ---------------------
// Restores the state that was present when DPMI_FixStack was called.

#pragma aux DPMI_InterruptRestore parm [DX EAX]




#endif

