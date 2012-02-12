// --------------------- MODELS.H -----------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#ifndef _MODELS_H_
#define _MODELS_H_

#include <object3d.h>
#include <is2code.h>

typedef struct {
    char         name[40];
    bool         automatic;
    int          level;
    int          n3d;
    int          maxSpeed;
    O3DM_PObject obj;
} MDL_TCar, *MDL_PCar;

typedef struct {
    char        name[40];
    int         nmap;
    int         level;
    int         length;
    int         best;
    int         record;
    int         nback;
    IS2_PSprite spr;
} MDL_TCircuit, *MDL_PCircuit;

// ----------------

PUBLIC int MDL_NCars;
PUBLIC int MDL_NCircuits;

PUBLIC MDL_PCar     MDL_Cars;
PUBLIC MDL_PCircuit MDL_Circuits;

PUBLIC bool MDL_InitCircuits(void);
PUBLIC bool MDL_LoadCircuits(void);
PUBLIC void MDL_UnloadCircuits(void);
PUBLIC void MDL_EndCircuits(void);

PUBLIC bool MDL_InitCars(void);
PUBLIC bool MDL_LoadCars(void);
PUBLIC void MDL_UnloadCars(void);
PUBLIC void MDL_EndCars(void);

#endif

// --------------------- MODELS.H -----------------------