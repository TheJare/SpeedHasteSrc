// ---------------------------- PIX.C -----------------------
// Written by Javier Ar‚valo Baeza.
// Screen loading functions.

#include "pix.h"
#include <jclib.h>
#include <string.h>
#include <ctype.h>

// =======================================================
// PCX

PUBLIC int PIX_LoadPCX(const char* file, unsigned char *outpix, unsigned char * pal, int* width, int* height)
{
   int            i, w, h;
   unsigned char  buf[1028];
   long pos;

   fp = JCLIB_Open(file);
   if( !fp ) {
      return( 0 );
   }
   pos = ftell(fp);
   fread( buf, 1, 4, fp );
   if( memcmp( (char *)buf, "\x0a\x05\x01\x08", 4 ) )
   {
      JCLIB_Close( fp );
      return( 0 );
   }
   fread( buf, 1, 12, fp );

   w = buf[4] + 256*buf[5] + 1 - (buf[0] + 256*buf[1]);
   if( width )
      *width = w;
   h = buf[6] + 256*buf[7] + 1 - (buf[2] + 256*buf[3]);
   if( height )
      *height = h;
   if( pal ) {
       fseek(fp, pos+JCLIB_FileSize(file)-768, SEEK_SET);
       for( i = 0; i < 768; i++)
          pal[i] = (unsigned char)getc( fp ) >> 2;
   }
   if(outpix == NULL) {
      JCLIB_Close(fp);
      return( 1 );
   }
   fseek(fp, pos+128, SEEK_SET);

   while (h-- > 0) {
      unsigned char c;
      i = 0;
      do {
        c = (byte)getc(fp);
        if ((c & 0xC0) != 0xC0) {
            OUTB(c);
            i++;
        } else {
            unsigned char v;
            v = (byte)getc(fp);
            c &= ~0xC0;
            while (c > 0 && i < w) {
                OUTB(v);
                i++;
                c--;
            }
        }
    } while (i < w);
   }
   JCLIB_Close( fp );
   return( 1 );
}

// =======================================================
// LBM

typedef unsigned char  LBMUBYTE;
typedef short          LBMWORD;
typedef unsigned short LBMUWORD;
typedef long           LBMLONG;
typedef char           LBMID[4];
typedef struct {
    LBMID      id;
    LBMLONG    size;
    LBMUBYTE   data[];
} LBMCHUNK;

    // A BitMapHeader is stored in a BMHD chunk.
typedef struct {
    LBMUWORD w, h;         /* raster width & height in pixels */
    LBMUWORD  x, y;         /* position for this image */
    LBMUBYTE nPlanes;      /* # source bitplanes */
    LBMUBYTE masking;    /* masking technique */
    LBMUBYTE compression;  /* compression algoithm */
    LBMUBYTE pad1;         /* UNUSED.  For consistency, put 0 here.*/
    LBMUWORD transparentColor;   /* transparent "color number" */
    LBMUBYTE xAspect, yAspect;   /* aspect ratio, a rational number x/y */
    LBMUWORD pageWidth, pageHeight;  /* source "page" size in pixels */
} LBMBMHD;

    // RowBytes computes the number of bytes in a row, from the width in pixels.
#define RowBytes(w)   (((w) + 15) >> 4 << 1)

#define LBMIDEQ(i1,i2) (*(long*)(i1) == *(long*)(i2))



PRIVATE LBMLONG EndianSwapL(LBMLONG l) {
    LBMLONG t = ((LBMLONG)((LBMUBYTE *)&l)[0] << 24) +
                ((LBMLONG)((LBMUBYTE *)&l)[1] << 16) +
                ((LBMLONG)((LBMUBYTE *)&l)[2] <<  8) +
                ((LBMLONG)((LBMUBYTE *)&l)[3] <<  0);
    return t;
}

