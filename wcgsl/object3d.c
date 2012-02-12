// ----------------------------- OBJECT3D.C ---------------------------
// For use with Watcom C.
// (C) Copyright 1995 by Jare & JCAB of Iguana.

// Definitive 3D object routines.

#include <object3d.h>
#include <stdio.h>
#include <strparse.h>
#include <string.h>
#include <ctype.h>
#include <jclib.h>
#include <stdarg.h>
#include <polygon.h>

#include <sincos.h>

//#define DOLOG

PUBLIC int O3DM_MaxDetail = O3DD_TEXGOURAUD;
PUBLIC int O3DM_OrderByZ  = TRUE;

#define MINDL 1600
#define MAXDL 16000

#define ORDER_FACES

// --------------------

static void LOG(const char *fmt, ...) {
#ifdef DOLOG
    va_list l;
    va_start(l, fmt);
    vprintf(fmt, l);
    va_end(l);
#endif
}

#ifdef DOLOG
#define CHECK(a) REQUIRE(a)
#else
#define CHECK(a)
#endif

typedef struct {
    FILE *f;
    char line[300];
    char *token[200];
    int  i, nt, skip;
} TParse, *PParse;

PRIVATE char *GetToken(PParse pa) {
    char *t;
    while (pa->skip >= 0) {
        t = NULL;
        if (pa->i < 0 || pa->i >= pa->nt) {
            while (!feof(pa->f)) {
                pa->line[0] = '\0';
                fgets(pa->line, sizeof(pa->line), pa->f);
                STRP_CleanLine(pa->line, pa->line);
                if (pa->line[0] == '\0' || pa->line[0] == ';' || pa->line[0] == '#')
                    continue;
                t = pa->line;
                break;
            }
            if (t == NULL)      // Couldn't find a line before eof.
                return NULL;
            pa->nt = STRP_SplitLine(pa->token, SIZEARRAY(pa->token), pa->line);
            pa->i = 0;
        }
        pa->skip--;
    }
    pa->skip = 0;
    return pa->token[pa->i++];
}

#define ADD(p,d) ((void*)(((byte *)(p))+(d)))

PRIVATE bool LoadInit(O3DM_PObject *po, O3DF_TObject *o, PParse pa) {
    char *token;
    
    while ( (token = GetToken(pa)) != NULL) {
        if (!strcmpi(token, "Object")) {
            memset(o, 0, sizeof(*o));
            LOG("Found object %s\n", GetToken(pa));
            break;
        }
    }
    return (token != NULL);
}

PRIVATE bool LoadHeader(O3DM_PObject *po, O3DF_TObject *o, PParse pa) {
    char   *token;
    sint32 v;

    while ( (token = GetToken(pa)) != NULL) {
        if (!strcmpi(token, "EndHeader")) {
            int size;
            O3DM_PObject obj;

            LOG("%d verts, %d normals, %d faces, %d faceverts, %d materials",
                   o->nVerts, o->nNormals, o->nFaces, o->nFaceVerts, o->nMaterials);

            size = sizeof(*obj)
                 + sizeof(obj->verts[0])        * o->nVerts
                 + sizeof(obj->normals[0])      * o->nNormals
                 + sizeof(obj->faces->h)        * o->nFaces
                 + sizeof(obj->faces->verts[0]) * o->nFaceVerts
                 + sizeof(obj->materials[0])    * o->nMaterials
                 ;
            REQUIRE( (obj = NEW(size)) != NULL);
            *po = obj;
            memset(obj, 0, size);
            obj->verts     = ADD(obj, sizeof(*obj));
            obj->normals   = ADD(obj->verts,   sizeof(obj->verts[0])       *o->nVerts);
            obj->faces     = ADD(obj->normals, sizeof(obj->normals[0])     *o->nNormals);
            obj->materials = ADD(obj->faces,   sizeof(obj->faces->h)       *o->nFaces
                                             + sizeof(obj->faces->verts[0])*o->nFaceVerts);
            obj->flags = o->flags;
            obj->nFaceVerts = o->nFaceVerts;
            obj->scx = o->scx;
            obj->scy = o->scy;
            obj->scz = o->scz;
            obj->dcx = o->dcx;
            obj->dcy = o->dcy;
            obj->dcz = o->dcz;
            obj->nMaterials = -1;
            LOG("\nReading materials:");
            break;
        } else if (!isdigit(token[0])) {
            char *tv;
            tv = GetToken(pa);
            sscanf(tv, "%li", &v);
            if (!strcmpi(token, "Vertices"))
                o->nVerts = v;
            else if (!strcmpi(token, "Normals"))
                o->nNormals = v;
            else if (!strcmpi(token, "Faces"))
                o->nFaces = v;
            else if (!strcmpi(token, "FaceVertices"))
                o->nFaceVerts = v;
            else if (!strcmpi(token, "Materials"))
                o->nMaterials = v;
            else if (!strcmpi(token, "Flags"))
                o->flags = v;
            else if (!strcmpi(token, "ScaleX"))
                o->scx = v;
            else if (!strcmpi(token, "ScaleY"))
                o->scy = v;
            else if (!strcmpi(token, "ScaleZ"))
                o->scz = v;
            else if (!strcmpi(token, "OffsetX"))
                o->dcx = v;
            else if (!strcmpi(token, "OffsetY"))
                o->dcy = v;
            else if (!strcmpi(token, "OffsetZ"))
                o->dcz = v;
            token = tv;
        }
    }
    return (token != NULL);
}

