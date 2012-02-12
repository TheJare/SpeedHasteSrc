// -------------------- STRPARSE.C ---------------------
// String parsing functions.
// (C) Copyright 1994-5 by Jare & JCAB of Iguana-VangeliSTeam.

#include "strparse.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>




// --------------------------
// Convert a numeric string to integer.

int STRP_ReadWord(const char *s) {
    int k = 0;
    if (s[0] == '$')
        sscanf(s+1, "%X", &k);
    else
        sscanf(s, "%i", &k);

    return k;
}


// --------------------------
// Remove a string's trailing and leading blanks. It also
// removes CTRL chars and C-style nestable comments.
// 'src' can be the same as 'dest' if desired.

char *STRP_CleanLine(char *dest, const char *src) {
    char *j;
    int incomment = 0;
    int algo = 0;

    if (src == NULL) {
        *dest='\0';
        return NULL;
    }

    while ( *src && (iscntrl(*src) || isspace(*src)) ) src++;
    if (!*src) {
        *dest = '\0';
        return dest;
    }
    j = dest;
    while (*src)
        if (!iscntrl(*src)) {
            if (incomment && *src == '*' && src[1] == '/') {
                src += 2;
                incomment--;
            } else if (*src == '/' && src[1] == '*') {
                src+=2;
                incomment++;
            } else if ((!incomment) && *src == '/' && src[1] == '/') {
                break;
            } else if (!incomment && (algo || !isspace(*src))) {
                algo = 1;
                *j++ = *src++;    /* Aqu¡ est  seguro de que hay algo. */
            } else src++;
        } else
            src++;
    if (j==dest) {
        *dest = '\0';
        return dest;
    }
    while (isspace(*(--j)) );
    j[1] = '\0';
    return dest;
}


// --------------------------
// Split a string in as many as nstr tokens. Tokens can be grouped
// by " or '. Returns the number of tokens, and fills ppc with
// pointers to the tokens. Last token is the rest of the string.
// This works really like creating an argc-argv for a 'main' function.
// NOTE: This modifies the src string.

int STRP_SplitLine(char *ppc[], int nstr, char *src) {
    char *p = src;
    int  n = 0;

    if (src == NULL || ppc == NULL || nstr == 0)
        return 0;
    do {
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '\0')
            return n;
        if (*p == '\"' || *p == '\'') {
            char del = *p++;
            *ppc++ = p;
            n++;
            if ( (p = strchr(p, del)) == NULL)
                return n;
            if (*p == '\0')
                return n;
             *p++ = '\0';
        } else {
            *ppc++ = p;
            while (*p != ' ' && *p != '\t' && *p != '\0')
                p++;
            n++;
            if (*p == '\0')
                return n;
            *p++ = '\0';
        }
    } while (*p != '\0' && n < nstr);
    if (*p != '\0')
        *ppc = p;
    return n;
}

