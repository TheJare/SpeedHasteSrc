// --------------------- MODELS.C -----------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "models.h"
#include <strparse.h>
#include <jclib.h>
#include <stdio.h>
#include <string.h>

#include "globals.h"

PUBLIC int MDL_NCars = 0;
PUBLIC int MDL_NCircuits = 0;

PUBLIC MDL_PCar     MDL_Cars = NULL;
PUBLIC MDL_PCircuit MDL_Circuits = NULL;

PUBLIC bool MDL_InitCircuits(void) {
    FILE *f;
    char line[200], *tok[10];
    int i, n, ntok;

    if (MDL_Circuits != NULL)
        MDL_EndCircuits();
    REQUIRE( (f = JCLIB_OpenText("circuits.lst")) != NULL);
    fgets(line, 200, f);
    STRP_CleanLine(line, line);
    MDL_NCircuits = STRP_ReadWord(line);
    REQUIRE(MDL_NCircuits > 0);
    MDL_Circuits = NEW(sizeof(*MDL_Circuits)*MDL_NCircuits);
    REQUIRE(MDL_Circuits != NULL);
    memset(MDL_Circuits, 0, sizeof(*MDL_Circuits)*MDL_NCircuits);
    n = -1;
    i = -1;
    while (!feof(f) && n < MDL_NCircuits) {
        fgets(line, 200, f);
        STRP_CleanLine(line, line);
        if (line[0] == '\0' || line[0] == ';')
            continue;
        ntok = STRP_SplitLine(tok, SIZEARRAY(tok), line);
        if (ntok <= 0)
            continue;
        if (stricmp(tok[0], "End") == 0)
            break;
        if (stricmp(tok[0], "Circuit") == 0)
            n++;
        else if (n >= 0) {
            if (stricmp(tok[0], "Name") == 0) {
                strncpy(MDL_Circuits[n].name, tok[1], SIZEARRAY(MDL_Circuits[n].name));
                MDL_Circuits[n].name[SIZEARRAY(MDL_Circuits[n].name)-1] = '\0';
            } else if (stricmp(tok[0], "Number") == 0)
                MDL_Circuits[n].nmap = STRP_ReadWord(tok[1]);
            else if (stricmp(tok[0], "Level") == 0)
                MDL_Circuits[n].level = STRP_ReadWord(tok[1]);
            else if (stricmp(tok[0], "Length") == 0)
                MDL_Circuits[n].length = STRP_ReadWord(tok[1]);
            else if (stricmp(tok[0], "Best") == 0)
                MDL_Circuits[n].best = STRP_ReadWord(tok[1]);
            else if (stricmp(tok[0], "Record") == 0)
                MDL_Circuits[n].record = STRP_ReadWord(tok[1+GL_CarType]);
            else if (stricmp(tok[0], "NBack") == 0)
                MDL_Circuits[n].nback = STRP_ReadWord(tok[1]);
        }
    }
    JCLIB_Close(f);
    if (n < 0)
        DISPOSE(MDL_Circuits);
    MDL_NCircuits = n+1;

    for (i = 0; i < MDL_NCircuits; i++)
        MDL_Circuits[i].spr = NULL;

    return MDL_Circuits != NULL;
}

PUBLIC bool MDL_LoadCircuits(void) {
    int i;
    char buf[100];

    if (MDL_Circuits == NULL)
        if (!MDL_InitCircuits())
            return FALSE;
    for (i = 0; i < MDL_NCircuits; i++) {
        sprintf(buf, "mapspr%02d.is2", MDL_Circuits[i].nmap);
        if ( (MDL_Circuits[i].spr = IS2_Load(buf)) == NULL)
            BASE_Abort("Trying to read map sprite %s", buf);
    }
    return TRUE;
}

PUBLIC void MDL_UnloadCircuits(void) {
    int i;
    if (MDL_Circuits == NULL)
        return;
    for (i = 0; i < MDL_NCircuits; i++)
        DISPOSE(MDL_Circuits[i].spr);
}

PUBLIC void MDL_EndCircuits(void) {
    int i;
    if (MDL_Circuits == NULL)
        return;
    MDL_UnloadCircuits();
    DISPOSE(MDL_Circuits);
    MDL_NCircuits = 0;
}

// -----------------------------