PRIVATE bool LoadMaterials(O3DM_PObject *po, O3DF_TObject *o, PParse pa) {
    char *token;
    O3DM_PObject obj = *po;
    sint32 v;

    while ( (token = GetToken(pa)) != NULL) {
        if (!strcmpi(token, "Material")) {
            int nv = -1;
            obj->nMaterials++;
            token = GetToken(pa);
            sscanf(token, "%i", &nv);
            CHECK(nv == obj->nMaterials && obj->nMaterials < o->nMaterials);
            LOG(" %s", token);
        } else if (!strcmpi(token, "EndMaterials")) {
            obj->nMaterials++;
            REQUIRE(obj->nMaterials == o->nMaterials);
            obj->nVerts = -1;
            LOG("\nReading vertices:");
            break;
        } else if (!isdigit(token[0])) {
            char *tv;
            REQUIRE(obj->nMaterials < o->nMaterials);
            tv = GetToken(pa);
            sscanf(tv, "%li", &v);
            if (!strcmpi(token, "Color"))
                obj->materials[obj->nMaterials].color = v;
            else if (!strcmpi(token, "Flags"))
                obj->materials[obj->nMaterials].flags = v;
            else if (!strcmpi(token, "Ambient"))
                obj->materials[obj->nMaterials].ambient = v;
            else if (!strcmpi(token, "Diffuse"))
                obj->materials[obj->nMaterials].diffuse = v;
            else if (!strcmpi(token, "Reflected"))
                obj->materials[obj->nMaterials].reflected = v;
            else if (!strcmpi(token, "Texture")) {
                strncpy(obj->materials[obj->nMaterials].texname, tv, 8);
                if (tv[0] != '\0') {
                    char buf[100];
                    long l;
                    sprintf(buf, "%.8s.tex", tv);
                    l = JCLIB_FileSize(buf);
                    if (l > 0)
                        obj->materials[obj->nMaterials].texture = NEW(l);
                    if (obj->materials[obj->nMaterials].texture != NULL) {
                        long pos;
                        pos = ftell(pa->f);
                        if (JCLIB_Load(buf, obj->materials[obj->nMaterials].texture, l) != l) {
                            LOG("Error reading texture <<%s>>", buf);
                            DISPOSE(obj->materials[obj->nMaterials].texture);
                        }
                        fseek(pa->f, pos, SEEK_SET);
                    }
                } else
                    obj->materials[obj->nMaterials].texture = NULL;
            }
            token = tv;
        }
    }
    return (token != NULL);
}

PRIVATE bool LoadVertices(O3DM_PObject *po, O3DF_TObject *o, PParse pa) {
    char *token;
    O3DM_PObject obj = *po;
    sint32 v;

    while ( (token = GetToken(pa)) != NULL) {
        if (!strcmpi(token, "Vertex")) {
            int nv = -1;
            obj->nVerts++;
            token = GetToken(pa);
            sscanf(token, "%i", &nv);
            CHECK(nv == obj->nVerts && obj->nVerts < o->nVerts);
            LOG(" %s", token);
        } else if (!strcmpi(token, "EndVertices")) {
            obj->nVerts++;
            REQUIRE(obj->nVerts == o->nVerts);
            obj->nNormals = -1;
            LOG("\nReading normals:");
            break;
        } else if (!isdigit(token[0])) {
            char *tv;
            REQUIRE(obj->nVerts < o->nVerts);
            tv = GetToken(pa);
            sscanf(tv, "%li", &v);
            if (!strcmpi(token, "X"))
                obj->verts[obj->nVerts].x = v;
            else if (!strcmpi(token, "Y"))
                obj->verts[obj->nVerts].y = v;
            else if (!strcmpi(token, "Z"))
                obj->verts[obj->nVerts].z = v;
            token = tv;
        }
    }
    return (token != NULL);
}

