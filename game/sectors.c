// ------------------------------ SECTORS.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "sectors.h"
#include <stdio.h>
#include <string.h>
#include <strparse.h>
#include <jclib.h>
#include <atan.h>

#include <llscreen.h>
#include <sincos.h>
#include <polygon.h>
#include <vertdraw.h>

#include "globals.h"
#include "race.h"

PRIVATE bool GetLine(FILE *f, char *line, int n) {
    while (TRUE) {
        {
            char *p = line;
            n--;
            while (n > 1) {
                int c;
                c = fgetc(f);
                if (c == '\xa')
                    continue;
                *p++ = c;
                n--;
                if (c == '\xd')
                    break;
            }
            if (n > 0)
                *p = '\0';
        }
/*
        if (fgets(line, n, f) == EOF)
            return FALSE;
*/
        STRP_CleanLine(line, line);
        if (line[0] != '\0' && line[0] != ';')
            break;
    }
    return TRUE;
}

PUBLIC bool SEC_LoadMap(SEC_PSectorMap sec, const char *name) {
    int i, nsv;
    FILE *f;
    char line[200];

    assert(sec != NULL);

    sec->tnames = NULL;

//    f = JCLIB_OpenText(name);
    f = JCLIB_Open(name);
    if (f == NULL)
        return FALSE;
    REQUIRE(GetLine(f, line, sizeof(line)));
    sscanf(line, "%i %i %i", &sec->nv, &sec->ns, &nsv);
    sec->ns++;      // Add room for default sector.
    sec->verts = NEW(sec->nv*sizeof(*sec->verts));
    REQUIRE(sec->verts != NULL);
    memset(sec->verts, 0, sec->nv*sizeof(*sec->verts));
    sec->secs = NEW(sec->ns*sizeof(*sec->secs));
    REQUIRE(sec->secs != NULL);
    memset(sec->secs, 0, sec->ns*sizeof(*sec->secs));
    sec->slist = NEW(sec->ns*sizeof(*sec->slist));
    REQUIRE(sec->slist != NULL);

    REQUIRE(GetLine(f, line, sizeof(line)));
    REQUIRE(strcmpi(line, "Vertices") == 0);
    for (i = 0; i < sec->nv; i++) {
        REQUIRE(GetLine(f, line, sizeof(line)));
        sscanf(line, "%i %i", &sec->verts[i].x, &sec->verts[i].y);
    }

    REQUIRE(GetLine(f, line, sizeof(line)));
    REQUIRE(strcmpi(line, "Sectors") == 0);
    sec->secs[0].flags = 0;
    sec->secs[0].nv    = 0;
    sec->secs[0].v     = NULL;
    for (i = 1; i < sec->ns; i++) {
        int j;
        REQUIRE(GetLine(f, line, sizeof(line)));
        sscanf(line, "%i %i", &sec->secs[i].nv, &sec->secs[i].flags);
        if (!((sec->secs[i].v = NEW(sec->secs[i].nv*sizeof(*sec->secs[i].v))) != NULL))
            BASE_Abort("Out of memory for sector map");
        for (j = 0; j < sec->secs[i].nv; j++) {
            char *tok[4];
            NL_PName n;
            int k;

            line[0] = 0;
            REQUIRE(GetLine(f, line, sizeof(line)));
            if ( (k = STRP_SplitLine(tok, SIZEARRAY(tok), line)) != SIZEARRAY(tok))
                BASE_Abort("Reading %s, found %d tokens.\n", name, k);
            sscanf(tok[0], "%i", &k);
            REQUIRE(k >= 0);
            REQUIRE(k < sec->nv);
            sec->secs[i].v[j].v0 = sec->verts + k;
            sscanf(tok[1], "%i", &k);
            REQUIRE(k >= 0);
            REQUIRE(k < sec->nv);
            sec->secs[i].v[j].v1 = sec->verts + k;

            sec->secs[i].v[j].angle
                = GetAngle(sec->secs[i].v[j].v1->x - sec->secs[i].v[j].v0->x,
                           -(sec->secs[i].v[j].v1->y - sec->secs[i].v[j].v0->y));
            if (tok[2][0] == '\0') {// || !WallsDetail
                sec->secs[i].v[j].tex = NULL;
            } else {
                n = NL_AddName(&sec->tnames, tok[2]);
                REQUIRE(n != NULL);
                if (n->data == NULL) {
                    char buf[200];
                    sprintf(buf, "%.8s.is2", tok[2]);
            //        printf("Reading guardrail texture \"%s\".\n", buf);
                    {
                        long l = ftell(f);
                        n->data = IS2_Load(buf);
                        fseek(f, l, SEEK_SET);
                    }
                    if (n->data == NULL)
                        BASE_Abort("Trying to read guardrail texture \"%s\"", buf);
                }
                sec->secs[i].v[j].tex = n->data;
            }
            sscanf(tok[3], "%i", &k);
            REQUIRE(k >= -1);
            REQUIRE(k < sec->ns-1);
            sec->secs[i].v[j].otherside = sec->secs + k + 1;
        }
    }

    REQUIRE(GetLine(f, line, sizeof(line)));
    REQUIRE(strcmpi(line, "End") == 0);
    JCLIB_Close(f);
    return TRUE;
}

