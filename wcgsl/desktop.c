// ----------------------------- DESKTOP.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#include <desktop.h>
#include <bitmap.h>
#include <llscreen.h>
#include <cursor.h>
#include <llkey.h>
#include <stdlib.h>
#include <text.h>
#include <string.h>

PRIVATE const struct {
    BM_TBitmapHeader h;
    byte data[8*8];
} ArrowCursor = {
    {8, 8, BMF_SPRITE},
    {
        15,  7,  8,  0,  0,  0,  0,  0,
         7, 15, 15,  7,  8,  0,  0,  0,
         8, 15, 15, 15, 15,  7,  8,  0,
         0,  7, 15, 15, 15,  0,  0,  0,
         0,  8, 15, 15, 15,  7,  8,  0,
         0,  0,  7,  8,  7, 15,  7,  8,
         0,  0,  0,  0,  8,  7, 15,  7,
         0,  0,  0,  0,  0,  8,  7, 15,
    }
}, CursorBg = {
    {8, 8, 0},
}, *Cursors[] = {
    NULL,
    &ArrowCursor,
    &ArrowCursor,
    &ArrowCursor
};

    // Init a desktop.
bool DSK_Init(DSK_TDesktop *dsk, int numctls,
              int cx0, int cy0, int cx1, int cy1) {
    assert(dsk != NULL);
    assert(numctls >= 0);
    dsk->maxCtls = numctls;
    dsk->numCtls = 0;
    dsk->ctls    = NEW(numctls*sizeof(*dsk->ctls));
    if (numctls > 0 && dsk->ctls == NULL)
        return FALSE;
    dsk->actual   = -1;     // None.
    dsk->tracking = FALSE;
    dsk->repaint  = TRUE;
    dsk->cb       = CURS_GetPosition(&dsk->cx, &dsk->cy);
    dsk->cursor   = DCS_ARROW;

        // Prepare caption background stuff.
    dsk->back = NULL;

    DSK_SetCaptionArea(dsk, cx0, cy0, cx1, cy1);
    return TRUE;
}

    // Adds a control to the desktop.
PUBLIC DSK_PControl DSK_Add(DSK_TDesktop *dsk,
                    int x0, int y0, int x1, int y1,
                    DSK_TControlFunc func, void *data) {
    assert(dsk != NULL);
    assert(dsk->numCtls < dsk->maxCtls);
    dsk->ctls[dsk->numCtls].x0   = x0;
    dsk->ctls[dsk->numCtls].y0   = y0;
    dsk->ctls[dsk->numCtls].x1   = x1;
    dsk->ctls[dsk->numCtls].y1   = y1;
    dsk->ctls[dsk->numCtls].func = func;
    dsk->ctls[dsk->numCtls].data = data;

    dsk->ctls[dsk->numCtls].cursorInside = FALSE;
    dsk->ctls[dsk->numCtls].mustCall     = FALSE;
    dsk->ctls[dsk->numCtls].repaint      = TRUE;

    dsk->numCtls++;
    return dsk->ctls + dsk->numCtls - 1;
}

    // Finds a control in the desktop.
PUBLIC DSK_PControl DSK_Find(DSK_TDesktop *dsk, void *data) {
    int i;
    for (i = 0; i < dsk->numCtls; i++)
        if (data == dsk->ctls[i].data)
            return dsk->ctls + i;
    return NULL;
}

    // Prepares the desktop to print its caption in the given place.
void DSK_SetCaptionArea(DSK_TDesktop *dsk,
                        int cx0, int cy0, int cx1, int cy1) {
    DSK_EndCaptionArea(dsk);
    if (cx0 != cx1 && cy0 != cy1) {
        dsk->cx0 = cx0;
        dsk->cy0 = cy0;
        dsk->cx1 = cx1;
        dsk->cy1 = cy1;
        dsk->caption = "DSK_SetCaptionArea";
        dsk->back = NEW(sizeof(BM_TBitmapHeader)+(cx1-cx0)*(cy1-cy0));
        if (dsk->back != NULL) {
            dsk->back->Header.Width  = cx1-cx0;
            dsk->back->Header.Height = cy1-cy0;
            dsk->back->Header.Flags  = 0;
            BM_Get(cx0, cy0, dsk->back);
        }
    }
}

    // Removes the caption area from the desktop.
void DSK_EndCaptionArea(DSK_TDesktop *dsk) {
    if (dsk->back != NULL) {
        BM_Draw(dsk->cx0, dsk->cy0, dsk->back);
        DISPOSE(dsk->back);
    }
}


#ifdef GRAYCAPTIONBACK
PRIVATE void fillCaption(DSK_TDesktop *dsk) {
    byte *ps, *pp;
    int i, j;

    ps = (byte*)LLS_Screen + dsk->cy0*LLS_SizeX + dsk->cx0;
    for (i = (dsk->cy1 - dsk->cy0); i > 0; i--) {
        pp = ps;
        if (i & 1)
            pp++;
        for (j = (dsk->cx1 - dsk->cx0); j > 0; j-=2, pp += 2)
            *pp = 8;
        ps += LLS_SizeX;
    }
}
#else
#define fillCaption(a)
#endif

    // Runs one step of desktop handling. Returns 0, or the number of the
    // control that requests to end the Run() loop, or -1 if left button.