PRIVATE bool LoadNormals(O3DM_PObject *po, O3DF_TObject *o, PParse pa) {
    char *token;
    O3DM_PObject obj = *po;
    sint32 v;

    while ( (token = GetToken(pa)) != NULL) {
        if (!strcmpi(token, "Normal")) {
            int nv = -1;
            obj->nNormals++;
            token = GetToken(pa);
            sscanf(token, "%i", &nv);
            CHECK(nv == obj->nNormals && obj->nNormals < o->nNormals);
            LOG(" %s", token);
        } else if (!strcmpi(token, "EndNormals")) {
            obj->nNormals++;
            REQUIRE(obj->nNormals == o->nNormals);
            LOG("\nReading faces:");
            obj->nFaces = -1;
            break;
        } else if (!isdigit(token[0])) {
            char *tv;
            REQUIRE(obj->nNormals < o->nNormals);
            tv = GetToken(pa);
            sscanf(tv, "%li", &v);
            if (!strcmpi(token, "X"))
                obj->normals[obj->nNormals].x = v;
            else if (!strcmpi(token, "Y"))
                obj->normals[obj->nNormals].y = v;
            else if (!strcmpi(token, "Z"))
                obj->normals[obj->nNormals].z = v;
            token = tv;
        }
    }
    return (token != NULL);
}

PRIVATE bool LoadFaces(O3DM_PObject *po, O3DF_TObject *o, PParse pa) {
    char *token;
    O3DM_PObject obj = *po;
    sint32 v;
    O3DM_PFace face = NULL;
    int    nfv = -1;

    while ( (token = GetToken(pa)) != NULL) {
        if (!strcmpi(token, "Face")) {
            int nv = -1;
            if (face == NULL) {
                face = obj->faces;
                face->h.back = NULL;
            } else {
                nfv++;
                REQUIRE(nfv == face->h.nVerts);
                face->h.front->h.back = face;
                face = face->h.front;
                nfv = -1;
            }
            REQUIRE(face != NULL);
            assert((byte*)face < (byte*)obj->materials);
            face->h.front = NULL;
            obj->nFaces++;
            token = GetToken(pa);
            sscanf(token, "%i", &nv);
            CHECK(nv == obj->nFaces && obj->nFaces < o->nFaces);
            LOG(") %s (", token);
        } else if (!strcmpi(token, "EndFaces")) {
            face->h.front = NULL;   // No next face.
            obj->nFaces++;
            REQUIRE(obj->nFaces == o->nFaces);
            LOG("\nDone!\n");
            break;
        } else if (!isdigit(token[0])) {
            char *tv;
            REQUIRE(obj->nFaces < o->nFaces);
            tv = GetToken(pa);
            sscanf(tv, "%li", &v);
            if (!strcmpi(token, "Vertices")) {
                face->h.nVerts = v;
                face->h.front = ADD(face,sizeof(face->h) + sizeof(face->verts[0])*face->h.nVerts);
            } else if (!strcmpi(token, "Material")) {
                if (v < 0)
                    face->h.material = NULL;
                else {
                    REQUIRE(v < obj->nMaterials);
                    face->h.material = obj->materials + v;
                }
            } else if (!strcmpi(token, "TOX"))
                face->h.tox = v;
            else if (!strcmpi(token, "TOY"))
                face->h.toy = v;
            else if (!strcmpi(token, "TSX"))
                face->h.tsx = v;
            else if (!strcmpi(token, "TSY"))
                face->h.tsy = v;
            else if (!strcmpi(token, "TA"))
                face->h.ta = v;
            else if (!strcmpi(token, "Flags"))
                face->h.flags = v;
            else if (!strcmpi(token, "Vertex")) {
                nfv++;
                face->verts[nfv].vert = obj->verts + v;
                LOG("%s,", tv);
            } else if (!strcmpi(token, "Normal"))
                face->verts[nfv].normal = obj->normals + v;
            else if (!strcmpi(token, "TextureX"))
                face->verts[nfv].tx = v;
            else if (!strcmpi(token, "TextureY"))
                face->verts[nfv].ty = v;
            token = tv;
        }
    }
    return (token != NULL);
}

