// ------------------------------ SINCOS_I.C ----------------------------------
// For use with Watcom C 9.0 and Tran's PMode.
// (C) Copyright 1993/4 by Jare & JCAB of Iguana.
// Modified for use with WATCOM 9.5 + DOS4GW

#include <sincos.h>

void InitSinCos(void)
{
    int i, j;

    for (j = 0; j < 3; j++)
        for(i = 1; i < 4096; i++)
            _CosTbl[i] += _CosTbl[i-1];
}

