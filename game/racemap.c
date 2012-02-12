// ------------------------------ RACEMAP.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "racemap.h"

#include <jclib.h>
#include <atan.h>
#include "things.h"
#include "globals.h"
#include "models.h"

typedef struct {
    word Version;
    word Reserved;
    word Map[64][64];
    byte Angulo[64][64];
} tDiskMap, *pDiskMap;

    // Loads things, starting positions and external cameras. Loading things
    // also means inserting their FS3 into the GL_NameList. Needs a previous
    // MAP_Load().
PUBLIC bool MAP_LoadThings(MAP_PMap map, const char *fname) {
    FILE *f;
    struct {
        word x, y, angle, type;
    } mapt;
    long l, p;
    uint32 tx, ty;
    int i;

    REQUIRE((f = JCLIB_Open(fname)) != NULL);
    l = JCLIB_FileSize(fname) - sizeof(tDiskMap);
    fseek(f, ftell(f) + sizeof(tDiskMap), SEEK_SET);
    while (l >= sizeof(mapt) && fread(&mapt, sizeof(mapt), 1, f) == 1) {
//        printf("Loading thing type 0x%X\n", mapt.type);
            // Correct mapper quirks with the data.
        mapt.angle = 0x4000 - mapt.angle;
        p = ftell(f);       // Save, as it can be in a datafile and things
                            // might get loaded by the next calls.
        tx = (mapt.x << 19) + (1 << 30);
        ty = (mapt.y << 19) + (1 << 30);
        if (mapt.type >= (240 << 8)) {
                // It is a special thing type.
            switch ((mapt.type & 0xFF00) >> 8) {
                case 241:
                    if ((mapt.type & 0xF) == 0)
                        MAP_AddStartPos(map,
                                        tx, ty,
                                        mapt.angle);
                    break;
                case 242:
                    MAP_AddStaticCamera(map,
                                        tx, ty,
                                        900 + (mapt.type & 0xFF)*500);
                    break;
            }
        } else {
            if ((RND_GetNum()&15) < DecorationDetail) {
                    // It is a normal obstacle/decoration.
                THN_AddThing(tx, ty, 0,
                             THNT_OBSTACLE + mapt.type, mapt.angle, 0);
            }
        }
        l -= sizeof(mapt);
        fseek(f, p, SEEK_SET);
    }
    JCLIB_Close(f);

    return TRUE;
}

    // Load clouds and mountains, create thingmap, load grafs.
PUBLIC bool MAP_Load(MAP_PMap map, const char *fname) {
    word     charmap[512];
    int      nchars = 0;
    static tDiskMap m;
    pDiskMap pm = &m;
    FILE     *f;
    long     seekbase, l;
    int      i, j;
    static byte grf[64*64];
    char     buf[30];
    int c;

    assert(map != NULL);

    c = fname[4] - '0';
    c = MDL_Circuits[c].nback;
    sprintf(buf, "nubes%d.pix", c);
    l = JCLIB_FileSize(buf);
    if (!( (map->backg = NEW(l)) != NULL))
        BASE_Abort("Out of memory for first background");
    JCLIB_Load(buf, map->backg, l);
    map->hbackg = l/320;

    sprintf(buf, "mount%d.pix", c);
    l = JCLIB_FileSize(buf);
    if (!( (map->backg2 = NEW(l)) != NULL))
        BASE_Abort("Out of memory for second background");
    JCLIB_Load(buf, map->backg2, l);
    map->hbackg2 = l/320;

    REQUIRE(JCLIB_Load(fname, pm, sizeof(*pm)) == sizeof(*pm));
    if (!((map->thnMap = NEW(64*64*sizeof(*map->thnMap))) != NULL))
        BASE_Abort("Out of memory for thing map");
    THN_InitMap(map->thnMap);
    map->staticCameras = NULL;
    map->startPos = NULL;
    nchars = 0;
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            word c;
            int k;

            c = pm->Map[i][j] | (((word)pm->Angulo[i][j]) << 14);
            for (k = 0; k < nchars; k++)
                if (c == charmap[k])
                    break;
            if (k >= nchars) {
                nchars++;
                charmap[k] = c;
            }
            pm->Map[i][j] = k;
        }
    }
//    printf("Getting %d bytes for %d floor graphics.\n", 64*64*nchars, nchars);
    map->nGrafs = nchars;
    if (!((map->grafs = NEW(64*64*nchars)) != NULL))
        BASE_Abort("Out of memory for %d map tiles", nchars);
    REQUIRE((f = JCLIB_Open("grafs.dat")) != NULL);
    seekbase = ftell(f);
    for (i = 0; i < nchars; i++) {
        byte *p;
        word rotacion;

        p = map->grafs+i*64*64;
        rotacion = charmap[i] >> 14;
        REQUIRE(fseek(f, seekbase + 64*64*(charmap[i] & 0x3FFF), SEEK_SET) >= 0);
        REQUIRE(fread(grf, 64*64, 1, f) == 1);

        {
            int i, j;
            if (rotacion == 0) {
                  // Rotar 0 posiciones.
                for(i=0;i<64;i++)
                   for (j = 0; j < 64; j++)
                        p[i*64+j] = grf[i*64+j];
            } else if (rotacion == 1) {
                  // Rotar 1 posiciones.
                for(i=0;i<64;i++)
                   for (j = 0; j < 64; j++)
                        p[i*64+j] = grf[(63-j)*64+i];
            } else if (rotacion == 2) {
                  // Rotar 2 posicion.
                for(i=0;i<64;i++)
                   for (j = 0; j < 64; j++)
                        p[i*64+j] = grf[(63-i)*64+63-j];
            } else if (rotacion == 3) {
                  // Rotar 3 posicion.
                for(i=0;i<64;i++)
                   for (j = 0; j < 64; j++)
                        p[i*64+j] = grf[j*64+63-i];
            }
        }
    }
    for (i = 0; i < 128; i++)
        for (j = 0; j < 128; j++) {
            if (i >= 32 && i < (64+32) && j >= 32 && j < (64+32))
                map->map[j*128+i] = map->grafs + 64*64*(dword)pm->Map[i-32][j-32];
            else
                map->map[j*128+i] = map->grafs;
        }
    JCLIB_Close(f);
