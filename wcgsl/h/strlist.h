

#ifndef _STRLIST_H_
#define _STRLIST_H_

#include <base.h>

struct STL_SNode;

typedef struct STL_SNode {
    struct STL_SNode *next;
    char str[1];
} STL_TNode, *STL_PNode;

typedef struct {
    STL_PNode first;
    int       nstr;
} STL_TStringList, *STL_PStringList;

PUBLIC bool STL_Init(STL_PStringList l);

PUBLIC bool STL_Add(STL_PStringList l, const char *str);

PUBLIC const char *STL_Get(STL_PStringList l, int i);

PUBLIC void STL_End(STL_PStringList l);

#endif