PUBLIC bool SEC_IsInSector(SEC_PSectorMap sec, SEC_PSector s, uint32 x, uint32 y) {
    int nhits = 0;
    int i;

    if (sec == NULL || s == NULL)
        return FALSE;

    for (i = 0; i < s->nv; i++) {
        SEC_PVertex v0, v1;

        v0 = s->v[i].v0;
        v1 = s->v[i].v1;
        if (((x >= v0->x && x < v1->x) || (x < v0->x && x >= v1->x))    // XRange
           ) {
            if (v0->y < y && v1->y < y)         // Totally above
                nhits++;
            else if (v0->y < y || v1->y < y) {  // Not below
                sint32 dy;
                if (v0->x < v1->x)
                    dy = v0->y + FPMultDiv(x-v0->x,v1->y-v0->y,v1->x-v0->x);
                else
                    dy = v1->y + FPMultDiv(x-v1->x,v0->y-v1->y,v0->x-v1->x);
                if (dy < y)
                    nhits++;
            }
        }
    }
    return (nhits & 1) == 1;
}


PUBLIC SEC_PSector SEC_FindSector(SEC_PSectorMap sec, SEC_PSector s, uint32 x, uint32 y) {
    int i;
    SEC_PSector ps;

    assert(sec != NULL);

    if (s != NULL && SEC_IsInSector(sec, s, x, y))
        return s;
    ps = sec->secs + 1;
    for (i = 1; i < sec->ns; i++, ps++)
        if (ps != s && SEC_IsInSector(sec, ps, x, y))
            return ps;
    return sec->secs + 0;
}

PRIVATE bool DelTex(NL_PName *n, void *data) {
    DISPOSE((*n)->data);
    DISPOSE(*n);            // Eat the tree from the bottom up.
    return TRUE;
}

PUBLIC void SEC_EndMap(SEC_PSectorMap sec) {
    int i;
    NL_WalkTree(&sec->tnames, &DelTex, NULL);
    sec->tnames = NULL;
    for (i = 0; i < sec->ns; i++)
        DISPOSE(sec->secs[i].v);
    DISPOSE(sec->slist);
    DISPOSE(sec->secs);
    DISPOSE(sec->verts);
}


// ------------------------------------


