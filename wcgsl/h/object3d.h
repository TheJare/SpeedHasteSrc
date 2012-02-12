// ----------------------------- OBJECT3D.H ---------------------------
// For use with Watcom C.
// (C) Copyright 1995 by Jare & JCAB of Iguana.

// Definitive 3D object routines.

#ifndef _OBJECT3D_H_
#define _OBJECT3D_H_

#include <rot3d.h>

/*

    An object should be composed of:

        Detail? Or differently detailed objects shall be different objects?

        Vertices
        Normals
        Faces
            Normal
            Color
            Texture
            Vertices
                Normal
                Mapping

    Proposed structure: O3DF_ means disk data, O3DM_ means memory data.

    Skeleton file: were a value seems redundant, it will be used for
          consistency check, and won't appear in the binary version.
          This is especially true of the order numbers.

    Object "name"
    Vertices # Normals # Faces # FaceVertices # Materials #
    Flags 0x0000
    EndHeader
    Material # Color # Flags # Ambient # Diffuse # Reflected # Texture "tname"
     ...
    EndMaterials
    Vertex # X # Y # Z #
     ...
    EndVertices
    Normal # X # Y # Z #
     ...
    EndNormals
    Face # Vertices # Material # Flags # Array
        Vertex # Normal # TextureX # TextureY #
         ...
     ...
    EndFaces
*/

// -----------------------------------------
// Disk image of the object.

typedef struct {
    sint32  x, y, z;            // Pretty obvious.
} O3DF_TVertex, O3DF_TNormal;

typedef struct {
    sint16 nvert;               // 3D vertex index.
    sint16 nnorm;               // Normal vector at this vertex.
    sint32 tx, ty;              // Texture values.
} O3DF_TFaceVertex;

typedef struct {
    word        nVerts;
    sint16      material;   // -1 => not visible.
    dword       flags;      // May indicate, for example, that it's a split.
    sint32      tox, toy, tsx, tsy, ta;    // Texture data.
} O3DF_TFaceHeader;

typedef struct {
    O3DF_TFaceHeader h;
//    O3DF_TFaceVertex fverts[h.nVerts];
} O3DF_TFace;

typedef struct {
    byte    color;
    word    flags;          // Semi-transparent? Translucid? etc.
    sint32  ambient;        // Lighting parameters.
    sint32  diffuse;
    sint32  reflected;
    char    texname[8];     // Filename of the texture, or "" for flat.
} O3DF_TMaterial;

typedef struct {
    word nVerts;        // Are rotated and translated.
    word nNormals;      // Are rotated, but not translated.
    word nFaces;
    word nFaceVerts;
    word nMaterials;
        // Object loader can determine the amount of mem to alloc in a
        // single block, from the nXXX values above -> Less heap overhead.
    word flags;
    sint32 scx, scy, scz;       // Scale factors for the application to handle.
                                // Recommended format is 16.16.
                                // But note that they default to 0.
    sint32 dcx, dcy, dcz;       // Center for the application to handle.
                                // Recommended format is: same as vertices
                                // *before* scaling.
//    O3DF_TVertex   verts[nVerts+nNormals];
//    O3DF_TFace     faces[nFaces];
//    O3DF_TMaterial materials[nMaterials];
} O3DF_TObject;

// -----------------------------------------
// Memory image of the object.

typedef struct {
    byte    color;
    word    flags;          // Semi-transparent? Translucid? etc.
    sint32  ambient;        // Lighting parameters.
    sint32  diffuse;
    sint32  reflected;
    char    texname[8];     // Filename of the texture, or "" for flat.
    byte    *texture;       // Pointer to loaded texture mem.
} O3DM_TMaterial, *O3DM_PMaterial;

typedef struct {
    sint32  x, y, z;            // Pretty obvious.
    sint32  rx, ry, rz;
    sint32  px, py;
    sint32  l;                  // Calculated.
} O3DM_TVertex, *O3DM_PVertex;

typedef struct {
    sint32  x, y, z;            // Pretty obvious.
    sint32  rx, ry, rz;
    sint32  l;                  // Calculated
} O3DM_TNormal, *O3DM_PNormal;

typedef struct {
    O3DM_PVertex vert;          // 3D vertex index.
    O3DM_PNormal normal;        // Normal vector at this vertex.
    sint32 l;                   // Calculated
    sint32 tx, ty;              // Texture values.
} O3DM_TFaceVertex, *O3DM_PFaceVertex;

struct O3DM_SFace;          // Forward declaration.