PUBLIC O3DM_PObject O3DM_LoadObject(const char *fname) {
    TParse a;
    O3DM_PObject obj = NULL;
    O3DF_TObject o;

        // Check if a binary object.
    {
        FILE *f;
        char sign[4];
        memset(sign, 0, sizeof(sign));
        f = JCLIB_Open(fname);
        if (f == NULL)
            return NULL;
        fread(sign, 4, 1, f);
        if (memcmp(sign, "B3D", 4) == 0) {
#define OF(p) ((void*)(((dword)(p))+base))
            O3DM_PFace face, pf2;
            dword   base;
            int     i;
            dword   size;

            size = 0;
            fread(&size, sizeof(size), 1, f);
            if ( (obj = NEW(size)) != NULL) {
                fread(obj, size, 1, f);
                base = (dword)obj;
                obj->verts     = OF(obj->verts);
                obj->normals   = OF(obj->normals);
                obj->faces     = OF(obj->faces);
                obj->materials = OF(obj->materials);
                pf2 = NULL;
                for (face = obj->faces; face != NULL; face = pf2) {
                    if (face->h.front != NULL)
                        face->h.front = OF(face->h.front);
                    pf2 = face->h.front;
                    for (i = 0; i < face->h.nVerts; i++) {
                        face->verts[i].vert = OF(face->verts[i].vert);
                        if (face->verts[i].normal != NULL)
                            face->verts[i].normal = OF(face->verts[i].normal);
                    }
                    if (face->h.material != NULL)
                        face->h.material = OF(face->h.material);
                }
                for (i = 0; i < obj->nMaterials; i++)
                    if (obj->materials[i].texture != NULL) {
                        long l;
                        if (obj->materials[i].flags & O3DMF_256)
                            l = 256*64;
                        else
                            l = 64*64;
                        obj->materials[i].texture = NEW(l);
                        if (obj->materials[i].texture != NULL)
                            fread(obj->materials[i].texture, l, 1, f);
                        else
                            fseek(f, 1, SEEK_CUR);
                    }
            }
            JCLIB_Close(f);
            return obj;
        }
        JCLIB_Close(f);
    }

//    if ( (a.f = fopen(fname, "rt")) == NULL)
    if ( (a.f = JCLIB_OpenText(fname)) == NULL)
        return NULL;

    obj = NULL;
    a.i = -1;
    a.skip = 0;

    if (!(   LoadInit(&obj, &o, &a)
          && LoadHeader(&obj, &o, &a)
          && LoadMaterials(&obj, &o, &a)
          && LoadVertices(&obj, &o, &a)
          && LoadNormals(&obj, &o, &a)
          && LoadFaces(&obj, &o, &a)
         )
        )
        DISPOSE(obj);
    JCLIB_Close(a.f);
    return obj;
}

// -----------------------------------------------------------

PRIVATE void SaveHeader   (FILE *f, O3DM_PObject obj) {
    fprintf(f,
        "Vertices %d Normals %d Faces %d FaceVertices %d Materials %d Flags 0x%04X\n"
        "ScaleX 0x%X ScaleY 0x%X ScaleZ 0x%X\n"
        "OffsetX %10d OffsetY %10d OffsetZ %10d\n"
        "EndHeader\n"
        , (dword)obj->nVerts, (dword)obj->nNormals, (dword)obj->nFaces
        , (dword)obj->nFaceVerts, (dword)obj->nMaterials, (dword)obj->flags
        , (dword)obj->scx, (dword)obj->scy, (dword)obj->scz
        , (dword)obj->dcx, (dword)obj->dcy, (dword)obj->dcz
    );
}

PRIVATE void SaveMaterials(FILE *f, O3DM_PObject obj) {
    int i;
    O3DM_PMaterial m;
    for (i = 0; i < obj->nMaterials; i++) {
        m = obj->materials + i;
        fprintf(f,
            "Material %d Color %d Flags 0x%04X Ambient %d Diffuse %d Reflected %d Texture \"%.8s\"\n"
            , (dword)i, (dword)m->color, (dword)m->flags, (dword)m->ambient,
            (dword)m->diffuse, (dword)m->reflected, m->texname);
    }
    fprintf(f, "EndMaterials\n");
}

