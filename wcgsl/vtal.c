// ----------------- VTAL.C ----------------------
// VT Audio Library. System-dependent functions.
// Watcom C 10.0 and DOS4GW version.
// (C) Copyright 1994-95 by JCAB of Iguana-VangeliSTeam.

#include <vtal.h>

#include <stdlib.h>
#include <jclib.h>

#undef  PUBLIC
#define PUBLIC




// ********************************************************
// --------------------------------------------------------
// Public code.

PUBLIC void PUBLICFUNC XTRN_GetMem (LPLPmem p, ulong l)
{
    if (p != NULL) {
        if (l > 0) {
            *p = malloc(l);
        } else
            *p = NULL;
    }
}


PUBLIC void PUBLICFUNC XTRN_FreeMem(LPLPmem p, ulong l)
{
    if (p != NULL && *p != NULL) {
        free(*p);
        *p = NULL;
    }
}


PUBLIC void PUBLICFUNC XTRN_ReallocMem(LPLPmem p, ulong ol, ulong nl)
{
    if (p != NULL && *p != NULL) {
        void *q = realloc(*p, nl);
        if (q != NULL)
            *p = q;
        else
            XTRN_FreeMem(p, ol);
    }
}


PUBLIC ulong PUBLICFUNC XTRN_FileSize(LPconststr fname)
{
    if (fname == NULL)
        return 0;

    return JCLIB_FileSize(fname);
}


PUBLIC ulong PUBLICFUNC XTRN_LoadFile(LPconststr fname, LPmem buf, ulong size)
{
    if (fname == NULL || buf == NULL || size == 0)
        return 0;

    return JCLIB_Load(fname, buf, size);
}


PUBLIC FILE_handle PUBLICFUNC XTRN_OpenFile(LPconststr fname, LPconststr mode)
{
    if (fname == NULL || mode == NULL)
        return NULL;

    return (FILE_handle) JCLIB_Open(fname);
}


PUBLIC void PUBLICFUNC XTRN_CloseFile(FILE_handle f)
{
    if (f == NULL)
        return;

    JCLIB_Close((FILE *) f);
}


PUBLIC ulong PUBLICFUNC XTRN_ReadFile(FILE_handle f, LPmem buf, ulong size)
{
    if (f == NULL || buf == NULL || size == 0)
        return 0;

    return fread(buf, 1, size, (FILE *) f);
}


PUBLIC ulong PUBLICFUNC XTRN_FilePos(FILE_handle f)
{
    if (f == NULL)
        return 0;

    return ftell((FILE *) f);
}


PUBLIC ulong PUBLICFUNC XTRN_SeekFile(FILE_handle f, ulong newpos)
{
    if (f == NULL)
        return 0;

    return fseek((FILE *) f, newpos, SEEK_SET);
}

PUBLIC bool PUBLICFUNC XTRN_InitStack(XTRN_PStack stk, uint size)
{
    if (stk == NULL || size == 0)
        return FALSE;

    stk->Size = size;
    XTRN_GetMem(&stk->Buf, size);
    if (stk->Buf == NULL)
        return FALSE;

    stk->Semaphore = 1;
    return TRUE;
}

PUBLIC void PUBLICFUNC XTRN_DoneStack(XTRN_PStack stk)
{
    if (stk == NULL || stk->Size == 0)
        return;

    stk->Semaphore = 0;
    XTRN_FreeMem(&stk->Buf, stk->Size);
}



