// --------------------------- BASE.C ------------------------------
// For use with with WATCOM 9.5 + DOS4GW
// (C) Copyright 1993/4 by Jare & JCAB of Iguana.

#include <base.h>
#include <llkey.h>
#include <llmouse.h>
#include <llscreen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

struct SREGS sregs;
union REGS inregs, outregs;

int ArgC    = 1;
PRIVATE char *dumbname[] = {
    "WCGSL",
    NULL
};
char **ArgV = dumbname;

extern void FinishProgram(void);

PUBLIC void BASE_Abort(const char *str, ...) {
    va_list arg;

    FinishProgram();
    printf("컴컴컴컴컴컴컴컴컴컴컴컴컴컴 ERROR 컴컴컴컴컴컴컴컴컴컴컴�\n");
    va_start(arg, str);
    vprintf(str, arg);
    va_end(arg);
    exit(1);
}

void BASE_Require(const char *cond, const char *file, int line) {
    BASE_Abort("Condition %s failed in source file %s, line %d.\n",
               cond, file, line);
}

int BASE_CheckArg(const char *parm) {
    int i;

    for (i = 1; i < ArgC; i++) {
        if (   (ArgV[i][0] == '-' || ArgV[i][0] == '/')
            && strcmpi(ArgV[i]+1, parm) == 0)
            return i+1;
    }
    return -1;
}

// ---------------------------------


PUBLIC dword RND_Seed1 = 0x07834348B;
PUBLIC dword RND_Seed2 = 0x078ED7F34;
PUBLIC dword RND_Seed3 = 0x0a78332bf;



// --------------------------- BASE.C ------------------------------