typedef struct {
    bool            visible;    // Calculated.
    word            nVerts;
    dword           flags;      // May indicate, for example, that it's a split.
    O3DM_PMaterial  material;   // NULL => not to be drawn.
    sint32      tox, toy, tsx, tsy, ta;    // Texture data.
    struct O3DM_SFace *back, *front; // BSP links, or doubly linked list
                                           // of regular faces in the BSP leaf.
    sint32 depth;               // Calculated.
    struct O3DM_SFace *next;
} O3DM_TFaceHeader, *O3DM_PFaceHeader;

typedef struct O3DM_SFace {
    O3DM_TFaceHeader h;
    O3DM_TFaceVertex verts[1];
} O3DM_TFace, *O3DM_PFace;

typedef struct {
    word nVerts;        // Are rotated and translated.
    word nNormals;      // Are rotated, but not translated.
    word nFaces;
    word nFaceVerts;
    word nMaterials;
        // Object loader can determine the amount of mem to alloc in a
        // single block, from the nXXX values above -> Less heap overhead.
    word flags;
    O3DM_PVertex   verts;
    O3DM_PNormal   normals;
    O3DM_PFace     faces;
    O3DM_PMaterial materials;

    R3D_PPosVector   pos;
    R3D_PAngles      rot;
    sint32 scx, scy, scz;       // Scale factors for the application to handle.
                                // Recommended format is 16.16.
                                // But note that they default to 0.
    sint32 dcx, dcy, dcz;       // Center for the application to handle.
                                // Recommended format is: same as vertices
                                // *before* scaling.
} O3DM_TObject, *O3DM_PObject;

enum {
        // Material flags.
    O3DMF_NOSHADE = 0x0001,     // Don't apply any lightning.
    O3DMF_TRANS   = 0x0002,     // Translucent.
    O3DMF_HOLES   = 0x0004,     // Color 0 in the texture is transparent.
    O3DMF_256     = 0x0008,     // 256 wide bitmap

        // Face flags.
    O3DFF_FLAT    = 0x0001,     // Face is flat shaded (1st vertex' normal).
    O3DFF_NORDER  = 0x0002,     // Face should be rendered prior to the rest.
    O3DFF_VISIBLE = 0x0004,     // Should be though of as visible always.
};


PUBLIC int O3DM_MaxDetail;
PUBLIC int O3DM_OrderByZ;

enum {
    O3DD_FLAT,
    O3DD_GOURAUD,
    O3DD_TEXTURED,
    O3DD_TEXLIGHT,
    O3DD_TEXGOURAUD,

    O3DD_MAXDETAIL,
};

PUBLIC O3DM_PObject O3DM_LoadObject(const char *fname);
PUBLIC bool         O3DM_SaveObject(const char *fname, O3DM_PObject obj);

    // Delete all memory taken by an object.
PUBLIC void O3DM_DeleteObject(O3DM_PObject obj);

    // Make a copy of an object, one that can be modified easily.
PUBLIC O3DM_PObject O3DM_DupObj(O3DM_PObject obj);

#define SIZEFACE(f) (sizeof((f)->h) + sizeof((f)->verts[0])*(f)->h.nVerts)

#define O3DM_RotateObjVerts(obj,m)   (R3D_Rot3DVector(&(obj)->verts[0].rx,(m),&(obj)->verts[0].x,(obj)->nVerts,sizeof((obj)->verts[0])))
#define O3DM_RotateObjNormals(obj,m) (R3D_Rot3DVector(&(obj)->normals[0].rx,(m), &(obj)->normals[0].x,(obj)->nNormals,sizeof((obj)->normals[0])))
#define O3DM_TranslateObj(obj,p)     (R3D_Add3DVector(&(obj)->verts[0].rx,(p),(obj)->nVerts,sizeof((obj)->verts[0])))
#define O3DM_ProjectObject(obj)      (R3D_Project3D(&(obj)->verts[0].px,&(obj)->verts[0].rx,(obj)->nVerts, sizeof(obj->verts[0]), sizeof((obj)->verts[0])))

PUBLIC O3DM_PFace O3DM_OrderFaces(O3DM_PObject obj);
PUBLIC void O3DM_ParseVisibility (O3DM_PObject obj);

PUBLIC void O3DM_CalcLights      (O3DM_PObject obj, dword ang);
PUBLIC void O3DM_DrawFace        (O3DM_PFace pl);
PUBLIC void O3DM_Draw            (O3DM_PObject obj);

#endif

// ----------------------------- OBJECT3D.H ---------------------------

