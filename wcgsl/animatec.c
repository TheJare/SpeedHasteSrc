// ----------------------------- ANIMATEC.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Animation (yeah, k00l KinematiKs).

// By now it doesn't support different windows or moving frames.
// Just 320x200 frames.

#include <animate.h>
#include <llscreen.h>
#include <jclib.h>
#include <string.h>
#include <stdlib.h>

    // Returns FALSE if there was an error.
bool ANIM_Init(ANIM_TAnimation *anim,
               const char *file,         // Animation filename.
               int         speed,        // Animation speed.
               int x, int y)             // Screen position.
{
    struct {
        char sign[4];
        word nframes,
             flags;
    } h;
    FILE *f;
    int i;

    assert(anim != NULL);
    anim->data = NULL;
    anim->nFrames  = 0;
    anim->curFrame = 0;
    anim->speed    = speed;
    anim->time     = 0;
    if ( (f = JCLIB_Open(file)) == NULL)
        return FALSE;
    fread(&h, sizeof(h), 1, f);
    if (memcmp(h.sign, ".IAF", 4) != 0) {
        JCLIB_Close(f);
        return FALSE;
    }
        // ALL these ignored.
    anim->x = x;
    anim->y = y;
    anim->w = 320;
    anim->h = 200;

    anim->flags   = h.flags;
    if ( (anim->data = NEW(h.nframes*sizeof(*anim->data))) == NULL) {
        JCLIB_Close(f);
        return FALSE;
    }

    for (i = 0; i < h.nframes; i++) {
        dword size;

        fread(&size, sizeof(size), 1, f);
        if (size > 4) {
            if ( (anim->data[i] = NEW(size-12)) == NULL) {
                while (--i >= 0)
                    DISPOSE(anim->data[i]);
                DISPOSE(anim->data);
                JCLIB_Close(f);
                return FALSE;
            }
            fseek(f, 8, SEEK_CUR);
            fread(anim->data[i], size-12, 1, f);
        } else
            anim->data[i] = NULL;
    }
    anim->nFrames = h.nframes;
    JCLIB_Close(f);
    return TRUE;
}

PRIVATE void doFrame(const ANIM_TAnimation *anim) {
    dword tot;
    byte  *p, *q;

    if ( (p = anim->data[anim->curFrame]) == NULL)
        return;
    tot = 0;
    q = LLS_Screen;
    while (q < (byte*)LLS_Screen + LLS_Size) {
        int  i;
        byte c;

        // Do equal run.

        i = *p++;
        if (i == 255)
            i += *((word *)p)++;
        c = *p++;
        if (c != 0) {
            dword c32 = (c << 24) | (c << 16) | (c << 8) | (c << 0);
            int j = (i >> 2);
            i &= 3;

            while (j > 0) {
                *(dword *)q = *(dword *)q ^ c32;
                ((dword *)q)++;
                j--;
            }
            while (i > 0) {
                *q = *q ^ c;
                q++;
                i--;
            }
        } else
            q += i;

        if (q >= (byte*)LLS_Screen + LLS_Size)
            break;

        // Do raw data run.

        i = *p++;
        if (i == 255)
            i += *((word *)p)++;
        {
            int j = (i >> 2);
            i &= 3;
            while (j > 0) {
                *(dword*)q = *(dword*)q ^ *((dword*)p)++;
                ((dword*)q)++;
                j--;
            }
        }
        while (i > 0) {
            *q = *q ^ *p++;
            q++;
            i--;
        }
    }
    assert(q == (byte*)LLS_Screen + LLS_Size);
}



/*
PUBLIC void ANIM_doFrame(byte *dest, const byte *from, int nbyt);
#pragma aux ANIM_doFrame parm [EDI] [ESI] [ECX]
*/
/*
PRIVATE void doFrame(const ANIM_TAnimation *anim) {
    dword tot;
    byte  *p, *q;

    if ( (p = anim->data[anim->curFrame]) == NULL)
        return;
    tot = 0;
    q = LLS_Screen;
    while (q < (byte*)LLS_Screen + LLS_Size) {
        int  i;
        byte c;

        // Do equal run.

        i = *p++;
        c = *p++;
        if (c != 0) {
            // memset(q, c, i);
            {
                int j = i;
                while (j-- > 0) {
                    *q = *q ^ c;
                    q++;
                }
            }
            //q += i;
            if (i == 255) {
                i = *((word *)p)++;
                c = *p++;
                //memset(q, c, i);
                {
                    int j = i;
                    while (j-- > 0) {
                        *q = *q ^ c;
                        q++;
                    }
                }
                //q += i;
            }
        } else {
            if (i == 255) {
                i += *((word *)p)++;
                p++;
            }
            q += i;
        }

        if (q >= (byte*)LLS_Screen + LLS_Size)
            break;
        // Do raw data run.
        i = *p++;
        //memcpy(q, p, i);
        {
            int j = i;
            while (j-- > 0) {
                *q = *q ^ *p++;
                q++;
            }
        }
//        p += i;
//        q += i;
        if (i == 255) {
            i = *((word *)p)++;
            c = *p++;
            //memcpy(q, p, i);
            {
                int j = i;
                while (j-- > 0) {
                    *q = *q ^ *p++;
                    q++;
                }
            }
            //p += i;
            //q += i;
        }
    }
    assert(q == (byte*)LLS_Screen + LLS_Size);
}
*/

    // Jumps 'nframes' in the animation and updates the picture.
    // Returns FALSE if the end (or beginning) of the animation was reached.
bool ANIM_Play(ANIM_TAnimation *anim,
               int nframes,              // Positive or negative.
               dword time)               // This time.
{
    assert(anim != NULL);

    if (anim->data == NULL)
        return FALSE;
    if (time != 0 && anim->time != 0 && anim->speed > 0
        && (time - anim->time) < anim->speed)
        return TRUE;

    anim->time = time;
    if (nframes > 0) {
        while (nframes-- > 0 && anim->curFrame < anim->nFrames) {
            doFrame(anim);
            anim->curFrame++;
        }
        return (anim->curFrame < anim->nFrames);
    } else if (nframes < 0) {
        while (nframes++ < 0 && anim->curFrame > 0) {
            anim->curFrame--;
            doFrame(anim);
        }
        return (anim->curFrame > 0);
    }
    return TRUE;
}


void ANIM_End(ANIM_TAnimation *anim) {
    int i;

    assert(anim != NULL);

    if (anim->data != NULL) {
        for (i = 0; i < anim->nFrames; i++)
            DISPOSE(anim->data[i]);
        DISPOSE(anim->data);
    }
}

// ----------------------------- ANIMATEC.C -------------------------------

