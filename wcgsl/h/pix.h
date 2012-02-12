// ---------------------------- PIX.H -----------------------
// Written by Javier Ar‚valo Baeza.
// Screen loading functions.

#ifndef _PIX_H_
#define _PIX_H_

#ifndef _BASE_H_
#include "base.h"
#endif

PUBLIC int PIX_LoadGIF(const char* file, unsigned char *outpix, unsigned char * pal, int* width, int* height);

PUBLIC int PIX_LoadPCX(const char* file, unsigned char *outpix, unsigned char * pal, int* width, int* height);

PUBLIC int PIX_LoadLBM(const char* file, unsigned char *outpix, unsigned char * pal, int* width, int* height);

PUBLIC int PIX_Load(const char* file, unsigned char *outpix, unsigned char * pal, int* width, int* height);

#endif

// ---------------------------- PIX.H -----------------------
