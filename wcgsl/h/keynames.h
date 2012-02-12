// ----------------------------- KEYNAMES.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Based on constants bye Patch/Avalanche.

#ifndef _KEYNAMES_H_
#define _KEYNAMES_H_

#include <llkey.h>

typedef struct {
    byte key;
    const char *name;
} KEYN_TKey, *KEYN_PKey;

PUBLIC KEYN_TKey KEYN_Keys[];

// ----------------

PUBLIC const char*KEYN_FindKey(byte key);

PUBLIC byte KEYN_FindKeyCode(const char *name);

#endif

// ----------------------------- KEYNAMES.H -------------------------------