PUBLIC bool MDL_InitCars(void) {
    FILE *f;
    char line[200], *tok[10];
    int i, n, ntok;

    if (MDL_Cars != NULL)
        MDL_EndCars();
    REQUIRE( (f = JCLIB_OpenText("cars.lst")) != NULL);
    fgets(line, 200, f);
    STRP_CleanLine(line, line);
    MDL_NCars = STRP_ReadWord(line);
    REQUIRE(MDL_NCars > 0);
    MDL_Cars = NEW(sizeof(*MDL_Cars)*MDL_NCars);
    REQUIRE(MDL_Cars != NULL);
    memset(MDL_Cars, 0, sizeof(*MDL_Cars)*MDL_NCars);
    n = -1;
    i = -1;
    while (!feof(f) && n < MDL_NCars) {
        fgets(line, 200, f);
        STRP_CleanLine(line, line);
        if (line[0] == '\0' || line[0] == ';')
            continue;
        ntok = STRP_SplitLine(tok, SIZEARRAY(tok), line);
        if (ntok <= 0)
            continue;
        if (stricmp(tok[0], "End") == 0)
            break;
        if (stricmp(tok[0], "Car") == 0)
            n++;
        else if (n >= 0 && ntok >= 4) {
            if (stricmp(tok[0], "Name") == 0) {
                strncpy(MDL_Cars[n].name, tok[1+GL_CarType], SIZEARRAY(MDL_Cars[n].name));
                MDL_Cars[n].name[SIZEARRAY(MDL_Cars[n].name)-1] = '\0';
            } else if (stricmp(tok[0], "Number") == 0)
                MDL_Cars[n].n3d = STRP_ReadWord(tok[1+GL_CarType]);
            else if (stricmp(tok[0], "Level") == 0)
                MDL_Cars[n].level = STRP_ReadWord(tok[1+GL_CarType]);
            else if (stricmp(tok[0], "Automatic") == 0)
                MDL_Cars[n].automatic = STRP_ReadWord(tok[1+GL_CarType]);
            else if (stricmp(tok[0], "MaxSpeed") == 0)
                MDL_Cars[n].maxSpeed = STRP_ReadWord(tok[1+GL_CarType]);
        }
    }
    JCLIB_Close(f);
    if (n < 0)
        DISPOSE(MDL_Cars);
    MDL_NCars = n+1;

    for (i = 0; i < MDL_NCars; i++)
        MDL_Cars[i].obj = NULL;

    return MDL_Cars != NULL;
}


PUBLIC bool MDL_LoadCars(void) {
    int i;
    char line[100];
    if (MDL_Cars == NULL)
        if (!MDL_InitCars())
            return FALSE;
    for (i = 0; i < MDL_NCars; i++) {
        O3DM_PObject obj;
        int j;
        sint32 minx, maxx, miny, maxy, minz, maxz;

        sprintf(line, "car%dn%d.i3d", GL_CarType, MDL_Cars[i].n3d);
        if ( (MDL_Cars[i].obj = O3DM_LoadObject(line)) == NULL) {
            sprintf(line, "car%dn%da.i3d", GL_CarType, MDL_Cars[i].n3d);
            if ( (MDL_Cars[i].obj = O3DM_LoadObject(line)) == NULL)
                BASE_Abort("Trying to read car 3D mesh %s", line);
        }
            // Process object's scale factors.
        obj = MDL_Cars[i].obj;
        if (obj->scx == 0) obj->scx = 1 << 19;  // 0x80000
        if (obj->scy == 0) obj->scy = 1 << 19;
        if (obj->scz == 0) obj->scz = 1 << 19;
        obj->scx >>= 5;
        obj->scy >>= 5;
        obj->scz >>= 5;
            // Forced car center.
        minx = maxx = obj->verts[0].x;
        miny = maxy = obj->verts[0].y;
        minz = maxz = obj->verts[0].z;
        for (j = 1; j < obj->nVerts; j++) {
            if (minx > obj->verts[j].x) minx = obj->verts[j].x;
            if (maxx < obj->verts[j].x) maxx = obj->verts[j].x;
            if (miny > obj->verts[j].y) miny = obj->verts[j].y;
            if (maxy < obj->verts[j].y) maxy = obj->verts[j].y;
            if (minz > obj->verts[j].z) minz = obj->verts[j].z;
            if (maxz < obj->verts[j].z) maxz = obj->verts[j].z;
        }
        obj->dcx = (maxx+minx)/2;
        obj->dcy = (maxy+miny)/2;
        obj->dcz = (maxz+minz)/2;
        for (j = 0; j < obj->nVerts; j++) {
            obj->verts[j].x = FPnMult(obj->verts[j].x - obj->dcx, obj->scx, 16);
            obj->verts[j].y = FPnMult(obj->verts[j].y - obj->dcy, obj->scy, 16);
            obj->verts[j].z = FPnMult(obj->verts[j].z - obj->dcz, obj->scz, 16);
        }
    }
    return TRUE;
}

PUBLIC void MDL_UnloadCars(void) {
    int i;
    if (MDL_Cars == NULL)
        return;
    for (i = 0; i < MDL_NCars; i++) {
        O3DM_DeleteObject(MDL_Cars[i].obj);
        MDL_Cars[i].obj = NULL;
    }
}

PUBLIC void MDL_EndCars(void) {
    int i;
    if (MDL_Cars == NULL)
        return;
    MDL_UnloadCars();
    DISPOSE(MDL_Cars);
    MDL_NCars = 0;
}

// --------------------- MODELS.C -----------------------