PRIVATE LBMUWORD EndianSwapW(LBMUWORD l) {
    LBMUWORD t = ((LBMUWORD)((LBMUBYTE *)&l)[0] << 8) +
                 ((LBMUWORD)((LBMUBYTE *)&l)[1] << 0);
    return t;
}

PRIVATE bool ReadChunk(LBMCHUNK *chunk, FILE *f) {
    if (fread(chunk, sizeof(*chunk), 1, f) != 1)
        return FALSE;
    chunk->size = EndianSwapL(chunk->size);
    if (chunk->size & 1)
        chunk->size++;
    return TRUE;
}

PUBLIC int PIX_LoadLBM(const char* file, unsigned char *outpix, unsigned char * pal, int* width, int* height)
{
    FILE *fp;
    long pos;
    LBMCHUNK c;
    LBMID    fid;
    LBMLONG    total;
    bool compression;
    int     w = 0, h = 0;

    fp = JCLIB_Open(file);
    if (fp == NULL)
        return 0;
    pos = ftell(fp);
    if (!ReadChunk(&c, fp) || !LBMIDEQ(c.id, "FORM")
     || c.size < (4+8+sizeof(LBMBMHD)+8+768))
        goto error;
    if (fread(&fid, sizeof(fid), 1, fp) != 1 || !LBMIDEQ(fid, "PBM "))
        goto error;
    total = c.size - sizeof(fid);
    while (total > 0) {
        if (!ReadChunk(&c, fp))
            goto error;
        total -= sizeof(c) + c.size;
        if (LBMIDEQ(c.id, "BMHD")) {
            LBMBMHD hd;
            if (c.size != sizeof(LBMBMHD) || fread(&hd, sizeof(hd), 1, fp) != 1
             || hd.nPlanes != 8)
                goto error;
            w = EndianSwapW(hd.w);
            h = EndianSwapW(hd.h);
            if (width != NULL)  *width  = w;
            if (height != NULL) *height = h;
            compression = hd.compression;
//            if (pal == NULL && outpix == NULL)
//                break;
        } else if (LBMIDEQ(c.id, "CMAP")) {
            int i;
            if (c.size > 768)
                goto error;
            if (pal == NULL) {
                fseek(fp, c.size, SEEK_CUR);
                continue;
            }
            if (fread(pal, c.size, 1, fp) != 1)
                goto error;
            for (i = 0; i < c.size; i++)
                pal[i] >>= 2;
        } else if (LBMIDEQ(c.id, "BODY")) {
            int i;
            if (outpix == NULL) {
                fseek(fp, c.size, SEEK_CUR);
                continue;
            }
            for (i = 0; i < h; i++) {
                if (!compression) {
                    if (fread(outpix, w, 1, fp) != 1)
                        goto error;
                    if (w & 1)
                        fgetc(fp);
                    outpix += w;
                } else {
                    int d, v;
                    d = w;
                    if (d & 1) d++;
                    while (d > 0) {
                        v = (signed char)fgetc(fp);
                        if (v > 0) {
                            v++;
                            d -= v;
                            if (fread(outpix, v, 1, fp) != 1)
                                goto error;
                            outpix += v;
                        } else {
                            int c;
                            v = -v + 1;
                            d -= v;
                            c = fgetc(fp);
                            if (c == EOF)
                                goto error;
                            memset(outpix, c, v);
                            outpix += v;
                        }
                    }
                }
            }
        } else {
            fseek(fp, c.size, SEEK_CUR);
        }
    }
    JCLIB_Close(fp);
    return TRUE;
  error:
    JCLIB_Close(fp);
    return FALSE;
}

// =======================================================


PUBLIC int PIX_Load(const char* file, unsigned char *outpix, unsigned char * pal, int* width, int* height) {
    if (!PIX_LoadPCX(file, outpix, pal, width, height))
        if (!PIX_LoadLBM(file, outpix, pal, width, height))
            return 0;
    return 1;
}

// ---------------------------- PIX.C -----------------------
