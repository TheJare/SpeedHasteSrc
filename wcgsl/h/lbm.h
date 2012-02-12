// --------------------------- LBM.H ------------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _LBM_H_
#define _LBM_H_

#ifndef _BASE_H_
#include <base.h>
#endif

PUBLIC bool LBM_Write(const char *fname, int width, int height,
                      const byte *pal, const byte *scr);

PUBLIC bool PCX_Write(const char *fname, int width, int height,
                      const byte *pal, const byte *scr);

#endif

// --------------------------- LBM.H ------------------------------

