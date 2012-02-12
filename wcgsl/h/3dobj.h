// ----------------------------- 3DOBJ.H ------------------------------
// For use with Watcom C 9.0 and Tran's PMode.
// (C) Copyright 1993 by Jare & JCAB of Iguana.

// Simple 3D object routines.

#ifndef _3DOBJ_H_
#define _3DOBJ_H_

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

    typedef struct {
        sint32  x, y, z;            // Pretty obvious.
    } O3DF_TVertex, O3DF_TNormal;

    typedef struct {
        sint32  x, y, z;            // Pretty obvious.
        sint32  rx, ry, rz;
        sint32  px, py;
    } O3DM_TVertex;

    typedef struct {
        sint32  x, y, z;            // Pretty obvious.
        sint32  rx, ry, rz;
        sint32  l;
    } O3DM_TNormal;

    typedef struct {
        sint16 nvert;               // 3D vertex index.
        sint16 nnorm;               // Normal vector at this vertex.
        sint32 tx, ty;              // Texture values.
    } O3DF_TFaceVertex;

    typedef struct {
        O3DM_PVertex vert;          // 3D vertex index.
        O3DM_PNormal norm;          // Normal vector at this vertex.
        sint32 tx, ty;              // Texture values.
    } O3DM_TFaceVertex;

    typedef struct {
        word        nVerts;
        sint16      material;   // -1 => not visible.
        dword       flags;      // May indicate, for example, that it's a split.
    } O3DF_TFaceHeader;

    typedef struct O3DM_SFaceHeader {
        bool            visible;    // Calculated.
        word            nVerts;
        O3DM_PMaterial *material;   // NULL => not to be drawn.
        dword           flags;      // May indicate, for example, that it's a split.
        struct O3DM_SFaceHeader *neg, *pos; // BSP links, or linked list
                                            // in a BSP node.
    } O3DM_TFaceHeader;

    typedef struct {
        O3DF_TFaceHeader h;
        O3DF_TFaceVertex fverts[h.nVerts];
    } O3DF_TFace;

    typedef struct {
        O3DM_TFaceHeader h;
        O3DM_TFaceVertex fverts[1];
    } O3DM_TFace;

    typedef struct {
        byte    color;
        word    flags;          // Semi-transparent? Translucid? etc.
        sint32  ambient;        // Lighting parameters.
        sint32  diffuse;
        sint32  reflected;
        char    texname[8];     // Filename of the texture, or "" for flat.
    } O3DF_TMaterial;

    typedef struct {
        byte    color;
        word    flags;          // Semi-transparent? Translucid? etc.
        sint32  ambient;        // Lighting parameters.
        sint32  diffuse;
        sint32  reflected;
        byte    *texname;       // Filename of the texture, or "" for flat.
    } O3DM_TMaterial;

    typedef struct {
        word nVerts;        // Are rotated and translated.
        word nNormals;      // Are rotated, but not translated.
        word nFaces;
        word nFaceVerts;
        word nMaterials;
            // Object loader can determine the amount of mem to alloc in a
            // single block, from the nXXX values above -> Less heap overhead.
        word flags;
        O3DF_TVertex   verts[nVerts+nNormals];
        O3DF_TFace     faces[nFaces];
        O3DF_TMaterial materials[nMaterials];
    } O3DF_TObject;

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

        R3D_PRotMatrix   rm;
    } O3DM_TObject;

    Skeleton file: were a value seems redundant, it will be used for
          consistency check, and won't appear in the binary version.

    Object "name"
    Vertices # Normals # Faces # FaceVertices # Materials #
    Flags 0x0000
    Vertex # X # Y # Z #
     ...
    Normal # X # Y # Z #
     ...
    Face # Vertices # Material # Flags # Array
        Vertex # Normal # TextureX # TextureY #
         ...
     ...
    Material # Color # Flags # Ambient # Diffuse # Reflected # Texture "tname"

*/

typedef struct O3D_SFaceVertex {
    word   nvert,
           nnorm;
    sint32 tx, ty;
    sint32 l;
} O3D_TFaceVertex, *O3D_PFaceVertex;

typedef struct O3D_SFace {
    byte        facNPoints;
    byte        facColor;
    word        facVisible;
    int         facPts[10];
    byte       *facTexture;
    sint32      facTexturePts[2*10];

    sint32      facDepth;
    struct O3D_SFace *next;
} O3D_TFace, *O3D_PFace;

typedef struct {
    R3D_TAngleValue *sobAngles;
    R3D_TIntValue   *sobPos;
    word             sobNPoints;
//    word             sobNVerts;
//    word             sobNNormals;
    word             sobNFaces;
    R3D_TPosVector  *sobPoints;
    O3D_TFace       *sobFaces;
    R3D_TPosVector  *sobRotPts;
    R3D_TProjPos    *sobProjPts;
    R3D_TRotMatrix   sobM;
} O3D_TSmallObject, *O3D_PSmallObject;

void O3D_InitObject     (O3D_PSmallObject obj);
void O3D_RotateObject   (O3D_PSmallObject obj);
void O3D_ProjectObject  (O3D_PSmallObject obj);
void O3D_ParseVisibility(O3D_PSmallObject obj);
void O3D_DrawShadeM13   (O3D_PSmallObject obj);
void O3D_DrawTextureM13 (O3D_PSmallObject obj);
void O3D_DrawLightTexM13(O3D_PSmallObject obj);
void O3D_DrawShadeTexM13(O3D_PSmallObject obj);
void O3D_DrawM13        (O3D_PSmallObject obj);
void O3D_DrawMX         (O3D_PSmallObject obj);
void O3D_DrawM16        (O3D_PSmallObject obj);


#endif
// ------------------------ End of 3DOBJ.H ----------------------------
