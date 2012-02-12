// ------------------------------ NAMELIST.H --------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

// Nifty binary tree to store something and access it by name. Store the
// data you want to be named in the "data" pointer field of the TName.

#ifndef _NAMELIST_H_
#define _NAMELIST_H_

#ifndef _BASE_H_
#include <base.h>
#endif

    // Node in the names binary tree.
typedef struct sNLName {
    char            name[8];
    void           *data;
    struct sNLName *left,
                   *right;
} NL_TName, *NL_PName;

typedef NL_PName  NL_TNameTree;
typedef NL_PName *NL_PNameTree;

// -----------------------------------

PUBLIC bool     NL_Init(NL_PNameTree t);

PUBLIC void     NL_End(NL_PNameTree t);

PUBLIC NL_PName NL_AddName(NL_PNameTree t, const char *name);

PUBLIC NL_PName NL_FindName(NL_PNameTree t, const char *name);

PUBLIC void     NL_AddTree(NL_PNameTree t, NL_TNameTree src);

PUBLIC void     NL_DelName(NL_PNameTree t, const char *name);

typedef bool (*NL_PWalkFunction)(NL_PName *n, void *data);

    // In-deep walk through the tree. Visit a node after its children.
PUBLIC bool NL_WalkTree(NL_PNameTree t, NL_PWalkFunction func, void *data);
//PUBLIC void NL_DumpTree(NL_PNameTree t);

#endif

// ------------------------------ NAMELIST.H --------------------

