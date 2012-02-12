// ----------------------------- CSPLINE.H ---------------------------
// For use with Watcom C.
// (C) Copyright 1995 by Jare & JCAB of Iguana.

// Spline functions.

#ifndef _CSPLINE_H_
#define _CSPLINE_H_

#ifndef _BASE_H_
#include <base.h>
#endif

typedef struct {
    double f,           // Function value at this control point.
           t,           // Function parameter at this control point.
           slope,       // Slope of function.
           a, b, c, d;  // Cubic function (ax3+bx2+cx+d) values from this
                        // Control point to the next.
} CSP_TPoint, *CSP_PPoint;

typedef struct {
    sint32     npts;    // real size of pt[] array.
    CSP_TPoint pts[1];
} CSP_TSpline, *CSP_PSpline;

#define CSP_Size(n) (sizeof(sint32) + sizeof(CSP_TPoint)*(n))

PUBLIC double CSP_CalcSlope(CSP_PSpline s, int i);

PUBLIC void CSP_CalcSpline(CSP_PSpline s);

PUBLIC int CSP_FindSegment(CSP_PSpline s, double t);

PUBLIC double CSP_Interpolate(CSP_PPoint p, double t);

#endif

// ----------------------------- CSPLINE.H ---------------------------

