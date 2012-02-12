// -------------------------------- 3DOBJ.C -----------------------------------
// For use with Watcom C.
// (C) Copyright 1994-5 by Jare & JCAB of Iguana-VangeliSTeam.

// Simple 3D object routines.

#include <3dobj.h>
#include <polygon.h>

#define MINDL 1600
#define MAXDL 16000

//#define ORDER_FACES


PRIVATE O3D_PFace OrderFaces(O3D_PSmallObject obj) {
    static O3D_PFace first;
    int i, j;
    sint32 z;

    first = NULL;
#ifdef ORDER_FACES
    for (i = 0; i < obj->sobNFaces; i++) {
        O3D_PFace f, g, *p;

        f = &obj->sobFaces[i];
        if ( (f->facColor == 0) || !f->facVisible)
            continue;

            // calc face Z

        z = obj->sobRotPts[f->facPts[0]][2];
        for (j = 1; j < f->facNPoints; j++)
            z += obj->sobRotPts[f->facPts[j]][2];
        z /= f->facNPoints;
        f->facDepth = z;

            // Insertion in Z-ordered list.
        p = &first;
        while (*p != NULL) {
            if ((*p)->facDepth <= z)
                break;
            p = &((*p)->next);
        }
        g = *p;
        *p = f;
        (*p)->next = g;
    }
#else
    for (i = 0; i < obj->sobNFaces; i++) {
        O3D_PFace f;

        f = &obj->sobFaces[i];
        if ( (f->facColor == 0) || !f->facVisible)
            continue;
        f->next = first;
        first = f;
    }
#endif
    return first;
}

void O3D_InitObject(O3D_PSmallObject obj) {
    obj->sobRotPts  = NEW(sizeof(*obj->sobRotPts) *obj->sobNPoints);
    obj->sobProjPts = NEW(sizeof(*obj->sobProjPts)*obj->sobNPoints);
}

void O3D_RotateObject(O3D_PSmallObject obj) {
    int i;

    R3D_Gen3DMatrix(&obj->sobM, obj->sobAngles);

    R3D_Rot3DVector(obj->sobRotPts, &obj->sobM,  obj->sobPoints, obj->sobNPoints);
    R3D_Add3DVector(obj->sobRotPts, obj->sobPos, obj->sobRotPts, obj->sobNPoints);
/*
    for (i = 0; i < obj->sobNPoints; i++) {
        R3D_Rot3DVector(obj->sobRotPts[i], &obj->sobM,  obj->sobPoints[i]);
        R3D_Add3DVector(obj->sobRotPts[i], obj->sobPos, obj->sobRotPts[i]);
    }
*/
}

void O3D_ProjectObject(O3D_PSmallObject obj)
{
    int i;

    R3D_Project3D(obj->sobProjPts, obj->sobRotPts, obj->sobNPoints);
/*
    for (i = 0; i < obj->sobNPoints; i++) {
        R3D_Project3D(obj->sobProjPts[i], obj->sobRotPts[i]);
        obj->sobProjPts[i][1] = obj->sobProjPts[i][1] * 200L / 240;
    }
*/
}

void O3D_ParseVisibility(O3D_PSmallObject obj) {
    int i;

    for (i = 0; i < obj->sobNFaces; i++) {
        obj->sobFaces[i].facVisible =
            (( obj->sobProjPts[obj->sobFaces[i].facPts[2]][0]
              -obj->sobProjPts[obj->sobFaces[i].facPts[1]][0])
            *( obj->sobProjPts[obj->sobFaces[i].facPts[0]][1]
              -obj->sobProjPts[obj->sobFaces[i].facPts[1]][1])
            -( obj->sobProjPts[obj->sobFaces[i].facPts[2]][1]
              -obj->sobProjPts[obj->sobFaces[i].facPts[1]][1])
            *( obj->sobProjPts[obj->sobFaces[i].facPts[0]][0]
              -obj->sobProjPts[obj->sobFaces[i].facPts[1]][0]))
             <= 0;
    }
}

void O3D_DrawM13(O3D_PSmallObject obj) {
    int i, j, k;
    O3D_PFace pl;
    pl = OrderFaces(obj);
    while (pl != NULL) {
        sint32 dl;
        k = 0;
        dl = 0;
        for (j = pl->facNPoints-1; j >= 0 ; j--) {
            sint32 l;

            l = obj->sobRotPts[pl->facPts[j]][2];
            if (l < MINDL)
                dl += 0x1F0000;
            else if (l > MAXDL)
                dl += 0x000000;
            else
                dl += FPMultDiv(0x1F0000,(MAXDL-l),(MAXDL-MINDL));

            POLY_ScrapPoly[k].x = obj->sobProjPts[pl->facPts[j]][0];
            POLY_ScrapPoly[k].y = obj->sobProjPts[pl->facPts[j]][1];
            k++;
        }
        POLY_ScrapPoly[0].l = dl/pl->facNPoints;
        POLY_SolidDraw(POLY_ScrapPoly, pl->facNPoints, pl->facColor);
        pl = pl->next;
    }
}

