// -------------------- ATAN.C ---------------------
// Arc tangent math function in fixed point.
// (C) Copyright 1994-5 by Jare & JCAB of Iguana-VangeliSTeam.

#include "atan.h"




extern uint16 ArcTanTable[2049];



uint16 GetAngle(sint32 x, sint32 y)
{
    if (y >= 0) {
        if (x >= 0) {
            if (x >= y) {
                if (x == 0 && y == 0)
                    return 0;
                return ArcTanTable[FPnDiv(y, x, 11)];
            } else
                return 16384 - ArcTanTable[FPnDiv(x, y, 11)];
        } else {
            if (-x >= y)
                return 32768 - ArcTanTable[FPnDiv(y, -x, 11)];
            else
                return 16384 + ArcTanTable[FPnDiv(-x, y, 11)];
        }
    } else {
        if (x >= 0) {
            if (x >= -y)
                return 65536 - ArcTanTable[FPnDiv(-y, x, 11)];
            else
                return 49158 + ArcTanTable[FPnDiv(x, -y, 11)];
        } else {
            if (-x >= -y)
                return 32768 + ArcTanTable[FPnDiv(y, x, 11)];
            else
                return 49152 - ArcTanTable[FPnDiv(x, y, 11)];
        }
    }
}


uint16 Atan(sint32 tan)
{
    return GetAngle(0x10000, tan);
}



