// ------------------------------ NAMELIST.C --------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "namelist.h"
#include <jclib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

PUBLIC bool NL_Init(NL_PNameTree t) {
    assert(t != NULL);
    *t = NULL;
    return TRUE;
}

PUBLIC void NL_End(NL_PNameTree t) {
    assert(t != NULL);
    if (*t == NULL)
        return;
    if ((*t)->left != NULL)
        NL_End(&(*t)->left);
    if ((*t)->right != NULL)
        NL_End(&(*t)->right);
    DISPOSE(*t);
}

PUBLIC NL_PName NL_AddName(NL_PNameTree t, const char *name) {
    NL_PName p, *g;

    assert(t != NULL);
    assert(name != NULL);
    g = t;
    while (*g != NULL) {
        int rez;

        p = *g;
        rez = strnicmp(p->name, name, sizeof(p->name));
        if (rez > 0)
            g = &p->left;
        else if (rez < 0)
            g = &p->right;
        else                    // found!
            return p;
    }
        // Not found, let's add a node.

    p = NEW(sizeof(*p));
    if (p == NULL)
        return NULL;

    *g = p;
    p->left  = NULL;
    p->right = NULL;
    strncpy(p->name, name, sizeof(p->name));
    p->data = NULL;
    return p;
}

PUBLIC void     NL_AddTree(NL_PNameTree t, NL_TNameTree src) {
    NL_PName p, *g;

    assert(t != NULL);
    if (src == NULL)
        return;
    g = t;
    while (*g != NULL) {
        int rez;

        p = *g;
        rez = strnicmp(p->name, src->name, sizeof(p->name));
        if (rez > 0)
            g = &p->left;
        else if (rez < 0)
            g = &p->right;
        else                    // found!
            BASE_Abort("AddTree found the name already in the tree.\n");
    }
        // Not found, let's add a node.

    *g = src;
}


PUBLIC NL_PName NL_FindName(NL_PNameTree t, const char *name) {
    NL_PName p;

    assert(t != NULL);
    assert(name != NULL);
    p = *t;
    while (p != NULL) {
        int rez;

        rez = strnicmp(p->name, name, sizeof(p->name));
        if (rez > 0)
            p = p->left;
        else if (rez < 0)
            p = p->right;
        else                    // found!
            return p;
    }
    return NULL;
}

PUBLIC void NL_DelName(NL_PNameTree t, const char *name) {
    NL_PName p, q, *g;

    assert(t != NULL);
    assert(name != NULL);
    g = t;
    while (*g != NULL) {
        int rez;

        p = *g;
        rez = strnicmp(p->name, name, sizeof(p->name));
        if (rez > 0)
            g = &p->left;
        else if (rez < 0)
            g = &p->right;
        else {                   // found!
            q = p->right;       // Save leaves.
            p = p->left;
            DISPOSE(*g);        // Delete node.
                // Now the leaves are separated from the root.
                // Fit them in again.
            NL_AddTree(g, p);
            NL_AddTree(g, q);
        }
    }
        // Not found!

    BASE_Abort("Node not found for deletion searching \"%s\"!", name);
}

PUBLIC bool NL_WalkTree(NL_PNameTree t, NL_PWalkFunction func, void *data) {
    assert(t != NULL);
    assert(func != NULL);
    if (*t != NULL) {
        if (!NL_WalkTree(&(*t)->left, func, data))  // First the children.
            return FALSE;
        if (!NL_WalkTree(&(*t)->right, func, data))
            return FALSE;
        if (!func(t, data))
            return FALSE;
    }
    return TRUE;
}

// ------------------------------ NAMELIST.C --------------------

