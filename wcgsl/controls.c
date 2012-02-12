// ----------------------------- CONTROLS.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Predefined desktop controls.

#include <controls.h>
#include <animate.h>
#include <jclib.h>

// ------------
// Simple control. Just requests to end when pressed.

int CTL_SimpleFunc(DSK_TDesktop *dsk, DSK_TControl *ctl, dword reason) {
    assert(dsk != NULL);
    assert(ctl != NULL);

    if (reason & DCFN_MOUSEIN)
        dsk->caption = ((CTL_TSimple*)(ctl->data))->caption;
    else if (reason & DCFN_MOUSEOUT)
        if (dsk->caption == ((CTL_TSimple*)(ctl->data))->caption)
            dsk->caption = "";
    if (reason & DCFN_LBUTTONDN)
        return DCFR_EXIT;
    return DCFR_ALLDONE;
}

// ------------
// Animated control. Shows an animation when cursor enters it.

void CTL_AnimatedInit(CTL_TAnimated *c, const char *animFile, int speed,
                      volatile dword *time,
                      const char *caption) {
    c->caption = caption;
    c->time    = time;
    ANIM_Init(&c->anim, animFile, speed, 0, 0);
    c->state = CTLA_CLOSED;
}

int  CTL_AnimatedFunc(DSK_TDesktop *dsk, DSK_TControl *ctl, dword reason) {
    CTL_TAnimated *a = (CTL_TAnimated *)ctl->data;
    int ret = DCFR_ALLDONE;

    assert(dsk != NULL);
    assert(ctl != NULL);
    if (reason & DCFN_MOUSEIN) {
        dsk->caption = a->caption;
        a->state = CTLA_OPENING;
    }
    if (reason & DCFN_MOUSEOUT) {
        if (dsk->caption == a->caption)
            dsk->caption = "";
        a->state = CTLA_CLOSING;
    }
    if (a->state == CTLA_OPENING || a->state == CTLA_CLOSING) {
        if (reason & DCFN_PAINT) {
            if (a->state == CTLA_OPENING) {
                a->anim.curFrame = 0;
                ret = DCFR_KEEP;
            } else if (a->state == CTLA_CLOSING) {
                a->state = CTLA_CLOSED;
            }
        } else if (ANIM_Play(&a->anim, a->state, *a->time))
            ret = DCFR_KEEP;
        else {
            if (a->state == CTLA_OPENING)
                a->state = CTLA_OPEN;
            else if (a->state == CTLA_CLOSING)
                a->state = CTLA_CLOSED;
        }
    }

    if (reason & DCFN_LBUTTONUP) {
        if (ctl->cursorInside)
            ret = DCFR_EXIT;
    } else if (reason & (DCFN_LBUTTONDN | DCFN_TRACKING))
            ret = DCFR_TRACK;
    return ret;
}

void CTL_AnimatedEnd (CTL_TAnimated *c) {
    ANIM_End(&c->anim);
}

// ------------
// Bitmapped button. Has up to three states : normal, pushed, and disabled.

bool CTL_ReadButtonBitmaps(BM_TBitmap *bitmaps[3], const char *fname,
                           int w, int h) {
    FILE *f;
    int i;

    assert(bitmaps != NULL);
    assert(fname   != NULL);

    for (i = 0; i < 3; i++)
        bitmaps[i] = NULL;

    f = JCLIB_Open(fname);
    if (f == NULL)
        return FALSE;
    for (i = 0; i < 3; i++) {
        bitmaps[i] = NEW(sizeof(BM_TBitmapHeader) + w*h);
        if (bitmaps[i] == NULL)
            break;

        bitmaps[i]->Header.Width  = w;
        bitmaps[i]->Header.Height = h;
        bitmaps[i]->Header.Flags  = 0;
        if (fread(bitmaps[i]->Data, w*h, 1, f) != 1) {
            DISPOSE(bitmaps[i]);
            break;
        }
    }
    JCLIB_Close(f);
    return (i > 0);
}

void CTL_ButtonInit(CTL_TButton *c, BM_TBitmap *bitmaps[3],
                    const char *caption) {
    assert(c != NULL);

    c->bitmaps[0] = bitmaps[0];
    c->bitmaps[1] = bitmaps[1];
    c->bitmaps[2] = bitmaps[2];
    c->state      = CTLB_NORMAL;
    c->caption    = caption;
}

