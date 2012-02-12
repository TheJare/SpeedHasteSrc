// ------------------------------ HORDRAW.H ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _HORDRAW_H_
#define _HORDRAW_H_

#ifndef _BASE_H_
#include <base.h>
#endif

extern void DRW_DoHorizontalDraw(byte *dest, const byte *data, int skip, int width);
#pragma aux DRW_DoHorizontalDraw parm [EDI] [ESI] [EDX] [EBX]

#endif

// ------------------------------ HORDRAW.H ----------------------------
