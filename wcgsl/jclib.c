// ------------------------ JCLIB.C ---------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#include <jclib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>




// ********************************************************
// --------------------------------------------------------
// Structure definitions.

typedef struct {

    char   name[24];
    sint32 offset;
    sint32 size;

} JCLIB_TLFDirEntry, *JCLIB_PLFDirEntry;

#define JCLIB_Magic 0xdf73b489


// ********************************************************
// --------------------------------------------------------
// Private data.

PRIVATE struct {
    sint32		  nfiles;
    sint32		  realnfiles;
    sint32		  lastoffset;
    FILE          *handle;
    JCLIB_PLFDirEntry dir;
} Files[JCLIB_MAXFILES];

PRIVATE int NFiles = 0;

// ********************************************************
// --------------------------------------------------------
// Stdio-like functions.


// --------------------------
// Loads a file and returns
// the number of loaded bytes.

PRIVATE sint32 fload(const char *name, void *buf, sint32 maxsize)
{
    FILE   *f;
    long l, r;
    sint32  size;
    byte *p;

    if ((f = fopen(name, "rb")) == NULL)
        return 0;

    p = buf;
    size = 0;
    do {
        l = maxsize;
        if (l > 32768)
            l = 32768;
        r = fread(p, 1, l, f);
        if (r > 0) {
            p += r;
            size += r;
            maxsize -= r;
        }
    } while (r > 0 && maxsize > 0);
    fclose(f);
    return size;
}


// --------------------------
// Returns the size of a file.

PRIVATE sint32 fsize(const char *name)
{
    FILE   *f;
    sint32  size;

    if ((f = fopen(name, "rb")) == NULL)
        return 0;

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fclose(f);

    return size;
}




// ********************************************************
// --------------------------------------------------------
// Filename handling functions.

PRIVATE bool CmpStr(const char *n1, const char *n2)
{
    while (*n1 != 0 && *n2 != 0) {
	if (toupper(*n1) != toupper(*n2)) return FALSE;
        n1++;
        n2++;
    }

    if (*n1 == 0 && *n2 == 0) return TRUE;
    return FALSE;
}


bool JCLIB_Init(const char *name)
{
    uint32 l, n;
    FILE  *f;

    if (NFiles >= JCLIB_MAXFILES)
        return FALSE;

    f = fopen(name, "rb");
    if (f == NULL) {
    	return FALSE;
    }

    Files[NFiles].dir = NULL;   // Prepare for DISPOSE() on Error

    //JCLIB_Done();
    Files[NFiles].handle = f;

    fseek(f, 0, SEEK_END);
    l = ftell(f);
    if (l <= 16) goto Error;
    fseek(f, -16, SEEK_END);
    if (fread(&n, 1, 4, f) != 4) goto Error;
    if (n != JCLIB_Magic) goto Error;

    if (fread(&Files[NFiles].nfiles,	 1, 4, f) != 4) goto Error;
    if (fread(&Files[NFiles].realnfiles, 1, 4, f) != 4) goto Error;
    if (fread(&Files[NFiles].lastoffset, 1, 4, f) != 4) goto Error;

    fseek(f, -Files[NFiles].lastoffset, SEEK_END);
    Files[NFiles].lastoffset = l - Files[NFiles].lastoffset;

    Files[NFiles].dir = NEW(sizeof(JCLIB_TLFDirEntry) * Files[NFiles].nfiles);
    if (Files[NFiles].dir == NULL) goto Error;
    l = fread(Files[NFiles].dir, 1, sizeof(JCLIB_TLFDirEntry) * Files[NFiles].nfiles, f);
    if (l != sizeof(JCLIB_TLFDirEntry) * Files[NFiles].nfiles) goto Error;

    for (n = 0; n < Files[NFiles].nfiles; n++)
        Files[NFiles].dir[n].offset =
            Files[NFiles].lastoffset - Files[NFiles].dir[n].offset;
    NFiles++;
    return TRUE;

Error:
    //JCLIB_Done();
    fclose(f);
    DISPOSE(Files[NFiles].dir);
    return FALSE;
}


void JCLIB_Done(void)
{
    int i;

    for (i = 0; i < NFiles; i++) {
        if (Files[i].handle != NULL)
    	    fclose(Files[i].handle);
        Files[i].handle = NULL;
	    DISPOSE(Files[i].dir);
    }
}


sint32 JCLIB_FileSize(const char *name)
{
    sint32 i, j;
    const char *p;

    i = fsize(name);
    if (i > 0) return i;

    p = strrchr(name, '\\');
    if (p != NULL)
        name = p+1;
    for (j = NFiles-1; j >= 0; j--) {
        if (Files[j].handle != NULL) {
        	for (i = 0; i < Files[j].nfiles; i++) {
	            if (CmpStr(name, Files[j].dir[i].name)) {
        	    	return Files[j].dir[i].size;
    	        }
        	}
        }
    }
    return 0;
}


sint32 JCLIB_Load(const char *name, char *buffer, sint32 maxsize)
{
    sint32 i, j;
    const char *p;

    i = fload(name, buffer, maxsize);
    if (i > 0) return i;

    p = strrchr(name, '\\');
    if (p != NULL)
        name = p+1;
    for (j = NFiles-1; j >= 0; j--) {
    	for (i = 0; i < Files[j].nfiles; i++) {
    	    if (CmpStr(name, Files[j].dir[i].name)) {
        		fseek(Files[j].handle, Files[j].dir[i].offset, SEEK_SET);
        		if (maxsize > Files[j].dir[i].size)
        		    maxsize = Files[j].dir[i].size;
        		return fread(buffer, 1, maxsize, Files[j].handle);
    	    }
    	}
    }
    return 0;
}


FILE *JCLIB_Open(const char *name)
{
    int   i, j;
    FILE *f;
    const char *p;

    f = fopen(name, "rb");
    if (f != NULL) return f;

    p = strrchr(name, '\\');
    if (p != NULL)
        name = p+1;
    for (j = NFiles-1; j >= 0; j--) {
    	for (i = 0; i < Files[j].nfiles; i++) {
    	    if (CmpStr(name, Files[j].dir[i].name)) {
        		fseek(Files[j].handle, Files[j].dir[i].offset, SEEK_SET);
        		return Files[j].handle;
    	    }
    	}
    }
    return NULL;
}

PUBLIC FILE  *JCLIB_OpenText(const char *name)
{
    int   i, j;
    FILE *f;
    const char *p;

    f = fopen(name, "rt");
    if (f != NULL) return f;

    p = strrchr(name, '\\');
    if (p != NULL)
        name = p+1;
    for (j = NFiles-1; j >= 0; j--) {
    	for (i = 0; i < Files[j].nfiles; i++) {
    	    if (CmpStr(name, Files[j].dir[i].name)) {
                int h = dup(fileno(Files[j].handle));
                setmode(h, O_TEXT);
                f = fdopen(h, "rt");
                if (f != NULL) {
                    fseek(f, Files[j].dir[i].offset, SEEK_SET);
                }
                return f;
        	}
    	}
    }
    return NULL;
}


void JCLIB_Close(FILE *h)
{
    int j;
    for (j = 0; j < NFiles; j++)
        if (Files[j].handle == h)
            return;
    fclose(h);
}

// ---------------------

PUBLIC int JCLIB_GetNLibs(void) {
    return NFiles;
}

PUBLIC int JCLIB_GetNFiles(void) {
    int j, n;
    n = 0;
    for (j = 0; j < NFiles; j++)
        n += Files[j].nfiles;
    return n;
}


// ------------------------ JCLIB.C ---------------------------