int CTL_ButtonFunc(DSK_TDesktop *dsk, DSK_TControl *ctl, dword reason) {
    CTL_TButton *c = (CTL_TButton *)ctl->data;
    int ret = DCFR_ALLDONE;
    int st = c->state;

    assert(c != NULL);
    assert(dsk != NULL);

    if (c->state != CTLB_DISABLED) {
        if (reason & DCFN_MOUSEIN) {
            dsk->caption = c->caption;
        }
        if (reason & DCFN_MOUSEOUT) {
            if (dsk->caption == c->caption)
                dsk->caption = "";
        }

        if (reason & DCFN_LBUTTONUP) {
            if (ctl->cursorInside)
                ret = DCFR_EXIT;
            ret |= DCFR_RESETME;
            c->state = CTLB_NORMAL;
        } else if (reason & (DCFN_LBUTTONDN | DCFN_TRACKING)) {
            ret = DCFR_TRACK;
            if (reason & DCFN_LBUTTONDN)
                c->state = CTLB_PRESSED;
        }
    }
    if (st != c->state || (reason & DCFN_PAINT)) {
        st = c->state;
        if (c->bitmaps[st] == NULL)
            st = 0;
        if (c->bitmaps[st] != NULL)
            BM_Draw(ctl->x0, ctl->y0, c->bitmaps[st]);
    }
    return ret;
}

void CTL_ButtonEnd (CTL_TButton *c) {
    assert(c != NULL);
}


// --------------------------------------
// Scrollbar and listbox.

#define SBBSIZE 12

typedef struct {
    int state, captp;
    int which;
    CTL_PScrollBar bar;
    DSK_PControl ctl;
} TSBButton;

typedef struct CTL_SScrollBarChilds {
    int x, y, len;
    TSBButton a1, a2, b1, b2, sl;
} TScrollBarChilds, *PScrollBarChilds;

PRIVATE void SBGetScrData(CTL_TScrollBar *c, int *d1, int *d2) {
    if (c->pos < c->min)
        c->pos = c->min;
    if (c->pos > c->max - c->shown)
        c->pos = c->max - c->shown;

    if (d1 != NULL)
        *d1 = (c->childs->len)*(c->pos-c->min)/(c->max - c->min);
    if (d2 != NULL)
        *d2 = (c->childs->len)*(c->pos-c->min+c->shown)/(c->max - c->min);
}

PRIVATE int SBGetPosData(CTL_TScrollBar *c, int scr) {
    return c->min + scr*(c->max - c->min)/(c->childs->len);
}


PRIVATE void SBButtonInit(TSBButton *c, int which, CTL_PScrollBar bar) {
    assert(c != NULL);

    c->state = CTLB_NORMAL;
    c->which = which;
    c->bar   = bar;
    c->captp = 0;
}