void O3D_DrawShadeM13   (O3D_PSmallObject obj) {
    int i, j, k;
    O3D_PFace pl;
    pl = OrderFaces(obj);
    while (pl != NULL) {
       k = 0;
       for (j = pl->facNPoints-1; j >= 0 ; j--) {
            sint32 l;

            l = obj->sobRotPts[pl->facPts[j]][2];
            if (l < MINDL)
                l = 0x1F0000;
            else if (l > MAXDL)
                l = 0x000000;
            else
                l = FPMultDiv(0x1F0000,(MAXDL-l),(MAXDL-MINDL));

            POLY_ScrapPoly[k].x = obj->sobProjPts[pl->facPts[j]][0];
            POLY_ScrapPoly[k].y = obj->sobProjPts[pl->facPts[j]][1];
            POLY_ScrapPoly[k].l  = l;
            k++;
        }
        POLY_ShadeDraw(POLY_ScrapPoly, pl->facNPoints, pl->facColor);
        pl = pl->next;
    }
}

void O3D_DrawTextureM13(O3D_PSmallObject obj) {
    int i, j, k;
    O3D_PFace pl;
    pl = OrderFaces(obj);
    while (pl != NULL) {
       k = 0;
       for (j = pl->facNPoints-1; j >= 0 ; j--) {
            POLY_ScrapPoly[k].x = obj->sobProjPts[pl->facPts[j]][0];
            POLY_ScrapPoly[k].y = obj->sobProjPts[pl->facPts[j]][1];
            POLY_ScrapPoly[k].tx = pl->facTexturePts[2*j+0];
            POLY_ScrapPoly[k].ty = pl->facTexturePts[2*j+1];
            k++;
        }
        POLY_TextureDraw(POLY_ScrapPoly, pl->facNPoints, pl->facTexture);
        pl = pl->next;
    }
}

void O3D_DrawLightTexM13(O3D_PSmallObject obj) {
    int i, j, k;
    O3D_PFace pl;
    pl = OrderFaces(obj);
    while (pl != NULL) {
        sint32 dl;
        k = 0;
        dl = 0;
        for (j = pl->facNPoints-1; j >= 0 ; j--) {
            sint32 l;

            l = obj->sobRotPts[pl->facPts[j]][2];
            if (l < MINDL)
                dl += 0x1F0000;
            else if (l > MAXDL)
                dl += 0x000000;
            else
                dl += FPMultDiv(0x1F0000,(MAXDL-l),(MAXDL-MINDL));

            POLY_ScrapPoly[k].x = obj->sobProjPts[pl->facPts[j]][0];
            POLY_ScrapPoly[k].y = obj->sobProjPts[pl->facPts[j]][1];
            POLY_ScrapPoly[k].tx = pl->facTexturePts[2*j+0];
            POLY_ScrapPoly[k].ty = pl->facTexturePts[2*j+1];
            k++;
        }
        POLY_ScrapPoly[0].l = dl/pl->facNPoints;
        POLY_LightTexDraw(POLY_ScrapPoly, pl->facNPoints, pl->facTexture);
        pl = pl->next;
    }
}

void O3D_DrawShadeTexM13(O3D_PSmallObject obj) {
    int i, j, k;
    O3D_PFace pl;
    pl = OrderFaces(obj);
    while (pl != NULL) {
       k = 0;
       for (j = pl->facNPoints-1; j >= 0 ; j--) {
            sint32 l;

            l = obj->sobRotPts[pl->facPts[j]][2];
            if (l < MINDL)
                l = 0x1F0000;
            else if (l > MAXDL)
                l = 0x000000;
            else
                l = FPMultDiv(0x1F0000,(MAXDL-l),(MAXDL-MINDL));

            POLY_ScrapPoly[k].x = obj->sobProjPts[pl->facPts[j]][0];
            POLY_ScrapPoly[k].y = obj->sobProjPts[pl->facPts[j]][1];
            POLY_ScrapPoly[k].tx = pl->facTexturePts[2*j+0];
            POLY_ScrapPoly[k].ty = pl->facTexturePts[2*j+1];
            POLY_ScrapPoly[k].l  = l;
            k++;
        }
        POLY_ShadeTexDraw(POLY_ScrapPoly, pl->facNPoints, pl->facTexture);
        pl = pl->next;
    }
}

// ---------------------------- End of 3DOBJ.C --------------------------------
