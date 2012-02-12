// ------------------------ JCLIB.H ---------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#ifndef _JCLIB_H_
#define _JCLIB_H_

#ifndef _BASE_H_
#include <base.h>
#endif

#include <stdio.h>


#define JCLIB_MAXFILES 20


PUBLIC bool   JCLIB_Init    (const char *name);
PUBLIC void   JCLIB_Done    (void);
PUBLIC sint32 JCLIB_FileSize(const char *name);
PUBLIC sint32 JCLIB_Load    (const char *name, void *buf, sint32 maxsize);
PUBLIC FILE  *JCLIB_Open    (const char *name);
PUBLIC FILE  *JCLIB_OpenText(const char *name);
PUBLIC void   JCLIB_Close   (FILE *h);

PUBLIC int JCLIB_GetNLibs(void);
PUBLIC int JCLIB_GetNFiles(void);

#endif