PRIVATE int SBButtonFunc(DSK_TDesktop *dsk, DSK_TControl *ctl, dword reason) {
    TSBButton *c = (TSBButton *)ctl->data;
    int ret = DCFR_ALLDONE;
    int st = c->state;
    static char *buttontext[5] = {"", "", "", "", ""};

    assert(c != NULL);
    assert(dsk != NULL);

    if (c->state != CTLB_DISABLED) {
        if (reason & DCFN_LBUTTONUP) {
            if (ctl->cursorInside)
                ret = DCFR_EXIT;
            ret |= DCFR_RESETME;
            c->state = CTLB_NORMAL;
        } else if (reason & (DCFN_LBUTTONDN | DCFN_TRACKING)) {
            ret = DCFR_TRACK;
            if (reason & DCFN_LBUTTONDN) {
                if (c->state != CTLB_PRESSED) {
                    c->captp = dsk->cy + (c->bar->childs->y + SBBSIZE) - ctl->y0;
                }
                c->state = CTLB_PRESSED;
            }
            if (c->which == 2) {
                CTL_SetScrollBarPos(c->bar, SBGetPosData(c->bar, dsk->cy - c->captp));
            }
        }
    }
    if (st != c->state || (reason & DCFN_PAINT)) {
//        bool hide;
//        hide = CURS_HideCursorRect(ctl->x0, ctl->y0, ctl->x1, ctl->y1);
        st = c->state;
        if (st == CTLB_NORMAL) {
//            M12_FillRectangle(ctl->x0, ctl->y0, ctl->x1, ctl->y1, TEX(7,7));
            if (c->which == 1 || c->which == 3)
;
            else {
//                M12_BoxRelief(ctl->x0, ctl->y0, ctl->x1, ctl->y1, 1, TEX(15,15),TEX(8,8));
                if (c->which != 2) {
/*
                    M12_PrepareTexture(0, TEX(15,15));
                    M12_Printf(ctl->x0 + 2+1, ctl->y0+1+1, buttontext[c->which]);
                    M12_PrepareTexture(0, TEX(12,12));
                    M12_Printf(ctl->x0 + 2, ctl->y0+1, buttontext[c->which]);
*/
                }
            }
        } else if (st == CTLB_PRESSED) {
            if (c->which == 1 || c->which == 3)
;//                M12_FillRectangle(ctl->x0, ctl->y0, ctl->x1, ctl->y1, TEX(8,8));
            else {
//                M12_FillRectangle(ctl->x0, ctl->y0, ctl->x1, ctl->y1, TEX(7,7));
//                M12_BoxRelief(ctl->x0, ctl->y0, ctl->x1, ctl->y1, -1, TEX(15,15),TEX(8,8));
                if (c->which != 2) {
/*
                    M12_PrepareTexture(0, TEX(8,8));
                    M12_Printf(ctl->x0 + 2+1, ctl->y0+1+1, buttontext[c->which]);
                    M12_PrepareTexture(0, TEX(14,14));
                    M12_Printf(ctl->x0 + 2, ctl->y0+1, buttontext[c->which]);
*/
                }
            }
        } else if (st == CTLB_DISABLED) {
//            M12_FillRectangle(ctl->x0, ctl->y0, ctl->x1, ctl->y1, TEX(7,7));
            if (c->which == 1 || c->which == 3)
;
            else {
//                M12_BoxRelief(ctl->x0, ctl->y0, ctl->x1, ctl->y1, 1, TEX(15,15),TEX(8,8));
                if (c->which != 2) {
//                    M12_PrepareTexture(0, TEX(15,15));
//                    M12_Printf(ctl->x0 + 2, ctl->y0+1, buttontext[c->which]);
                }
            }
        }
//        if (hide)
//            CURS_ShowCursor();
    }
    return ret;
}

void SBButtonEnd (TSBButton *c) {
    assert(c != NULL);
}


PUBLIC void CTL_ScrollBarInit(CTL_TScrollBar *c, DSK_TDesktop *dsk,
                              int x, int y, int len,
                              int flags, int min, int max, int shown, int pos) {
    PScrollBarChilds sc;
    int d1, d2;
    assert(c != NULL);
    assert(dsk != NULL);
//    assert(min < max);

    REQUIRE( (sc = NEW(sizeof(*sc))) != NULL);
    SBButtonInit(&sc->a1, 0, c);
    SBButtonInit(&sc->b1, 1, c);
    SBButtonInit(&sc->sl, 2, c);
    SBButtonInit(&sc->b2, 3, c);
    SBButtonInit(&sc->a2, 4, c);
    c->childs = sc;
    sc->x = x;
    sc->y = y;
    sc->len = len - 2*SBBSIZE;
    if (shown <= 0)
        shown = 1;
    if (max < min+shown)
        max = min+shown;
    c->min = min;
    c->max = max;
    c->pos = pos;
    c->shown = shown;
    c->flags = flags;
    c->dsk   = dsk;
    SBGetScrData(c, &d1, &d2);
//M12_BoxRelief(x - 1, y - 1, x + SBBSIZE + 1, y + len + 1, -1, TEX(15,15),TEX(8,8));
    sc->a1.ctl = DSK_Add(dsk, x, y, x + SBBSIZE, y + SBBSIZE, SBButtonFunc, &sc->a1);
    sc->b1.ctl = DSK_Add(dsk, x, y+SBBSIZE, x + SBBSIZE, y + SBBSIZE+d1, SBButtonFunc, &sc->b1);
    sc->sl.ctl = DSK_Add(dsk, x, y+SBBSIZE+d1, x + SBBSIZE, y+SBBSIZE+d2, SBButtonFunc, &sc->sl);
    sc->b2.ctl = DSK_Add(dsk, x, y+SBBSIZE+d2, x + SBBSIZE, y + len - SBBSIZE, SBButtonFunc, &sc->b2);
    sc->a2.ctl = DSK_Add(dsk, x, y+len-SBBSIZE, x + SBBSIZE, y+len, SBButtonFunc, &sc->a2);
}

