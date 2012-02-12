
#include "strlist.h"

#include <string.h>

PUBLIC bool STL_Init(STL_PStringList l) {
    assert(l != NULL);
    l->first = NULL;
    l->nstr  = 0;
    return TRUE;
}

PUBLIC bool STL_Add(STL_PStringList l, const char *str) {
    STL_PNode *n;

    assert(l != NULL);
    n = &l->first;
    while (*n != NULL)
        n = &((*n)->next);
    *n = NEW(sizeof(STL_PNode) + strlen(str)+1);
    if (*n == NULL)
        return FALSE;
    (*n)->next = NULL;
    strcpy((*n)->str, str);
    l->nstr++;
    return TRUE;
}

PUBLIC const char *STL_Get(STL_PStringList l, int i) {
    STL_PNode p;
    assert(l != NULL);
    if (i >= l->nstr)
        return NULL;
    p = l->first;
    while (i > 0 && p != NULL) {
        p = p->next;
        i--;
    }
    if (p == NULL)
        return NULL;
    return p->str;
}

PUBLIC void STL_End(STL_PStringList l) {
    STL_PNode p, q;

    assert(l != NULL);
    for (p = l->first; p != NULL; p = q) {
        q = p->next;
        DISPOSE(p);
    }
    DISPOSE(l->first);
    l->nstr = 0;
}