/* Formula for drawing walls:

    x, z are in world space (already rotated by the camera).

        x = Ox + túTx
        z = Oz + túTz

    Tx,Tz is the vector for which the texture advances one unit.

    Then, with the projection formula px = xúf/z

        x = pxúz/f, or substituting

        Ox + túTx = pxú(Oz + túTz)/f
        fúOx + túfúTx = pxúOz + túpxúTz
        tú(fúTx - pxúTz) = pxúOz - fúOx

            pxúOz - fúOx
        t = ------------
            fúTx - pxúTz

        In reality, we have px=(x>>4)úf/z to correct focus, so take care.

    Ox >> 4 + túTx >> 4 = pxú(Oz + túTz)/f

    fOx>>4 + tfTx>>4 = pxOz + tpxTz

    t(fTx>>4 - pxTz) = pxOz - fOx>>4

    which gives the texture coordinate 't' for each screen x column 'px'.

    Now, Ox and Oz will be fixed to one of the vertices, say v0.

        Ox = v0.x
        Oz = v0.z

    Tx and Tz will be the texture vector unit, ergo

        Tx = Cos(angle)
        Tz = Sin(angle)

    where 'angle' is the wall angle.

    The wall height and y position are calculated in an easier fashion,
    as they are somehow proportional to the screen x:

    px = xúf/z     horizontal
    py = yúfy/z    vertical

    x and z are proportional,

    (x-x0)údz/dx = (z-z0)údx/dz

    px = xúf/(z0 + (x-x0)údz/x), but also

    px = (x0 + (z-z0)údx/dz)úf/z
    px = x0úf + zúdx/dzúf - z0údx/dzúf

    pxúz = x0úf + (z-z0)údx/dzúf

    zú(px - fúdx/dz) = fúx0 - z0úfúdx/dz

        fúx0 + z0úfúdx/dz   fúx0údz - z0úfúdx
    z = ----------------- = -----------------
          px - fúdx/dz        pxúdz - fúdx

                          pxúdz - fúdx
    wy = byúfy/z = bfyú-----------------
                       fú(x0údz - z0údx)

                               dz
    wy(px+1) = wy(px) + -----------------
                        fú(x0údz - z0údx)
                                    pxúdz - fúdx
    If fy == f, we have wy(px) = byú-------------
                                    x0údz - z0údx

    In reality, we have px=(x>>4)úf/z to correct focus, so take care.

*/

// Ok, sometimes I mess up with what means 'y' and 'z', but well...
// In the world, y is depth, but in 3D y usually is height and z is depth.