//    DISPOSE(grf);
//    DISPOSE(pm);
    map->trans = GL_ClrTable;
    return TRUE;
}

PUBLIC void MAP_Free(MAP_PMap map) {
    assert(map != NULL);
    DISPOSE(map->backg);
    DISPOSE(map->backg2);
    DISPOSE(map->grafs);
    DISPOSE(map->thnMap);
}

 // ------------------------------------

PUBLIC bool MAP_AddStaticCamera(MAP_PMap map, uint32 x, uint32 y, uint32 z) {
    if (map->staticCameras == NULL) {
        map->nStaticCameras = 0;
        map->staticCameras = NEW(MAP_MAX_STATIC_CAMERAS*sizeof(*map->staticCameras));
        if (map->staticCameras == NULL)
            return FALSE;
    }
    if (map->nStaticCameras >= MAP_MAX_STATIC_CAMERAS)
        return FALSE;
    map->staticCameras[map->nStaticCameras].x = x;
    map->staticCameras[map->nStaticCameras].y = y;
    map->staticCameras[map->nStaticCameras].z = z;
    map->nStaticCameras++;
    return TRUE;
}

PUBLIC MAP_PStaticCamera MAP_FindStaticCamera(MAP_PMap map, uint32 x, uint32 y, dword *d) {
    dword i, j, mindist;
    if (map->staticCameras == NULL)
        return NULL;
    assert(map->nStaticCameras > 0);
    mindist = 0xFFFFFFFFUL;
    j = 0;
    for (i = 0; i < map->nStaticCameras; i++) {
        dword dist;
        dist = Pow2((map->staticCameras[i].x >> 20) - (x >> 20))
             + Pow2((map->staticCameras[i].y >> 20) - (y >> 20));
        if (dist < mindist) {
            j = i;
            mindist = dist;
        }
    }
    if (d != NULL)
        *d = mindist;
    return map->staticCameras + j;
}

PUBLIC void MAP_EndStaticCameras(MAP_PMap map) {
    DISPOSE(map->staticCameras);
    map->nStaticCameras = 0;
}

 // ------------------------------------

PUBLIC bool MAP_AddStartPos(MAP_PMap map, uint32 x, uint32 y, word angle) {
    if (map->startPos == NULL) {
        map->nStartPos = 0;
        map->startPos = NEW(MAP_MAX_START_POS*sizeof(*map->startPos));
        if (map->startPos == NULL)
            return FALSE;
    }
    if (map->nStartPos >= MAP_MAX_START_POS)
        return FALSE;
    map->startPos[map->nStartPos].x = x;
    map->startPos[map->nStartPos].y = y;
    map->startPos[map->nStartPos].angle = angle;
    map->startPos[map->nStartPos].used = FALSE;
    map->nStartPos++;
    return TRUE;
}

PUBLIC int MAP_UseStartPos(MAP_PMap map) {
    int i;
    for (i = 0; i < map->nStartPos; i++)
        if (!map->startPos[i].used)
            break;
    if (i >= map->nStartPos)
        return -1;
    i = -1;
    do {
        i = RND_GetNum()%map->nStartPos;
    } while (map->startPos[i].used);
    map->startPos[i].used = TRUE;
    return i;
}

PUBLIC void MAP_EndStartPos(MAP_PMap map) {
    DISPOSE(map->startPos);
    map->nStartPos = 0;
}

 // ------------------------------------

typedef union {

    char  ascii[4];
    dword num;

} PATH_TMagic, *PATH_PMagic;


typedef union {

    struct {
        PATH_TMagic magic;
        char id[192-sizeof(PATH_TMagic)];

        int  version;
        int  numpoints;
        int  pointsize;
    };
    byte header[256];

} PATH_THeader, *PATH_PHeader;

void PATH_Init(PATH_PPath path, const char *fname)
{
    PATH_TMagic magic = { { 'P', 'A', 'T', 'H' } };
    FILE *f;
    int   i;
    PATH_THeader hdr;

    path->points = NULL;

    f = JCLIB_Open(fname);
    REQUIRE(fread(&hdr, 1, sizeof(hdr), f) == sizeof(hdr));
    REQUIRE(hdr.magic.num == magic.num);
    if (!((path->points = NEW(sizeof(*path->points)*hdr.numpoints)) != NULL))
        BASE_Abort("Out of memory for path points");
    for (i = 0; i < hdr.numpoints; i++) {
        REQUIRE(fread(path->points + i, 4*sizeof(uint32), 1, f) == 1);
        path->points[i].ncars = 0;
    }
    path->numpoints = hdr.numpoints;
    JCLIB_Close(f);
}

void PATH_Done(PATH_PPath path)
{
    if (path->points != NULL)
        DISPOSE(path->points);
    path->points = NULL;
}


// ------------------------------ RACEMAP.C ----------------------------

