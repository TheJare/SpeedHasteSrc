// --------------------------- BASE.H ------------------------------
// For use with with WATCOM 9.5 + DOS4GW
// (C) Copyright 1993/4 by Jare & JCAB of Iguana-VangeliSTeam.

/*
 * Base definitions and expected things like SREGS, NULL, etc.
 */

#ifndef _BASE_H_
#define _BASE_H_

#include <i86.h>
#include <stdlib.h>
#include <string.h>

#pragma library (local);


typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;
typedef   signed char  sbyte;
typedef   signed short sword;
typedef   signed long  sdword;

typedef unsigned char  uint8;
typedef   signed char  sint8;
typedef unsigned short uint16;
typedef   signed short sint16;
typedef unsigned long  uint32;
typedef   signed long  sint32;

typedef uint8 bool;

typedef signed   int   sint;
typedef unsigned int   uint;
typedef signed   long  slong;
typedef unsigned long  ulong;

#ifdef TRUE
#undef TRUE
#undef FALSE
#endif

enum {
    TRUE  = 1,
    FALSE = 0
};

#ifndef NULL
#define NULL (0L)
#endif

#define PUBLIC  extern
#define PRIVATE static

#define PUBLICFUNC __stdcall    // Defines a library function.
#define PUBLICDATA __stdcall    // Defines a library variable.

//#define PUBLICFUNC
#define LOCALFUNC

#define FAR

#ifndef swap
#define swap(a,b) ((a)^=(b),(b)^=(a),(a)^=(b))
#endif

#define swap16(w) ((w) = ((w) >> 8) | ((w) << 8))

extern unsigned inp(unsigned __port);
extern unsigned inpw(unsigned __port);
extern unsigned outp(unsigned __port, unsigned __value);
extern unsigned outpw(unsigned __port,unsigned __value);

#define inb(a)    inp(a)
#define outb(a,b) outp(a,b)
#define inw(a)    inpw(a)
#define outw(a,b) outpw(a,b)

#pragma intrinsic(inp,inpw,outp,outpw)

PUBLIC struct SREGS sregs;
PUBLIC union REGS inregs, outregs;

PUBLIC int    ArgC;
PUBLIC char **ArgV;

#define NEW(a)     ((void*)(((a)>0)?malloc((a)):NULL))
#define DISPOSE(a) (((a)!=NULL)?(void)(free((a)),(a)=NULL):(void)0)

PUBLIC void BASE_Require(const char *, const char *file, int line);
PUBLIC void BASE_Abort(const char *str, ...);

PUBLIC int BASE_CheckArg(const char *parm);

#ifndef NDEBUG
void breakpoint(void);
#pragma aux breakpoint = "INT 3";
#define REQUIRE(a) ((a)?(void)0:(void)(breakpoint(),BASE_Require("\n" #a "\n", __FILE__, __LINE__)))
#define assert(a) REQUIRE(a)
#else
void breakpoint(void);
#pragma aux breakpoint = "INT 3";
#define REQUIRE(a) ((a)?(void)0:(void)(breakpoint(),BASE_Require("", __FILE__, __LINE__)))
//#define breakpoint()
#define assert(a)
#endif

#define SIZEARRAY(a) (sizeof(a)/sizeof(*(a)))

extern  sint32 FP0Div(sint32 a, sint32 b);
#define FP0Div(a,b) ((a)/(b))
extern  sint32 FP0Mult(sint32 a, sint32 b);
#define FP0Mult(a,b) ((a)*(b))

extern sint32 FP8Div(sint32 a, sint32 b);
#pragma aux   FP8Div modify nomemory [EDX] parm [EAX] [ECX] value [EAX] = \
                    "MOV    EDX,EAX" \
                    "SAR    EDX,31"  \
                    "SHLD EDX,EAX,8" \
                    "SHL  EAX,8"     \
                    "IDIV ECX"

extern sint32 FP8Mult(sint32 a, sint32 b);
#pragma aux   FP8Mult modify nomemory parm [EAX] [EDX] value [EAX] = \
                    "IMUL EDX"        \
                    "SHRD EAX,EDX,8"

extern sint32 FP16Div(sint32 a, sint32 b);
#pragma aux   FP16Div modify nomemory [EDX] parm [EAX] [ECX] value [EAX] = \
                    "MOV    EDX,EAX" \
                    "SAR    EDX,31"  \
                    "SHLD EDX,EAX,16" \
                    "SHL  EAX,16"     \
                    "IDIV ECX"

extern sint32 FP16Mult(sint32 a, sint32 b);
#pragma aux   FP16Mult modify nomemory parm [EAX] [EDX] value [EAX] = \
                    "IMUL EDX"        \
                    "SHRD EAX,EDX,16"

