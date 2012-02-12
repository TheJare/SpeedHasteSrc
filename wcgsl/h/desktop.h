// ----------------------------- DESKTOP.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#ifndef _DESKTOP_H_
#define _DESKTOP_H_

#include <bitmap.h>

#ifndef _BASE_H_
#include <base.h>
#endif

struct DSK_SControl;
struct DSK_SDesktop;

typedef int (*DSK_TControlFunc)(struct DSK_SDesktop *dsk,    // Current desktop.
                                struct DSK_SControl *ctl,    // Your control.
                                dword reason);               // Why I call you.

enum {
        // Control function return codes.
    DCFR_ALLDONE  = 0,              // Don't call me until situation changes.
    DCFR_KEEP     = 1,              // Keep calling me until I return ALLDONE.
    DCFR_TRACK    = 2,              // Behave like you're over me.
    DCFR_EXIT     = 3,              // Notify the caller to end the desktop.

    DCFR_RESETME  = 0x8000,         // Reset my data, as if I were new.

        // Control function notification codes.
    DCFN_CONTINUE  = 0x0001,        // You asked me to call you.
    DCFN_TRACKING  = 0x0002,        // You are tracking the cursor.
    DCFN_MOUSEIN   = 0x0004,        // Mouse has entered your area.
    DCFN_MOUSEOUT  = 0x0008,        // Mouse has exited your area.
    DCFN_LBUTTONDN = 0x0010,        // Left button was pressed inside you.
    DCFN_LBUTTONUP = 0x0020,        // Left button was released inside you.
    DCFN_RBUTTONDN = 0x0040,        // Right button was pressed inside you.
    DCFN_RBUTTONUP = 0x0080,        // Right button was released inside you.
    DCFN_BUTTONS   = 0x00F0,        // Button event mask.
    DCFN_PAINT     = 0x0100,        // Screen has changed - paint yourself.
};

// --------------
// Control and Desktop structures.

typedef struct DSK_SControl {
    int  x0, y0, x1, y1;        // Rectangle delimiting the control area.
    bool mustCall;              // Must call this control with CONTINUE.
    bool cursorInside;          // Cursor is located inside the control.
    bool repaint;               // Flag to force repaint of control.
    DSK_TControlFunc func;      // Control function.
    void            *data;      // Data for the function to work on.
} DSK_TControl, *DSK_PControl;

typedef struct DSK_SDesktop {
    int           maxCtls;      // Maximum number of controls in the desktop.
    int           numCtls;      // Current number of controls in the desktop.
    DSK_TControl *ctls;         // Array for the controls.
    int           actual;       // Currently selected control.
    bool          tracking;     // If tracking actual control.
    bool          repaint;      // If controls must be repainted.
    word          cb;           // Cursor buttons pressed.
    int           cx, cy;       // Cursor buttons pressed.
    int           cursor;       // Current cursor.

    const char   *caption;      // Text to display in a kind of status bar.
                                // NEVER set to NULL. Set to "" instead.
    int           cx0, cy0,
                  cx1, cy1;     // Rectangle of the status bar.
    BM_TBitmap   *back;         // Background of the status bar.
} DSK_TDesktop, *DSK_PDesktop;

// Cursor shapes.

enum {
    DCS_NONE,       // No cursor (hidden).
    DCS_ARROW,      // Good ol' arrow.
    DCS_CROSS,      // Simple cross.
    DCS_TEXT        // Some nice text cursor.
};

PUBLIC bool DSK_Init(DSK_TDesktop *dsk, int numctls,
                     int cx0, int cy0, int cx1, int cy1);
    // Init a desktop.

PUBLIC DSK_PControl DSK_Add(DSK_TDesktop *dsk,
                            int x0, int y0, int x1, int y1,
                            DSK_TControlFunc func, void *data);
    // Adds a control to the desktop.

PUBLIC DSK_PControl DSK_Find(DSK_TDesktop *dsk, void *data);
    // Finds a control in the desktop.

    // Caption handling. Controls can call this to change the location
    // of the caption, but they must set the text by assigning it
    // to dsk->caption, i.e. dsk->caption = "My house".
PUBLIC void DSK_SetCaptionArea(DSK_TDesktop *dsk,
                               int cx0, int cy0, int cx1, int cy1);
    // Prepares the desktop to print its caption in the given place.

PUBLIC void DSK_EndCaptionArea(DSK_TDesktop *dsk);
    // Removes the caption area from the desktop.


PUBLIC int  DSK_Run(DSK_TDesktop *dsk);
    // Runs one step of desktop handling. Returns 0, or the number of the
    // control that requests to end the Run() loop, or -1 if left button.

PUBLIC void DSK_Reset(DSK_TDesktop *dsk);
    // Resets all controls to the default state (mouse not in, etc.).

PUBLIC int  DSK_SetCursorShape(DSK_TDesktop *dsk, int shape);
    // Sets cursor shape and returns the old one.

PUBLIC void DSK_End(DSK_TDesktop *dsk);
    // Ends the desktop.

#endif

// ----------------------------- DESKTOP.H -------------------------------

