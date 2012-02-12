// -------------------- STRPARSE.H ---------------------
// String parsing functions.
// (C) Copyright 1994-5 by Jare & JCAB of Iguana-VangeliSTeam.

#ifndef _STRPARSE_H_
#define _STRPARSE_H_


#include <base.h>




int   STRP_ReadWord (const char *s);
char *STRP_CleanLine(char *dest, const char *src);
int   STRP_SplitLine(char *ppc[], int nstr, char *src);




#endif

