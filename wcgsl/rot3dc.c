// ----------------------- ROT3DC.C -------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-5 by Jare & JCAB of Iguana-VangeliSTeam.
// 3D basic routines.

#include <rot3d.h>
#include <sqrt.h>
#include <atan.h>
#include <sincos.h>

/*
extern void R3D_Gen3DVectorMatrix(R3D_PRotMatrix dest, R3D_PPosVector vec) {
    R3D_TPosVector rz, rx, ry;
    uint32 m;

        // Normalize vectors.
    m = SQR_Sqrt(Pow2(vec[0]) + Pow2(vec[1]) + Pow2(vec[2]));
    if (m > 0) {
        rz[0] = FPnDiv(vec[0],m,30);    // Normal is in 2.30 format.
        rz[1] = FPnDiv(vec[1],m,30);
        rz[2] = FPnDiv(vec[2],m,30);
    } else
        rz[0] = rz[1] = rz[2] = 0;

    m = SQR_Sqrt(Pow2(vec[2]) + Pow2(vec[0]));
    if (m > 0) {
        rx[0] = FPnDiv(-vec[2],m,30);    // Normal is in 2.30 format.
        rx[1] = 0;
        rx[2] = FPnDiv(vec[0],m,30);
    } else
        rx[0] = rx[1] = rx[2] = 0;

    R3D_CrossProduct(ry, rz, rx, 30);
    dest[0*3+0] = rx[0]; dest[0*3+1] = ry[0]; dest[0*3+2] = rz[0];
    dest[1*3+0] = rx[1]; dest[1*3+1] = ry[1]; dest[1*3+2] = rz[1];
    dest[2*3+0] = rx[2]; dest[2*3+1] = ry[2]; dest[2*3+2] = rz[2];
}
*/

extern void R3D_Vec2Angles(R3D_PAngles dest, R3D_PPosVector from, R3D_PPosVector target) {
    sint32 r, c, s;
    R3D_TPosVector p;

    p[0] = target[0] - from[0];
    p[1] = target[1] - from[1];
    p[2] = target[2] - from[2];

    dest[2] = GetAngle(p[2], -p[0]);
    c = Cos(dest[2]);
    s = Sin(dest[2]);
    if (Abs32(c) > Abs32(s))
        r = FPDiv(p[2], c);      // r = Z/cos é
    else
        r = FPDiv(p[0], s);      // r = -X/sin é
    dest[1] = GetAngle(r, p[1]);
}

/*
extern void R3D_CrossProduct(R3D_PPosVector dest, R3D_PPosVector v1, R3D_PPosVector v2, int dec) {
    R3D_TPosVector p;

    if (dest == v1) {
        memcpy(&p, v1, sizeof(p));
        v1 = p;
    }
    if (dest == v2) {                   // If both are true doesn't matter.
        memcpy(&p, v2, sizeof(p));
        v2 = p;
    }
    dest[0] = FPnMult(v1[2],v2[1],dec) - FPnMult(v1[1],v2[2],dec);
    dest[1] = FPnMult(v1[0],v2[2],dec) - FPnMult(v1[2],v2[0],dec);
    dest[2] = FPnMult(v1[1],v2[0],dec) - FPnMult(v1[0],v2[1],dec);
}
*/

extern R3D_TIntValue R3D_Module(R3D_PPosVector v) {
    return SQR_Sqrt(Pow2(v[0]) + Pow2(v[1]) + Pow2(v[2]));
}


// ----------------------- ROT3DC.C -------------------------

