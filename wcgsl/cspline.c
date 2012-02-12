// ----------------------------- CSPLINE.C ---------------------------
// For use with Watcom C.
// (C) Copyright 1995 by Jare & JCAB of Iguana.
// Spline functions.

#include <cspline.h>

#include <math.h>
#include <string.h>

typedef double TMatrix4[4][4];
typedef double (*PMatrix4)[4];

typedef double TMatrix3[3][3];
typedef double (*PMatrix3)[3];

PRIVATE double Determinant3(PMatrix3 m) {
    return m[0][0]*m[1][1]*m[2][2]
         + m[1][0]*m[2][1]*m[0][2]
         + m[0][1]*m[1][2]*m[2][0]
         - m[0][2]*m[1][1]*m[2][0]
         - m[0][1]*m[1][0]*m[2][2]
         - m[1][2]*m[2][1]*m[0][0];
}

PRIVATE double Determinant4(PMatrix4 m) {
    int i;
    double r, s;

    r = 0;
    s = 1;
    for (i = 0; i < 4; i++) {
        TMatrix3 m3;
        int j, l;
            // Build submatrix
        for (j = 0, l = 0; j < 3; j++) {
            if (i == l)
                l++;
            m3[0][j] = m[1][l];
            m3[1][j] = m[2][l];
            m3[2][j] = m[3][l];
            l++;
        }
        r = r + s*Determinant3(m3)*m[0][i];
        s = -s;
    }
    return r;
}

PRIVATE void PrepareMatrix(PMatrix4 dest, PMatrix4 m, double *c, int i) {
    memcpy(dest, m, sizeof(TMatrix4));
    dest[0][i] = c[0];
    dest[1][i] = c[1];
    dest[2][i] = c[2];
    dest[3][i] = c[3];
}

// --------------------------------------------------

PRIVATE double SPow2(double a) {return a*a;}
PRIVATE double SPow3(double a) {return a*a*a;}

// --------------------------------------------------

PUBLIC double CSP_CalcSlope(CSP_PSpline s, int i) {
    assert(s != NULL);
    assert(i >= 0);
    assert(i < s->npts);
    
    if (i == 0)
        return (s->pts[i+1].f - s->pts[i].f) /
               (s->pts[i+1].t - s->pts[i].t);
    else if (i == s->npts-1)
        return (s->pts[i].f - s->pts[i-1].f) /
               (s->pts[i].t - s->pts[i-1].t);
    else
        return (s->pts[i+1].f - s->pts[i-1].f) /
               (s->pts[i+1].t - s->pts[i-1].t);
/*
        return tan((atan((s->pts[i+1].f - s->pts[i].f) /
                         (s->pts[i+1].t - s->pts[i].t)) +
                    atan((s->pts[i].f - s->pts[i-1].f) /
                         (s->pts[i].t - s->pts[i-1].t)))/2);
*/
}

PUBLIC void CSP_CalcSpline(CSP_PSpline s) {
    int      i;
    double   da, d1;
    TMatrix4 ma, m1;
    double   b[4];

    assert(s != NULL);

    for (i = 0; i < s->npts; i++)
        s->pts[i].slope = CSP_CalcSlope(s, i);

/*
    Each segment is interpolated using a cubic equation that will give us four
    degrees of freedom: Those four values will be the values of the function
    and of its derivative at the starting and ending points of the segment.
    This means the cubic spline is C1 continuous.

        For segment i (i >= 0, i < s->npts-1)
    call x == s->pts[i  ].t   beginning of segment.
    call X == s->pts[i+1].t   end of segment (beginning of the next segment).

    The system to solve is

    ÚÄ               Ä¿ ÚÄ Ä¿   ÚÄ     Ä¿
    ³  x^3  x^2  x  1 ³ ³ a ³   ³  f(x) ³
    ³  X^3  X^2  X  1 ³ú³ b ³ = ³  f(X) ³
    ³ 3x^2 2x    1  0 ³ ³ c ³   ³ f'(x) ³
    ³ 3X^2 2X    1  0 ³ ³ d ³   ³ f'(X) ³
    ÀÄ               ÄÙ ÀÄ ÄÙ   ÀÄ     ÄÙ

        We do it using Kramer's rule.
*/

        // Constant values in the linear system matrix
    ma[0][3] = 1;
    ma[1][3] = 1;
    ma[2][2] = 1;
    ma[3][2] = 1;
    ma[2][3] = 0;
    ma[3][3] = 0;
    for (i = 0; i < s->npts-1; i++) {
        b[0] = s->pts[i  ].f;
        b[1] = s->pts[i+1].f;
        b[2] = s->pts[i  ].slope;
        b[3] = s->pts[i+1].slope;
        ma[0][2] =   s->pts[i  ].t;
        ma[1][2] =   s->pts[i+1].t;
        ma[0][1] =   SPow2(s->pts[i  ].t);
        ma[1][1] =   SPow2(s->pts[i+1].t);
        ma[2][1] = 2*s->pts[i  ].t;
        ma[3][1] = 2*s->pts[i+1].t;
        ma[0][0] =   SPow3(s->pts[i  ].t);
        ma[1][0] =   SPow3(s->pts[i+1].t);
        ma[2][0] = 3*SPow2(s->pts[i  ].t);
        ma[3][0] = 3*SPow2(s->pts[i+1].t);
        da = Determinant4(ma);

            // Solve system.
        PrepareMatrix(m1, ma, b, 0);
        d1 = Determinant4(m1);
        s->pts[i].a = d1/da;

        PrepareMatrix(m1, ma, b, 1);
        d1 = Determinant4(m1);
        s->pts[i].b = d1/da;

        PrepareMatrix(m1, ma, b, 2);
        d1 = Determinant4(m1);
        s->pts[i].c = d1/da;

        PrepareMatrix(m1, ma, b, 3);
        d1 = Determinant4(m1);
        s->pts[i].d = d1/da;
    }
}

PUBLIC int CSP_FindSegment(CSP_PSpline s, double t) {
    int i;
    assert(s != NULL);
    if (t > s->pts[s->npts-1].t)
        return s->npts-2;
    for (i = 0; i < s->npts-1; i++) {
        if (t >= s->pts[i].t && t <= s->pts[i+1].t)
            return i;
    }
    return 0;
}

PUBLIC double CSP_Interpolate(CSP_PPoint p, double t) {
    assert(p != NULL);
    return p->a*SPow3(t) + p->b*SPow2(t) + p->c*t + p->d;
}

// ----------------------------- CSPLINE.C ---------------------------

