// ----------------------------- TEXT.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.

#ifndef _TEXT_H_
#define _TEXT_H_

#ifndef _BASE_H_
#include <base.h>
#endif

  // Font handling.
typedef struct {
    word  flags;
    byte  width;
    byte  height;
    byte *data;
    word *offs;
} FONT_TFont;

enum {
    FONT_FALPHA = 1,
    FONT_F128   = 2,
    FONT_FTIGHT = 4,
    FONT_CHRMAP = 8,
};

PUBLIC bool FONT_Load(FONT_TFont *font, const char *fname);

PUBLIC void FONT_End(FONT_TFont *font);

PUBLIC int FONT_Index(FONT_TFont *font, int c, byte **pc);

    // Typical fonts of general use by other library routines.
    // Fill or change as you like.
PUBLIC FONT_TFont FONT_System;
PUBLIC FONT_TFont FONT_Big;
PUBLIC FONT_TFont FONT_Mini;
PUBLIC FONT_TFont FONT_Border;


  // Text writer routine.
PUBLIC int TEXT_Write(const FONT_TFont *font, int x, int y, const char *str, int ink);

PUBLIC int TEXT_Printf(const FONT_TFont *font, int x, int y, int ink, const char *fmt, ...);

PUBLIC int TEXT_GetExtent(const FONT_TFont *font, int x, int y, const char *str, int *w, int *h);


  // Commands for the text writer, embedded in the string.
#define TXC_PREFIX  "\xFF"
#define TXC_PREFIXC '\xFF'
  // Carriage return.
#define TXC_CR      TXC_PREFIX "\x0D"
#define TXC_CRC     '\x0D'
  // Line feed.
#define TXC_LF      TXC_PREFIX "\x0A"
#define TXC_LFC     '\x0A'
  // Use font width of 8.
#define TXC_F8      TXC_PREFIX "\x08"
#define TXC_F8C     '\x08'
  // Use font width of 6.
#define TXC_F6      TXC_PREFIX "\x06"
#define TXC_F6C     '\x06'
  // Use font width of 4.
#define TXC_F4      TXC_PREFIX "\x04"
#define TXC_F4C     '\x04'
  // Position cursor at pixel coordinates. Resets margins to X default.
#define TXC_AT(a,b) TXC_PREFIX "\x02" #a "\0" #b "\0"
#define TXC_ATC     '\x02'
  // Set ink colour.
#define TXC_INK(a)  TXC_PREFIX "\x1b" #a "\0"
#define TXC_INKC    '\x1b'
  // Set border colour.
#define TXC_BORDER(a)  TXC_PREFIX "\x1a" #a "\0"
#define TXC_BORDERC    '\x1a'
  // Set right margin for wrapping text.
#define TXC_RM(a)   TXC_PREFIX "\x0e" #a "\0"
#define TXC_RMC     '\x0e'
  // Set left margin for starting after a carriage return.
#define TXC_LM(a)   TXC_PREFIX "\x0f" #a "\0"
#define TXC_LMC     '\x0f'

#endif

// ----------------------------- TEXT.H -------------------------------