extern sint32 FP24Div(sint32 a, sint32 b);
#pragma aux   FP24Div modify nomemory [EDX] parm [EAX] [ECX] value [EAX] = \
                    "MOV    EDX,EAX" \
                    "SAR    EDX,31"  \
                    "SHLD EDX,EAX,24" \
                    "SHL  EAX,24"     \
                    "IDIV ECX"

extern sint32 FP24Mult(sint32 a, sint32 b);
#pragma aux   FP24Mult modify nomemory parm [EAX] [EDX] value [EAX] = \
                    "IMUL EDX"        \
                    "SHRD EAX,EDX,24"

extern sint32 FP32Div(sint32 a, sint32 b);
#pragma aux   FP32Div modify nomemory [EAX] parm [EDX] [ECX] value [EAX] = \
                    "XOR  EAX,EAX"    \
                    "IDIV ECX"

extern sint32 FP32Mult(sint32 a, sint32 b);
#pragma aux   FP32Mult modify nomemory parm [EAX] [EDX] value [EDX] = \
                    "IMUL EDX"

extern sint32 FPnDiv(sint32 a, sint32 b, uint32 r);
#pragma aux   FPnDiv modify nomemory [EDX] parm [EAX] [EBX] [ECX] value [EAX] = \
                    "MOV    EDX,EAX" \
                    "SAR    EDX,31"  \
                    "SHLD EDX,EAX,CL" \
                    "SHL  EAX,CL"     \
                    "IDIV EBX"

extern sint32 FP16Pow2(sint32 a);
#pragma aux   FP16Pow2 modify nomemory [EDX] parm [EAX] value [EAX] = \
                    "IMUL EAX"        \
                    "SHRD EAX,EDX,16"

extern sint32 FP24Pow2(sint32 a);
#pragma aux   FP24Pow2 modify nomemory [EDX] parm [EAX] value [EAX] = \
                    "IMUL EAX"        \
                    "SHRD EAX,EDX,24"

extern sint32 FP32Pow2(sint32 a);
#pragma aux   FP32Pow2 modify nomemory parm [EAX] value [EDX] = \
                    "IMUL EAX"

/*
#pragma aux   FPnRoundDiv modify nomemory [EDX] parm [EAX] [EBX] [ECX] value [EAX] = \
                    "CDQ"             \
                    "SHLD EDX,EAX,CL" \
                    "SHL  EAX,CL"     \
                    "MOV  ECX,EBX"    \
                    "SAR  ECX,1"      \
                    "ADD  EAX,ECX"    \
                    "ADC  EDX,0"      \
                    "IDIV EBX"
*/

extern sint32 FPnMult(sint32 a, sint32 b, uint32 r);
#pragma aux   FPnMult modify nomemory parm [EAX] [EDX] [ECX] value [EAX] = \
                    "IMUL EDX"        \
                    "SHRD EAX,EDX,CL"

extern sint32 FPMultDiv(sint32 a, sint32 b, sint32 c);
#pragma aux   FPMultDiv modify nomemory parm [EAX] [EDX] [EBX] value [EAX] = \
                    "IMUL EDX"        \
                    "IDIV EBX"

extern sint32 FPUMultDiv(sint32 a, sint32 b, sint32 c);
#pragma aux   FPUMultDiv modify nomemory parm [EAX] [EDX] [EBX] value [EAX] = \
                    "MUL EDX"        \
                    "DIV EBX"

extern uint32 FP16Inverse(uint32 a);     // 1/a  all 16.16
#pragma aux FP16Inverse modify nomemory [EDX] parm [ECX] value [EAX] = \
        "XOR    EAX,EAX"    \
        "MOV    EDX,1"      \
        "DIV    ECX"

extern void RepStosd(void *d, dword c, uint32 a);
#pragma aux RepStosd parm [EDI] [EAX] [ECX] = \
        "CLD" \
        "REP STOSD"

extern void MemSetD(void *d, dword c, uint32 a);
#pragma aux MemSetD parm [EDI] [EAX] [ECX] = \
        "PUSH ECX"   \
        "SHR  ECX,2" \
        "REP STOSD"  \
        "POP  ECX"   \
        "AND  ECX,3" \
        "REP STOSB"

extern void MemSetW(void *d, dword c, uint32 a);
#pragma aux MemSetW parm [EDI] [EAX] [ECX] = \
        "SHR ECX,1"     \
        "REP STOSW"     \
        "ADC ECX,ECX"   \
        "REP STOSB"

extern void MemSetB(void *d, dword c, uint32 a);
#pragma aux MemSetB parm [EDI] [EAX] [ECX] = \
        "REP STOSB"

extern void RepMovsb(void *d, const void *c, uint32 a);
#pragma aux RepMovsb parm [EDI] [ESI] [ECX] = \
        "CLD"        \
        "PUSH ECX"   \
        "SHR  ECX,2" \
        "REP MOVSD"  \
        "POP  ECX"   \
        "AND  ECX,3" \
        "REP MOVSB"

