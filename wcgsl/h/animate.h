// ----------------------------- ANIMATE.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Animation (yeah, k00l KinematiKs).

// By now it doesn't support different windows or moving frames.
// Just 320x200 frames.

#ifndef _ANIMATE_H_
#define _ANIMATE_H_

#ifndef _BASE_H_
#include <base.h>
#endif

    // Animation flags.
enum {
    ANIMF_LOOP    = 0x0001,         // Animation loops from end to beginning.
};

/*
    // Animation frame (same format as in the file).
typedef struct {
    dword size;
    word  x0, y0, x1, y1;
    byte  data[1];
} ANIM_TFrame;
*/
    // Animation.
typedef struct {
    int    x, y, w, h;                  // Animated rectangle.
    dword  flags;                       // Animation flags.
    int    nFrames;                     // Total number of frames.
//    ANIM_TFrame **data;                 // Animation data.
    byte **data;                        // Animation data.
    int    curFrame;                    // Current frame number.
    dword  time;                        // Last time the anim was updated.
    int    speed;                       // # ticks in time between frames.
} ANIM_TAnimation;

PUBLIC bool ANIM_Init(ANIM_TAnimation *anim,
                      const char *file,         // Animation filename.
                      int         speed,        // Animation speed.
                      int x, int y);            // Screen position.
    // Returns FALSE if there was an error.

PUBLIC bool ANIM_Play(ANIM_TAnimation *anim,
                      int nframes,              // Positive or negative.
                      dword time);              // This time.
    // Jumps 'nframes' in the animation and updates the picture.
    // Returns FALSE if the end (or beginning) of the animation was reached.

PUBLIC void ANIM_End(ANIM_TAnimation *anim);

#endif

// ----------------------------- ANIMATE.H -------------------------------