PRIVATE void SaveVertices (FILE *f, O3DM_PObject obj) {
    int i;
    O3DM_PVertex v;
    for (i = 0; i < obj->nVerts; i++) {
        v = obj->verts + i;
        fprintf(f,
            "Vertex %5d X %10d Y %10d Z %10d\n"
            , (dword)i, (dword)v->x, (dword)v->y, (dword)v->z);
    }
    fprintf(f, "EndVertices\n");
}

PRIVATE void SaveNormals  (FILE *f, O3DM_PObject obj) {
    int i;
    O3DM_PNormal v;
    for (i = 0; i < obj->nNormals; i++) {
        v = obj->normals + i;
        fprintf(f,
            "Normal %5d X %10d Y %10d Z %10d\n"
            , (dword)i, (dword)v->x, (dword)v->y, (dword)v->z);
    }
    fprintf(f, "EndNormals\n");
}

PRIVATE void SaveFaces    (FILE *f, O3DM_PObject obj) {
    int i, j;
    O3DM_PFace p;

    for (i = 0, p = obj->faces; p != NULL; i++, p = p->h.front) {
        fprintf(f,
            "Face %d Vertices %d Material %d Flags 0x%04X TOX %d TOY %d TSX %d TSY %d TA 0x%04X\n"
            , (dword)i, (dword)p->h.nVerts,
            (dword)((p->h.material==NULL)?-1:(p->h.material - obj->materials)),
            (dword)p->h.flags,
            p->h.tox, p->h.toy, p->h.tsx, p->h.tsy, p->h.ta
            );
        for (j = 0; j < p->h.nVerts; j++) {
            fprintf(f,
                "    Vertex %5d Normal %5d TextureX 0x%08X TextureY 0x%08X\n"
                , (dword)(p->verts[j].vert - obj->verts),
                (dword)(p->verts[j].normal - obj->normals),
                (dword)p->verts[j].tx, (dword)p->verts[j].ty);
        }
    }
    fprintf(f, "EndFaces\n");
}

PUBLIC bool O3DM_SaveObject(const char *fname, O3DM_PObject obj) {
    FILE *f;

    if ( (f = fopen(fname, "wt")) == NULL)
        return FALSE;
    fprintf(f, "; File saved from a previously loaded object.\n"
              "Object \"%s\"\n", fname);
    SaveHeader   (f, obj);
    SaveMaterials(f, obj);
    SaveVertices (f, obj);
    SaveNormals  (f, obj);
    SaveFaces    (f, obj);
    fclose(f);
    return TRUE;
}

// -----------------------------------------------------------

    // Delete all memory taken by an object.
PUBLIC void O3DM_DeleteObject(O3DM_PObject obj) {
    int i;
    if (obj == NULL)
        return;
    for (i = 0; i < obj->nMaterials; i++)
        DISPOSE(obj->materials[i].texture);
    DISPOSE(obj);
}

PUBLIC O3DM_PObject O3DM_DupObj(O3DM_PObject obj) {
    O3DM_PObject o;
    O3DM_PFace p, *g, prev;
    int i;

    REQUIRE( (o = NEW(sizeof(*o))) != NULL);
    memset(o, 0, sizeof(*o));
    o->nVerts = obj->nVerts;
    o->nNormals = obj->nNormals;
    o->nFaces = obj->nFaces;
    o->nFaceVerts = obj->nFaceVerts;
    o->nMaterials = obj->nMaterials;
    o->flags = obj->flags;
    o->scx = obj->scx;
    o->scy = obj->scy;
    o->scz = obj->scz;
    o->dcx = obj->dcx;
    o->dcy = obj->dcy;
    o->dcz = obj->dcz;

    o->verts     = NEW(o->nVerts*sizeof(o->verts[0]));
    o->normals   = NEW(o->nNormals*sizeof(o->normals[0]));
    o->materials = NEW(o->nMaterials*sizeof(o->materials[0]));

    memcpy(o->verts, obj->verts, o->nVerts*sizeof(o->verts[0]));
    memcpy(o->normals, obj->normals, o->nNormals*sizeof(o->normals[0]));
    memcpy(o->materials, obj->materials, o->nMaterials*sizeof(o->materials[0]));

    g = &o->faces;
    prev = NULL;
    for (p = obj->faces; p != NULL; p = p->h.front) {
        *g = NEW(sizeof(p->h) + p->h.nVerts*sizeof(p->verts[0]));
        REQUIRE(*g != NULL);
        memcpy(*g, p, sizeof(p->h) + p->h.nVerts*sizeof(p->verts[0]));
        (*g)->h.material = o->materials + (p->h.material - obj->materials);
        (*g)->h.back = prev;
        prev = *g;
        for (i = 0; i < p->h.nVerts; i++) {
            (*g)->verts[i].vert   = o->verts   + (p->verts[i].vert - obj->verts);
            (*g)->verts[i].normal = o->normals + (p->verts[i].normal - obj->normals);
        }
        g = &(*g)->h.front;
    }
    return o;
}