extern void RepMovsd(void *d, const void *c, uint32 a);
#pragma aux RepMovsd parm [EDI] [ESI] [ECX] = \
        "CLD"        \
        "REP MOVSD"

// ---------------------- Returns (a>0 ? 1 : -1) ------------------------
sint32 Sgn(sint32 a);
#pragma aux Sgn modify nomemory          \
        parm caller [EAX]                \
        value [EAX] =                    \
        "SAR EAX,31"                     \
        "SHL EAX,1"                      \
        "INC EAX"

// ---------------------- Returns (a>0 ? a : -a) ------------------------
sint32 Abs32(sint32 a);
#pragma aux Abs32 modify nomemory        \
        parm caller [EBX]                \
        value [EAX] =                    \
        "MOV EAX,EBX"                    \
        "ADD EBX,EBX"                    \
        "SBB EBX,EBX"                    \
        "XOR EAX,EBX"                    \
        "SUB EAX,EBX"

// ---------------------- Returns a^2
sint32 Pow2(sint32 a);
#pragma aux Pow2 modify nomemory         \
        parm caller [EDX]                \
        value [EAX] =                    \
        "MOV EAX,EDX"                    \
        "IMUL EAX"

sint32 Pow3(sint32 a);
#pragma aux Pow3 modify nomemory         \
        parm caller [EDX]                \
        value [EAX] =                    \
        "MOV EAX,EDX"                    \
        "IMUL EAX,EAX"                   \
        "IMUL EDX"

// ---------------------- Endian stuff

extern dword BSwapDword(dword a);
#pragma aux  BSwapDword modify nomemory parm [EAX] value [EAX] = \
    "XCHG   AL,AH"  \
    "ROR    EAX,16" \
    "XCHG   AL,AH"

extern dword BSwapWord(dword a);
#pragma aux  BSwapWord modify nomemory parm [EAX] value [EAX] = \
    "XCHG   AL,AH"

// ---------------------- Get Segment values

dword _DS(void);
#pragma aux _DS modify nomemory value [EAX] = \
    "XOR EAX,EAX"   \
    "MOV AX,DS"

dword _CS(void);
#pragma aux _CS modify nomemory value [EAX] = \
    "XOR EAX,EAX"   \
    "MOV AX,CS"

dword _ES(void);
#pragma aux _ES modify nomemory value [EAX] = \
    "XOR EAX,EAX"   \
    "MOV AX,ES"

dword _FS(void);
#pragma aux _FS modify nomemory value [EAX] = \
    "XOR EAX,EAX"   \
    "MOV AX,FS"

dword _GS(void);
#pragma aux _GS modify nomemory value [EAX] = \
    "XOR EAX,EAX"   \
    "MOV AX,GS"

void SetES(dword sel);
#pragma aux SetES modify nomemory parm [EAX] = \
    "MOV ES,AX"

void SetFS(dword sel);
#pragma aux SetFS modify nomemory parm [EAX] = \
    "MOV FS,AX"

void SetGS(dword sel);
#pragma aux SetGS modify nomemory parm [EAX] = \
    "MOV GS,AX"

// ----------------------------------

extern void FinishProgram(void);

// ----------------------------------

PUBLIC dword RND_Seed1, RND_Seed2, RND_Seed3;

#define BIOS_Clock (((dword *)0x46C)[0])

PUBLIC void RND_Randomize(dword seed);
#pragma aux RND_Randomize parm [EAX] = \
    "    MOV     [RND_Seed1],EAX"  \
    "    ROR     EAX,13         "  \
    "    MOV     [RND_Seed2],EAX"  \
    "    ROR     EAX,9          "  \
    "    MOV     [RND_Seed3],EAX"

PUBLIC dword RND_GetNum(void);
#pragma aux RND_GetNum modify nomemory [EBX EDX] value [EAX] = \
    "    MOV     EAX,[RND_Seed1]" \
    "    MOV     EBX,[RND_Seed2]" \
    "    MOV     EDX,[RND_Seed3]" \
    "    ADD     EAX,0x0B35FA137" \
    "    ADD     EBX,0x0354C63f7" \
    "    ADD     EDX,0x0067B784B" \
    "    ROL     EAX,2          " \
    "    MOV     [RND_Seed1],EAX" \
    "    ADD     EBX,EAX        " \
    "    ROR     EBX,1          " \
    "    MOV     [RND_Seed2],EBX" \
    "    SUB     EDX,EBX        " \
    "    XOR     EAX,EDX        " \
    "    MOV     [RND_Seed3],EDX" \
    "    ADD     EAX,EBX        "

#endif

// --------------------------- BASE.H ------------------------------