PUBLIC void SEC_RenderSectorWalls(SEC_PSector s, int cx, int cy, F3D_PCamera cam, bool inv) {
    int i;
    SEC_PSide pv;
    dword a;
    sint32 sa, ca;

        // Project verts
    a = cam->angle;
    sa = Sin(a);
    ca = Cos(a);
    pv = s->v;
    for (i = 0; i < s->nv; i++, pv++) {
//        pv = *ppv;
        if (pv->v0->magic != MagicNumba) {
            sint32 x, y, rx, ry;
            pv->v0->magic = MagicNumba;

#define ZLIM (8 << 12)
            x = (sint32)((uint32)SEC_TOMAP(pv->v0->x) >> 6) - (sint32)((uint32)cam->x >> 6);
            y = (sint32)((uint32)SEC_TOMAP(pv->v0->y) >> 6) - (sint32)((uint32)cam->y >> 6);

            rx = FPMult(y, ca) - FPMult(x, sa);
            ry = FPMult(x, ca) + FPMult(y, sa);
            pv->v0->rx = rx;     // Unclipped
            pv->v0->rz = ry;     // Unclipped
            if (ry < ZLIM)
                ry = ZLIM;
            pv->v0->pz = ry;            // Clipped

            pv->v0->py = cy + FPMultDiv((cam->data.h << 2), cam->data.focus, ry)
                       - cam->data.horizon;

            pv->v0->px = cx + FPMultDiv(rx >> 4, cam->data.focus, ry);
        }
    }

    pv = s->v;
    for (i = 0; i < s->nv; i++, pv++) {
        SEC_PVertex v0, v1;
        if (inv) {
            v0 = pv->v1;
            v1 = pv->v0;
        } else {
            v0 = pv->v0;
            v1 = pv->v1;
        }
//        pv = *ppv;
        if (pv->tex != NULL //&& pv->v0->rx < pv->v1->rx
         && (v0->rz >= ZLIM || v1->rz >= ZLIM)) {
            byte c = ((dword)pv) & 0xC;
/*{
            sint32 h0, h1;
            sint32 px0, px1;

            if (pv->v0->rz < ZLIM) px0 = DRW_MinX;
            else                   px0 = pv->v0->px;
            if (pv->v1->rz < ZLIM) px1 = DRW_MaxX;
            else                   px1 = pv->v1->px;

            h0 = cy + FPMultDiv((cam->data.h << 2) - (0x1800), cam->data.focus, pv->v0->pz)
                       - cam->data.horizon;
            h1 = cy + FPMultDiv((cam->data.h << 2) - (0x1800), cam->data.focus, pv->v1->pz)
                       - cam->data.horizon;
            POLY_ScrapPoly[0].x = px0;
            POLY_ScrapPoly[0].y = pv->v0->py;
            POLY_ScrapPoly[1].x = px0;
            POLY_ScrapPoly[1].y = h0;
            POLY_ScrapPoly[2].x = px1;
            POLY_ScrapPoly[2].y = h1;
            POLY_ScrapPoly[3].x = px1;
            POLY_ScrapPoly[3].y = pv->v1->py;
//            POLY_SolidDraw(POLY_ScrapPoly, 4, c);
            POLY_Line(pv->v0->px, pv->v0->py, pv->v1->px, pv->v1->py, c);
}*/
{
            sint32 px0, px1;
            sint32 ox, oz, tx, tz;
            sint32 dx, dz, den;
            sint32 tw, th;

            dx = (v1->rx - v0->rx);
            dz = (v1->rz - v0->rz);

                // Check for face visibility.
                // Either v0 or v1 must be >= ZLIM
                // dz will not be too small, at least it must be
                // greater that the ZLIM-z factor. No problem there.
            if (v0->rz < ZLIM) {
                sint32 ix;
                if (v1->px <= DRW_MinX)
                    continue;
                ix = v0->rx + FPMultDiv((ZLIM - v0->rz),dx,dz);
                ix = cx + FPMultDiv(ix >> 4, cam->data.focus, ZLIM);
                if (ix >= v1->px)
                    continue;
                v0->px = ix;
            } else if (v1->rz < ZLIM) {
                sint32 ix;
                if (v0->px >= DRW_MaxX)
                    continue;
                ix = v1->rx + FPMultDiv((ZLIM - v1->rz),dx,dz);
                ix = cx + FPMultDiv(ix >> 4, cam->data.focus, ZLIM);
                if (ix <= v0->px)
                    continue;
                v1->px = ix;
            }
            tw = pv->tex->w << (16-4);
            th = pv->tex->h << (13-5);

            ox = v0->rx >> (4+4);
            oz = v0->rz >> 4;
            tx = FPMult(tw, Sin(pv->angle - cam->angle)) >> (4+4);
            tz = FPMult(tw, Cos(pv->angle - cam->angle)) >> 4;

            den = FPnMult(v0->rx, dz, 16)
                - FPnMult(v0->rz, dx, 16);

            px0 = v0->px;
            if (px0 < DRW_MinX || v0->rz < ZLIM)
                px0 = DRW_MinX;
            px1 = v1->px;
            if (px1 > DRW_MaxX || v1->rz < ZLIM)
                px1 = DRW_MaxX;

            DRW_TranslatePtr = GL_ClrTable + 16*256;
            DRW_Tile = FALSE;
            while (px0 < px1) {
                sint32 y0, y1;
                sint32 num;
                sint32 t;

                num = FPnMult((px0-cx),dz,16-4) - FPnMult(cam->data.focus,dx,16);
//                num = (FPnMult(px0,dz,0) - FPnMult(cam->data.focus,dx,0)) >> 16;
//                num = (px0*dz - cam->data.focus*dx) >> 16;
                y1 = cy - cam->data.horizon
                   + FPMultDiv((cam->data.h << 2), num, den);
                if (y1 < cy) {
                    px0++;
                    continue;
                }
                y0 = cy - cam->data.horizon
                   + FPMultDiv((cam->data.h << 2) - th, num, den);
                if (DRW_SetVerticalSpan(px0, y0, y1 - y0, pv->tex->h)) {
//                    if (Detail & 1)
                    if (WallsDetail) {
                        sint32 b1, b2, b3, b4;
                        b1 = (px0-cx)*oz;
                        b2 = cam->data.focus*ox;
                        b3 = cam->data.focus*tx;
                        b4 = (px0-cx)*tz;
                        if (b3 != b4) { // Shouldn't but... :)
                            t = FPMultDiv((b1 - b2), pv->tex->w, (b3 - b4));
                            t = Abs32(t) % pv->tex->w;
                            if (LLS_VMode == LLSVM_640x400x256 || LLS_VMode == LLSVM_640x480x256)
                                DRW_DoVerticalDraw640(LLS_Screen[0]+LLS_SizeX*(y0+DRW_Skip)+px0,
                                           ((byte*)pv->tex) + pv->tex->offsets[t]);
                            else
                                DRW_DoVerticalDraw(LLS_Screen[0]+LLS_SizeX*(y0+DRW_Skip)+px0,
                                           ((byte*)pv->tex) + pv->tex->offsets[t]);
                        }
                    } else {
                        byte *p;
                        y0 += DRW_Skip;
                        p = LLS_Screen[0]+LLS_SizeX*y0 + px0;
                        while (DRW_Height-- > 0) {
                            *p = c;
                            p += LLS_SizeX;
                        }
                    }
                }
                px0++;
            }
}
        }
    }
}