PUBLIC void CTL_SetScrollBarPos(CTL_TScrollBar *c, int pos) {
    int d1, d2;

    if (pos < c->min)
        pos = c->min;
    if (pos > c->max - c->shown)
        pos = c->max - c->shown;

    if (pos <= c->min && c->childs->a1.state != CTLB_DISABLED) {
        c->childs->a1.state = CTLB_DISABLED;
        c->childs->a1.ctl->repaint = TRUE;
    } else if (pos > c->min && c->childs->a1.state == CTLB_DISABLED) {
        c->childs->a1.state = CTLB_NORMAL;
        c->childs->a1.ctl->repaint = TRUE;
    }
    if (pos >= c->max - c->shown && c->childs->a2.state != CTLB_DISABLED) {
        c->childs->a2.state = CTLB_DISABLED;
        c->childs->a2.ctl->repaint = TRUE;
    } else if (pos < c->max - c->shown && c->childs->a2.state == CTLB_DISABLED) {
        c->childs->a2.state = CTLB_NORMAL;
        c->childs->a2.ctl->repaint = TRUE;
    }

    if (pos == c->pos)
        return;
    c->pos = pos;
    SBGetScrData(c, &d1, &d2);
    c->childs->b1.ctl->y1 = c->childs->y + SBBSIZE + d1;
    c->childs->b1.ctl->repaint = TRUE;
    c->childs->sl.ctl->y0 = c->childs->y + SBBSIZE + d1;
    c->childs->sl.ctl->repaint = TRUE;
    c->childs->sl.ctl->y1 = c->childs->y + SBBSIZE + d2;
    c->childs->b2.ctl->y0 = c->childs->y + SBBSIZE + d2;
    c->childs->b2.ctl->repaint = TRUE;
}

PUBLIC void CTL_SetScrollBarRange(CTL_TScrollBar *c, int min, int max, int shown) {
    int d1, d2;

    if (shown <= 0)
        shown = 1;
    if (max < min+shown)
        max = min+shown;

    if (c->pos <= min && c->childs->a1.state != CTLB_DISABLED) {
        c->childs->a1.state = CTLB_DISABLED;
        c->childs->a1.ctl->repaint = TRUE;
    } else if (c->pos > min && c->childs->a1.state == CTLB_DISABLED) {
        c->childs->a1.state = CTLB_NORMAL;
        c->childs->a1.ctl->repaint = TRUE;
    }
    if (c->pos >= max - c->shown && c->childs->a2.state != CTLB_DISABLED) {
        c->childs->a2.state = CTLB_DISABLED;
        c->childs->a2.ctl->repaint = TRUE;
    } else if (c->pos < max - c->shown && c->childs->a2.state == CTLB_DISABLED) {
        c->childs->a2.state = CTLB_NORMAL;
        c->childs->a2.ctl->repaint = TRUE;
    }

    if (min == c->min && max == c->max && shown == c->shown)
        return;
    c->min = min;
    c->max = max;
    c->shown = shown;
    SBGetScrData(c, &d1, &d2);
    c->childs->b1.ctl->y1 = c->childs->y + SBBSIZE + d1;
    c->childs->b1.ctl->repaint = TRUE;
    c->childs->sl.ctl->y0 = c->childs->y + SBBSIZE + d1;
    c->childs->sl.ctl->repaint = TRUE;
    c->childs->sl.ctl->y1 = c->childs->y + SBBSIZE + d2;
    c->childs->b2.ctl->y0 = c->childs->y + SBBSIZE + d2;
    c->childs->b2.ctl->repaint = TRUE;
}

PUBLIC int  CTL_ScrollBarFunc(DSK_TDesktop *dsk, DSK_TControl *ctl, dword reason) {
    return DCFR_ALLDONE;
}