// -----------------------------------------------------------

#define PLANE 300

PUBLIC void O3DM_ParseVisibility(O3DM_PObject obj) {
    O3DM_PFace face;

    for (face = obj->faces; face != NULL; face = face->h.front) {
        if (face->h.nVerts < 3 || (face->h.flags & O3DFF_VISIBLE))
            face->h.visible = TRUE;
        else {
            O3DM_PVertex v0, v1, v2;
            O3DM_PFaceVertex p;
            int i;

            p = face->verts;
            for (i = 0; i < face->h.nVerts; i++, p++)
                if (p->vert->rz > PLANE)
                    break;
            if (i >= face->h.nVerts) {
                face->h.visible = FALSE;
                continue;
            }

            p = face->verts;
            v0 = p->vert; p++;
            v1 = p->vert; p++;
            v2 = p->vert;
            face->h.visible =
                (( v2->px - v1->px)*( v0->py - v1->py)
                -( v2->py - v1->py)*( v0->px - v1->px))
                 < 0;
//            face->h.visible = TRUE;
        }
    }
}

// -----------------------------------------------------------

PUBLIC O3DM_PFace O3DM_OrderFaces(O3DM_PObject obj) {
    static O3DM_PFace dlist[1024];
    static O3DM_PFace last[1024];
    static O3DM_PFace first;
    O3DM_PFace face;
    int j, maxfi;
    sint32 z;

    first = NULL;
    if (O3DM_OrderByZ) {
        O3DM_PFace f, g, *p;

        RepStosd(dlist, 0, 1024);
        RepStosd(last, 0, 1024);
        maxfi = 0;
        for (face = obj->faces; face != NULL; face = face->h.front) {
            int fi;
            O3DM_PFaceVertex fv;

            f = face;
            if ( (f->h.material == NULL) || !f->h.visible)
                continue;

                // calc face Z

            if (f->h.flags & O3DFF_NORDER) {
                z = 0x7FFFFFFF;
            } else {
                fv = f->verts;
                z = fv->vert->rz + Abs32(fv->vert->rx)/4 + Abs32(fv->vert->ry)/4;
                fv++;
                for (j = 1; j < f->h.nVerts; j++, fv++)
                    z += fv->vert->rz + Abs32(fv->vert->rx)/4 + Abs32(fv->vert->ry)/4;
                z /= f->h.nVerts;
            }
            f->h.depth = z;

                // Insertion in Z-ordered list.
            fi = (z >> (31-16-7));
            if (fi >= 1023)
//continue;
                fi = 1023;
            else if (fi < 0)
//continue;
                fi = 0;
            else if (maxfi < fi)
                maxfi = fi;
            p = &dlist[fi];
            if (fi != 1023) {   // Far away -> no ordering.
                while (*p != NULL) {
                    if ((*p)->h.depth <= z)
                        break;
                    p = &((*p)->h.next);
                }
            }
            if (*p == NULL)
                last[fi] = f;
            g = *p;
            *p = f;
            (*p)->h.next = g;
        }
            // Join all the chunks.
        p = &first;
            // Do 1023 always, then all from the highest.
        if (dlist[1023] != NULL) {
            *p = dlist[1023];
            p = &last[1023]->h.next;
        }
        for (j = maxfi; j >= 0; j--) {
            if (dlist[j] != NULL) {
                *p = dlist[j];
                p = &last[j]->h.next;
            }
        }
    } else {
        for (face = obj->faces; face != NULL; face = face->h.front) {
            O3DM_PFace f;

            f = face;
            if ( (f->h.material == NULL) || !f->h.visible)
                continue;
            f->h.next = first;
            first = f;
        }
    }
    return first;
}

