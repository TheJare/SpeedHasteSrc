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

#endif

// --------------------------- BASE.H ------------------------------
