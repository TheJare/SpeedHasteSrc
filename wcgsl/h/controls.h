// ----------------------------- CONTROLS.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Predefined desktop controls.

#ifndef _CONTROLS_H_
#define _CONTROLS_H_

#include <desktop.h>
#include <animate.h>
#include <bitmap.h>
#include <strlist.h>

// ------------
// Simple control. Just requests to end when pressed.

typedef struct {
    DSK_PControl ctl;       // Assign to this with תתת.ctl = DSK_Add();
    const char *caption;
} CTL_TSimple;

PUBLIC void CTL_SimpleInit(CTL_TSimple *c, const char *caption);
PUBLIC int  CTL_SimpleFunc(DSK_TDesktop *dsk, DSK_TControl *ctl, dword reason);
PUBLIC void CTL_SimpleEnd (CTL_TSimple *c);

#define CTL_SimpleInit(a,c) ((a)->caption = (c))
#define CTL_SimpleEnd(a)

// ------------
// Animated control. Shows an animation when cursor enters it.

    // Animation states.
enum {
    CTLA_CLOSED  =  0,
    CTLA_OPENING =  1,
    CTLA_OPEN    =  2,
    CTLA_CLOSING = -1
};

typedef struct {
    DSK_PControl ctl;       // Assign to this with תתת.ctl = DSK_Add();
    const char     *caption;
    ANIM_TAnimation anim;
    int             state;
    volatile dword *time;
} CTL_TAnimated;

PUBLIC void CTL_AnimatedInit(CTL_TAnimated *c, const char *animFile, int speed,
                             volatile dword *time,
                             const char *caption);
PUBLIC int  CTL_AnimatedFunc(DSK_TDesktop *dsk, DSK_TControl *ctl, dword reason);
PUBLIC void CTL_AnimatedEnd (CTL_TAnimated *c);

// ------------
// Bitmapped button. Has up to three states : normal, pushed, and disabled.

    // Button states.
enum {
    CTLB_NORMAL,
    CTLB_PRESSED,
//    CTLB_TRACKING,
    CTLB_DISABLED,
};

typedef struct {
    DSK_PControl ctl;       // Assign to this with תתת.ctl = DSK_Add();
    const char     *caption;
    BM_TBitmap     *bitmaps[3];
    int             state;
} CTL_TButton;

PUBLIC bool CTL_ReadButtonBitmaps(BM_TBitmap *bitmaps[3], const char *fname,
                                  int w, int h);

PUBLIC void CTL_ButtonInit(CTL_TButton *c, BM_TBitmap *bitmaps[3],
                           const char *caption);
PUBLIC int  CTL_ButtonFunc(DSK_TDesktop *dsk, DSK_TControl *ctl, dword reason);
PUBLIC void CTL_ButtonEnd (CTL_TButton *c);

// -----------------------------------------------
// Scroll bar.
// Has arrows, bars and slider.

enum {
    CTLSB_HORIZONTAL,
    CTLSB_VERTICAL,
};

struct CTL_SScrollBarChilds;

typedef struct {
    DSK_PDesktop    dsk;
    int flags;
    int min, max, shown, pos;
    struct CTL_SScrollBarChilds *childs;
} CTL_TScrollBar, *CTL_PScrollBar;

PUBLIC void CTL_ScrollBarInit(CTL_TScrollBar *c, DSK_TDesktop *dsk,
                              int x, int y, int len,
                              int flags, int min, int max, int shown, int pos);

PUBLIC void CTL_SetScrollBarPos(CTL_TScrollBar *c, int pos);

PUBLIC void CTL_SetScrollBarRange(CTL_TScrollBar *c, int min, int max, int shown);

//PUBLIC int  CTL_ScrollBarFunc(DSK_TDesktop *dsk, DSK_TControl *ctl, dword reason);

PUBLIC void CTL_ScrollBarEnd (CTL_TScrollBar *c);


// ---------------------------------------------
// Listbox.
// Has a scrollbar.

typedef struct {
    DSK_PControl ctl;
    int oldp;
    CTL_TScrollBar sb;
    STL_TStringList sl;
} CTL_TListbox, *CTL_PListbox;

PUBLIC void CTL_ListboxInit(CTL_PListbox c, DSK_TDesktop *dsk,
                            int x0, int y0, int x1, int y1);

PUBLIC void CTL_ListboxAdd(CTL_PListbox c, const char *str);

PUBLIC int  CTL_ControlListbox(CTL_PListbox c, int rez);

PUBLIC void CTL_ListboxEnd(CTL_PListbox c);



#endif

// ----------------------------- CONTROLS.H -------------------------------