PUBLIC void CTL_ScrollBarEnd (CTL_TScrollBar *c) {
    PScrollBarChilds sc;
    sc = (PScrollBarChilds)(c->childs);
    SBButtonEnd(&sc->a1);
    SBButtonEnd(&sc->a2);
    SBButtonEnd(&sc->b1);
    SBButtonEnd(&sc->b1);
    SBButtonEnd(&sc->sl);
    DISPOSE(c->childs);
}

// ---------------------------------------------
// Listbox.
// Has a scrollbar.


PRIVATE bool DrawListboxLine(CTL_PListbox c, int el, bool erase) {
//    bool hide;
    int i;
    DSK_TControl *ctl = c->ctl;
    const char *p;

    if (el >= (c->sb.pos + c->sb.shown) || (!erase && el >= c->sl.nstr))
        return FALSE;
    i = el - c->sb.pos;
 /*
    hide = CURS_HideCursorRect(ctl->x0, ctl->y0+i*8,
                               ctl->x1, ctl->y0+(i+1)*8);
    M12_FillRectangle(ctl->x0, ctl->y0+i*8,
                      ctl->x1, ctl->y0+(i+1)*8,
                      TEX(0,0));
 */
    p = STL_Get(&c->sl, el);
    if (p != NULL) {
//        M12_PrepareTexture(0, TEX(14,14));
//        M12_Printf(ctl->x0, ctl->y0+8*i, "%s", STL_Get(&c->sl, el));
    }
//    if (hide)
//        CURS_ShowCursor();
    return TRUE;
}


PRIVATE int ListboxFunc(DSK_TDesktop *dsk, DSK_TControl *ctl, dword reason) {
    CTL_PListbox c = (CTL_PListbox)ctl->data;
    if (reason & DCFN_PAINT) {
        int i;
        for (i = 0; i < c->sb.shown; i++)
            DrawListboxLine(c, i + c->sb.pos, TRUE);
    }
    if (reason & DCFN_LBUTTONDN)
        return DCFR_EXIT;
    return DCFR_ALLDONE;
}

PUBLIC void CTL_ListboxInit(CTL_PListbox c, DSK_TDesktop *dsk,
                            int x0, int y0, int x1, int y1) {
    assert(c != NULL);
    c->oldp = 0;
    STL_Init(&c->sl);
    c->ctl = DSK_Add(dsk, x0, y0, x1, y1, ListboxFunc, c);
    CTL_ScrollBarInit(&c->sb, dsk, x1+1, y0+1, y1-y0-2,
                      CTLSB_VERTICAL, 0, 0, (y1-y0)/8, 0);
}

PUBLIC void CTL_ListboxAdd(CTL_PListbox c, const char *str) {
    assert(c != NULL);
    STL_Add(&c->sl, str);
    DrawListboxLine(c, c->sl.nstr - 1, FALSE);
    CTL_SetScrollBarRange(&c->sb, c->sb.min, c->sl.nstr, c->sb.shown);
//    CTL_SetScrollBarPos(&c->sb, c->sb.pos); // Refresh.
}

PUBLIC int CTL_ControlListbox(CTL_PListbox c, int rez) {
    int ret;

    if (c->sb.pos > c->sl.nstr)
        c->sb.pos = c->sl.nstr;
    CTL_SetScrollBarRange(&c->sb, c->sb.min, c->sl.nstr, c->sb.shown);
    rez--;
    rez -= (c->ctl - c->sb.dsk->ctls);
    if (rez == 1)
        CTL_SetScrollBarPos(&c->sb, c->sb.pos-1);
    else if (rez == 2)
        CTL_SetScrollBarPos(&c->sb, c->sb.pos-10);
    else if (rez == 3)
        ;
    else if (rez == 4)
        CTL_SetScrollBarPos(&c->sb, c->sb.pos+10);
    else if (rez == 5)
        CTL_SetScrollBarPos(&c->sb, c->sb.pos+1);
    if (c->sb.pos != c->oldp) {
        c->ctl->repaint = TRUE;
        c->oldp = c->sb.pos;
    }
    if (rez != 0)
        return -1;
    ret = c->sb.pos + (c->sb.dsk->cy - c->ctl->y0)/8;
    if (ret >= c->sl.nstr)
        ret = -1;
    return ret;
}

PUBLIC void CTL_ListboxEnd(CTL_PListbox c) {
    STL_End(&c->sl);
    CTL_ScrollBarEnd(&c->sb);
}


// ----------------------------- CONTROLS.C -------------------------------