/*
void O3DM_Draw(O3DM_PObject obj) {
    int i, j, k, n;
    O3DM_PFace pl;
    for (pl = O3DM_OrderFaces(obj); pl != NULL; pl = pl->h.next) {
        sint32 dl;
        O3DM_PFaceVertex p;

        k = 0;
        dl = 0;
        n = 0;
        for (j = pl->h.nVerts-1; j >= 0 ; j--) {
//        for (j = 0; j < pl->h.nVerts; j++) {
            sint32 l;

            l = pl->verts[j].vert->rz;
            if (l >= 5)
                n++;
            if (l < MINDL)
//                l += 0x1F0000;
                l += 0xF0000;
            else if (l > MAXDL)
                l += 0x000000;
            else
//                l += FPMultDiv(0x1F0000,(MAXDL-l),(MAXDL-MINDL));
                l += FPMultDiv(0x0F0000,(MAXDL-l),(MAXDL-MINDL));
            dl += l;

            POLY_ScrapPoly[k].x  = pl->verts[j].vert->px;
            POLY_ScrapPoly[k].y  = pl->verts[j].vert->py;
            POLY_ScrapPoly[k].tx = pl->verts[j].tx;
            POLY_ScrapPoly[k].ty = pl->verts[j].ty;
//            POLY_ScrapPoly[k].l  = pl->verts[j].l;
//            POLY_ScrapPoly[k].l  = l;
            POLY_ScrapPoly[k].l  = ((pl-obj->faces + k*3) % 16) << 16;

            k++;
        }
        if (n <= 1)
            continue;
        if (pl->h.material->texture == NULL || O3DM_MaxDetail < O3DD_TEXTURED) {
            if ((pl->h.material->flags & O3DMF_NOSHADE)
             || (pl->h.flags & O3DFF_FLAT)
             || O3DM_MaxDetail < O3DD_GOURAUD) {
                if (pl->h.material->flags & O3DMF_NOSHADE)
//                    POLY_ScrapPoly[0].l = 0x1F0000;
                    POLY_ScrapPoly[0].l = 0xF0000;
                else
;//                    POLY_ScrapPoly[0].l = dl/pl->h.nVerts;
                POLY_SolidDraw(POLY_ScrapPoly, pl->h.nVerts, pl->h.material->color);
            } else if (pl->h.material->flags & O3DMF_TRANS)
                POLY_TransDraw(POLY_ScrapPoly, pl->h.nVerts);
            else
                POLY_ShadeDraw(POLY_ScrapPoly, pl->h.nVerts, pl->h.material->color);
        } else {
            if ((pl->h.material->flags & O3DMF_NOSHADE)
             || O3DM_MaxDetail < O3DD_TEXLIGHT)
                POLY_TextureDraw(POLY_ScrapPoly, pl->h.nVerts, pl->h.material->texture);
            else if ((pl->h.flags & O3DFF_FLAT)
                   || O3DM_MaxDetail < O3DD_TEXGOURAUD) {
//                POLY_ScrapPoly[0].l = dl/pl->h.nVerts;
                POLY_LightTexDraw(POLY_ScrapPoly, pl->h.nVerts, pl->h.material->texture);
            } else
                POLY_ShadeTexDraw(POLY_ScrapPoly, pl->h.nVerts, pl->h.material->texture);
        }
    }
}
*/

#define LIGHT_LEVELS 31

PUBLIC void O3DM_CalcLights(O3DM_PObject obj, dword ang) {
    int i;
    dword l;
    O3DM_PVertex pv;
    assert(obj != NULL);

    l = 0;
    pv = obj->verts;
    for (i = 0; i < obj->nVerts; i++, pv++, l += (5 << 10))
        pv->l = (16 << 16) + FPMult(15 << 16, Sin(ang + l));
}

