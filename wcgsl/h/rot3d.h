// ----------------------- ROT3D.H -------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-5 by Jare & JCAB of Iguana-VangeliSTeam.

// 3D basic routines.

#ifndef _ROT3D_H_
#define _ROT3D_H_

#ifndef _BASE_H_
#include <base.h>
#endif

typedef sint32    R3D_TTrigValue;
typedef sint32    R3D_TIntValue;
typedef sint32    R3D_TProjValue;
typedef word      R3D_TAngleValue;

typedef R3D_TTrigValue  R3D_TRotMatrix[3*3];
typedef R3D_TTrigValue *R3D_PRotMatrix;

typedef R3D_TIntValue   R3D_TPosVector[3];
typedef R3D_TIntValue  *R3D_PPosVector;

typedef R3D_TAngleValue  R3D_TAngles[3];
typedef R3D_TAngleValue *R3D_PAngles;

typedef R3D_TProjValue  R3D_TProjPos[2];
typedef R3D_TProjValue *R3D_PProjPos;

/* -----------------------------------
 � Rotation is as follows: First rotation, around Z axis (deep).
 �                         Second, around X axis (lateral).
 �                         Third, around Y axis (vertical).
 � a = Roll(Z), b = Pitch(X), c = Yaw(Y)

     ��                 Ŀ      ��                 Ŀ      ��                 Ŀ
     �  COSa  SINa     0 �      �     1     0     0 �      �  COSc     0 -SINc �
Ra � � -SINa  COSa     0 � Rb � �     0  COSb  SINb � Rc � �     0     1     0 �
     �     0     0     1 �      �     0 -SINb  COSb �      �  SINc     0  COSc �
     ��                 ��      ��                 ��      ��                 ��

 RM = Rc�Rb�Ra And the inverse will be RM' = Ra'�Rb'�Rc' (' � -1)
 As the book says, Ra' � R(-a) � Ra` (` � T) for any axis, and
 holds true for any composite rotation matrix: R' � R`

         ��                           Ŀ
         �  COSa       SINa          0 �
 Rb�Ra = � -COSb�SINa  COSb�COSa  SINb � , and finally
         �  SINb�SINa -SINb�COSa  COSb �
         ��                           ��

             ��                 Ŀ ��                           Ŀ
             �  COSc     0 -SINc � �  COSa       SINa          0 �
 Rc�Rb�Ra =  �     0     1     0 ��� -COSb�SINa  COSb�COSa  SINb � =
             �  SINc     0  COSc � �  SINb�SINa -SINb�COSa  COSb �
             ��                 �� ��                           ��
   ��                                                               Ŀ
   �  COSc�COSa-SINc�SINb�SINa  COSc�SINa+SINc�SINb�COSa  -SINc�COSb �
 = � -COSb�SINa                 COSb�COSa                  SINb      �
   �  SINc�COSa+COSc�SINb�SINa  SINc�SINa-COSc�SINb�COSa   COSc�COSb �
   ��                                                               ��

   The rotation matrix implemented uses a strange approach with all the
   angles negated, because the library's Sin() returns -Sin(). It's just
   a matter of signs, and while you keep it uniform there's no problem.

 �      ��                                                              Ŀ
 �      �  COSc*COSa+SINc*SINb*SINa -COSc*SINa+SINc*SINb*COSa  SINc*COSb �
 � RM � �  COSb*SINa                 COSb*COSa                -SINb      �
 �      � -SINc*COSa+COSc*SINb*SINa  SINc*SINa+COSc*SINb*COSa  COSc*COSb �
 �      ��                                                              ��
 � Mode of operation:
 �   Calc 3D Matrix from your set of Roll, Pitch, Yaw angles.
 �   Rot your 3-D, 0-centered vertex, mult'ing by that matrix.
 �   Add the coordinates of the 0-center of the object to it.

 That makes the object positioned in the virtual world. Now, to display
 it as seen from a given camera, you must create the camera's matrices
 using the inverses (because you're not translating the camera into the
 world, but rather the world into the camera. Therefore, a given point Vw
 in the world will pass through this process:

        Vc = Rc`�(Vw-Vc) The point had been calculated through
        Vw = Vo + Ro�V hence
        Vc = Rc`�(Vo + Ro�V - Vc) = Rc`�Vo + Rc`�Ro�V - Rc`�Vc
    which, for placing a given object in the camera, means
        Vco = Rc`�(Vo-Vc)
        Rco = Rc`�Ro
    and, for all Vi vertices in an object, the operation to carry is
        V = Rco�Vi + Vco
    and all steps are summed in one.

    Calculate Vco first and you might avoid the rest if, say, the object
    is behind the camera (Vco.z < 0) or such. Also, if you're going to order
    the objects by depth, you will use Vco too.
    If many objects are going to rotate by the same angles, or none at all,
    like the buildings of a city, you may want to calc Rco once for all
    of them. Vco still needs to be calculated for each of them.
*/