PUBLIC void SEC_RenderSector(SEC_PSector s, int cx, int cy, F3D_PCamera cam) {
    THN_PThing p;
        // Render objects.
    FSP_ClearObjs(cx, cy, cam);
    p = s->list;
    while (p != NULL) {
        if (p->spr != NULL)
            FSP_AddObj(p->x, p->y, p->z, p->angle, 0, p->spr, p->tirerot, p->tiredir);
        p = p->sector.next;
    }
    FSP_DumpObjs(cam, Map.trans, cx, cy);
}

    // Little trick here. First render the sector 0, then all others.
PUBLIC void SEC_Render(SEC_PSectorMap sec, SEC_PSector s, int cx, int cy, F3D_PCamera cam) {
    int base, n;
    int i, j, NSecList;
    SEC_PSector s2;
    int maxsectors;

    MagicNumba++;

    NSecList = 0;
    s2 = SEC_FindSector(sec, s, SEC_TOSEC(cam->x), SEC_TOSEC(cam->y));
    if (s2 != sec->secs) {      // Camera can't be in first sector.
        cam->sec = s2;
        s = s2;
    }
    REQUIRE(NSecList < sec->ns);
    sec->slist[NSecList++] = s;

        // Force first sector to not be processed.
    sec->secs[0].magic = MagicNumba;
    s->magic = MagicNumba;
    base = 0;

    maxsectors = 20 + BackgroundDetail*2 + ExtraDetail*10;
    do {
        if (NSecList >= maxsectors)
            break;
        n = NSecList;
        for (i = base; i < n; i++) {
            SEC_PSector ps;
            ps = sec->slist[i];
            for (j = 0; j < ps->nv; j++) {
                if (ps->v[j].otherside->magic != MagicNumba) {
                    ps->v[j].otherside->magic = MagicNumba;
/*
                    REQUIRE(NSecList < sec->ns);
                    {
                        int k;
                        for (k = 0; k < NSecList; k++)
                            REQUIRE(sec->slist[k] != ps->v[j].otherside);
                    }
*/
                    sec->slist[NSecList++] = ps->v[j].otherside;
                }
            }
        }
        base = n;
    } while (base < NSecList);

        // Render first sector.
    sec->slist[NSecList++] = sec->secs;
    for (i = NSecList-1; i >= 0; i--) {
            // Render sector walls.
        SEC_RenderSectorWalls(sec->slist[i], cx, cy, cam, FALSE);

            // Render sector objects.
        SEC_RenderSector(sec->slist[i], cx, cy, cam);

            // Render sector walls.
        SEC_RenderSectorWalls(sec->slist[i], cx, cy, cam, TRUE);

        if ((i & 7) == 0)
            RACE_HandleComms(FALSE);
    }
}

// ------------------------------ SECTORS.C ----------------------------