int DSK_Run(DSK_TDesktop *dsk) {
    word but;
    int x, y, i;
    int ret = 0;
    bool rep;

    assert(dsk != NULL);

    rep = dsk->repaint;
    dsk->repaint  = FALSE;

    if (LLK_Keys[kC])
        CURS_Calibrate();
    but = CURS_GetPosition(&x, &y);
    for (i = 0; i < dsk->numCtls; i++) {
        dword notif = 0;
        word b;
        bool in;
        const char *caption;

        caption = dsk->caption;     // To detect changes in the text.
        b = dsk->cb ^ but;          // Detect changes in buttons.
        in =  (x >= dsk->ctls[i].x0
            && x <  dsk->ctls[i].x1
            && y >= dsk->ctls[i].y0
            && y <  dsk->ctls[i].y1);
        if (!dsk->tracking || i == dsk->actual) {
                // While tracking, actual doesn't change. If not tracking:
            if (in && !dsk->ctls[i].cursorInside) {
                notif |= DCFN_MOUSEIN;
                if (!dsk->tracking)
                    dsk->actual = i;
            } else if (!in && dsk->ctls[i].cursorInside) {
                notif |= DCFN_MOUSEOUT;
                if (!dsk->tracking && dsk->actual == i)
                    dsk->actual = -1;
            }
            dsk->ctls[i].cursorInside = in;
        }

        if (rep)
            notif |= DCFN_PAINT;
        if (dsk->ctls[i].mustCall)
            notif |= DCFN_CONTINUE;

        if (notif != 0 || (i == dsk->actual && b != 0)) {
            if (i == dsk->actual) {
                if (dsk->tracking)
                    notif |= DCFN_TRACKING;
                if (b & 1)
                    if (but & 1)
                        notif |= DCFN_LBUTTONDN;
                    else
                        notif |= DCFN_LBUTTONUP;
                if (b & 2)
                    if (but & 2)
                        notif |= DCFN_RBUTTONDN;
                    else
                        notif |= DCFN_RBUTTONUP;
            }
            if (dsk->ctls[i].func != NULL) {
                int rez;
                rez = dsk->ctls[i].func(dsk, &dsk->ctls[i], notif);
                if (i == dsk->actual)
                    dsk->tracking = FALSE;
                if ( (rez & DCFR_RESETME) != 0) {
                    dsk->ctls[i].cursorInside = FALSE;
                    if (i == dsk->actual)
                        dsk->actual = -1;
                    rez &= ~DCFR_RESETME;
                }
                if (i == dsk->actual && rez == DCFR_TRACK) {
                    dsk->tracking = TRUE;
                    dsk->ctls[i].mustCall = TRUE;
                } else if (rez == DCFR_KEEP) {
                    dsk->ctls[i].mustCall = TRUE;
                } else if (rez == DCFR_EXIT)
                    ret = i+1;
                else {
                    dsk->ctls[i].mustCall = FALSE;
                }
            }
        }
          // Now detect changes in the caption, and reflect them.
        if (dsk->back != NULL) {
            if (   (caption == NULL || *caption == '\0')
                && (dsk->caption != NULL && *dsk->caption != '\0')) {
                fillCaption(dsk);
                TEXT_Write(&FONT_Big,
                           dsk->cx0, dsk->cy0,
                           dsk->caption, 15);

            } else if (   (caption != NULL && *caption != '\0')
                && (dsk->caption == NULL || *dsk->caption == '\0')) {
                BM_Draw(dsk->cx0, dsk->cy0, dsk->back);
            } else if (strcmp(caption, dsk->caption) != 0) {
                BM_Draw(dsk->cx0, dsk->cy0, dsk->back);
                fillCaption(dsk);
                TEXT_Write(&FONT_Big,
                           dsk->cx0, dsk->cy0,
                           dsk->caption, 15);
            }
        }
    }
    if (Cursors[dsk->cursor] != NULL) {
        BM_Get(x, y, &CursorBg);
        BM_Draw(x, y, Cursors[dsk->cursor]);
    }
    LLS_Update();
// ú JC!!!   VGA_VSync();
    if (Cursors[dsk->cursor] != NULL)
        BM_Draw(x, y, &CursorBg);
    dsk->cx = x;
    dsk->cy = y;
    dsk->cb = but;

    if (ret == 0 && (but & 2))
        return -1;
    return ret;
}

void DSK_Reset(DSK_TDesktop *dsk) {
    int i;

    assert(dsk != NULL);
    dsk->actual   = -1;
    dsk->tracking = FALSE;
    dsk->repaint  = TRUE;
    for (i = 0; i < dsk->numCtls; i++)
        dsk->ctls[i].cursorInside = FALSE;
}

    // Sets cursor shape and returns the old one.
int DSK_SetCursorShape(DSK_TDesktop *dsk, int shape) {
    int o;
    assert(dsk != NULL);

    o = dsk->cursor;
    if (shape >= DCS_NONE && shape <= DCS_TEXT)
        dsk->cursor = shape;
    return o;
}

    // Ends the desktop.
void DSK_End(DSK_TDesktop *dsk) {
    assert(dsk != NULL);
    DSK_EndCaptionArea(dsk);
    DISPOSE(dsk->ctls);
    dsk->numCtls = 0;
}

// ----------------------------- DESKTOP.C -------------------------------