//  ----------------------------

PUBLIC R3D_TRotMatrix R3D_MIdentity;
PUBLIC R3D_TPosVector R3D_PZero;

    // Constants for the projection. Default to (256,256*5/6), (160,100).
PUBLIC sint32 R3D_FocusX, R3D_FocusY, R3D_CenterX, R3D_CenterY;

//  ----------------------------

extern void R3D_CrossProduct(R3D_PPosVector dest, R3D_PPosVector u,
                             R3D_PPosVector v, int prec);
#pragma aux R3D_CrossProduct parm caller [EDI] [ESI] [EBX] [ECX]
   //        ***********
   // Vector cross product.

extern void R3D_Gen3DMatrix(R3D_PRotMatrix dest, R3D_PAngles angs);
#pragma aux R3D_Gen3DMatrix parm caller [EDI] [ESI]
   //        ***********
   // Fill rotation matrix with angle rotation values.

extern void R3D_Gen3DInverseMatrix(R3D_PRotMatrix dest, R3D_PAngles angs);
#pragma aux R3D_Gen3DInverseMatrix parm caller [EDI] [ESI]
   //        ***********
   // Fill inverse rotation matrix with angle rotation values.

//extern void R3D_Gen3DVectorMatrix(R3D_PRotMatrix dest, R3D_PPosVector vec, R3D_PPosVector up);
extern void R3D_Gen3DVectorMatrix(R3D_PRotMatrix dest, R3D_PPosVector vec);
   //        ***********
   // Fill inverse rotation matrix with direction and up vector.

extern void R3D_Vec2Angles(R3D_PAngles dest,
                           R3D_PPosVector from, R3D_PPosVector target);
   //        ***********
   // Calc angles b & c (Y & X) for looking from 'from' to 'target'.

extern void R3D_Rot3DMatrix(R3D_PRotMatrix dest, R3D_PRotMatrix m1, R3D_PRotMatrix m2);
#pragma aux R3D_Rot3DMatrix parm caller [EDI] [ESI] [EBX]
   //        ***********
   // Matrix multiply dest = m1�m2.

extern void R3D_Rot3DVector(R3D_PPosVector dest, R3D_PRotMatrix m, R3D_PPosVector v, int n, int size);
#pragma aux R3D_Rot3DVector parm caller [EDI] [ESI] [EBX] [ECX] [EDX]
   //        ***********
   // Vektor multiply by matrix dest = m�v.

extern void R3D_Add3DVector(R3D_PPosVector dest, R3D_PPosVector v1, int n, int size);
#pragma aux R3D_Add3DVector parm caller [EDI] [ESI] [ECX] [EBX]
   //        ***********
   // Add vektors dest = dest�v1.

extern void R3D_Project3D(R3D_PProjPos dest, R3D_PPosVector v, int n, int size1, int size2);
#pragma aux R3D_Project3D   parm caller [EDI] [ESI] [ECX] [EBX] [EDX]
   //        *********
   // Projects the 3D point v into the 2D point dest.

/*
extern void R3D_CrossProduct(R3D_PPosVector dest,
                             R3D_PPosVector v1, R3D_PPosVector v2,
                             int dec);
*/

#endif

// ----------------------- ROT3D.H -------------------------