PUBLIC void O3DM_DrawFace(O3DM_PFace pl) {
    int i, j, nv;
    sint32 dl;
    O3DM_PFaceVertex p;
    POLY_PFullVertex pv;
    O3DM_PFaceVertex lastv = NULL;

    pv = POLY_ScrapPoly;                nv = 0;
    p = pl->verts + pl->h.nVerts - 1;   j = pl->h.nVerts-1;
    lastv = pl->verts;      // For wraparound correctly.
    while (j >= 0) {
        if (p->vert->rz < PLANE) {
            if (lastv->vert->rz > PLANE) {  // if == PLANE, new vertex would overlap lastv.
                // We're behind, coming from being in front.
                // So cut the edge.
                sint32 d1, d2, x, y, l, tx, ty;
                d1 = lastv->vert->rz - p->vert->rz;
                d2 = PLANE - p->vert->rz;
                x  = p->vert->rx + FPMultDiv(lastv->vert->rx - p->vert->rx, d2, d1);
                y  = p->vert->ry + FPMultDiv(lastv->vert->ry - p->vert->ry, d2, d1);
                pv->rx = x;
                pv->ry = y;
                pv->rz = PLANE;
                tx = p->tx + FPMultDiv(lastv->tx - p->tx, d2, d1);
                ty = p->ty + FPMultDiv(lastv->ty - p->ty, d2, d1);
                x =  FPMultDiv(x,R3D_FocusX,PLANE) + R3D_CenterX;
                y = -FPMultDiv(y,R3D_FocusY,PLANE) + R3D_CenterY;
                pv->x  = x;
                pv->y  = y;
                pv->tx = tx;
                pv->ty = ty;
                pv->l  = p->vert->l;
                nv++; pv++;
                lastv = p; p--; j--;
            }
            while (j >= 0 && p->vert->rz < PLANE) {
                lastv = p; p--; j--;
            }
            if (j < 0)
                p = pl->verts + pl->h.nVerts - 1;

            if (p->vert->rz > PLANE) {   // if == PLANE, new vertex would overlap lastv.
                // We're in front, coming from behind
                // So cut the edge.
                sint32 d1, d2, x, y, l, tx, ty;

                assert(lastv->vert->rz < PLANE);
                d1 = p->vert->rz - lastv->vert->rz;
                d2 = PLANE - lastv->vert->rz;
                x  = lastv->vert->rx + FPMultDiv(p->vert->rx - lastv->vert->rx, d2, d1);
                y  = lastv->vert->ry + FPMultDiv(p->vert->ry - lastv->vert->ry, d2, d1);
                pv->rx = x;
                pv->ry = y;
                pv->rz = PLANE;
                tx = lastv->tx + FPMultDiv(p->tx - lastv->tx, d2, d1);
                ty = lastv->ty + FPMultDiv(p->ty - lastv->ty, d2, d1);
                x =  FPMultDiv(x,R3D_FocusX,PLANE) + R3D_CenterX;
                y = -FPMultDiv(y,R3D_FocusY,PLANE) + R3D_CenterY;
                pv->x  = x;
                pv->y  = y;
                pv->tx = tx;
                pv->ty = ty;
                pv->l  = p->vert->l;
                nv++; pv++;
            }

            if (j < 0)
                break;
        }
        pv->rx = p->vert->rx;
        pv->ry = p->vert->ry;
        pv->rz = p->vert->rz;
        pv->x  = p->vert->px;
        pv->y  = p->vert->py;
        pv->tx = p->tx;
        pv->ty = p->ty;
        pv->l  = p->vert->l;
        nv++; pv++;

        lastv = p; p--; j--;
    }
    if (nv < 3)
        return;
    if (pl->h.material->flags & O3DMF_TRANS) {
        POLY_ScrapPoly[0].l = (pl->h.material->ambient & LIGHT_LEVELS) << 16;
        POLY_TransDraw(POLY_ScrapPoly, nv);
    } else if (pl->h.material->texture == NULL || O3DM_MaxDetail < O3DD_TEXTURED) {
        if ((pl->h.material->flags & O3DMF_NOSHADE)
         || (pl->h.flags & O3DFF_FLAT)
         || O3DM_MaxDetail < O3DD_GOURAUD) {
            if (pl->h.material->flags & O3DMF_NOSHADE)
                POLY_ScrapPoly[0].l = LIGHT_LEVELS << 16;
            POLY_SolidDraw(POLY_ScrapPoly, nv, pl->h.material->color);
        } else
//            POLY_GouraudDraw(POLY_ScrapPoly, nv, pl->h.material->color);
            POLY_ShadeDraw(POLY_ScrapPoly, nv, pl->h.material->color);
    } else {
        if ((pl->h.flags & O3DFF_FLAT)
         || (pl->h.material->flags & O3DMF_NOSHADE)
         || O3DM_MaxDetail < O3DD_TEXLIGHT) {
            if (pl->h.material->flags & O3DMF_HOLES)
                POLY_HoleTexDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
            else
                POLY_TextureDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
        } else if (O3DM_MaxDetail < O3DD_TEXGOURAUD) {
            POLY_LightTexDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
        } else
            POLY_ShadeTexDraw(POLY_ScrapPoly, nv, pl->h.material->texture);
    }
}

void O3DM_Draw(O3DM_PObject obj) {
    O3DM_PFace pl;

    for (pl = O3DM_OrderFaces(obj); pl != NULL; pl = pl->h.next) {
         O3DM_DrawFace(pl);
    }
}

// ----------------------------- OBJECT3D.C ---------------------------
