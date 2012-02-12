// ------------------------ TEXTSCR.H ----------------------
// Text screen handy functions. Must get something done for the
// color support, thou...

#ifndef _TEXTSCR_H_
#define _TEXTSCR_H_

#include "base.h"

#define TXS_DoubleFrame "赏缓纪群"
//#define TXS_DownFrame        "悄逗纪群"
#define TXS_DownFrame   "掏购纪群"

/*
#define CBACK   0x07
#define CSHADOW 0x08
#define CDLGF   0x17
#define CDLGI   0x17
#define CDLGTIT 0x1F
#define CDLGBOT 0x1E
#define CDLGT   0x17
#define CDLGB   0x70
#define CDLGTIP 0x0E
*/
/*
#define CBACK   0x32
#define CSHADOW 0x03
#define CTOP    0x27
#define CDLGF   0x21
#define CDLGI   0x24
#define CDLGTIT 0x25
#define CDLGBOT 0x25
#define CDLGT   0x24
#define CDLGB   0x60
#define CDLGTIP 0x24
*/
/*
#define CBACK   0x0B
#define CSHADOW 0x08
#define CTOP    0x1E
#define CDLGF   0x13
#define CDLGI   0x1E
#define CDLGTIT 0x1F
#define CDLGBOT 0x1F
#define CDLGT   0x1E
#define CDLGB   0x70
#define CDLGTIP 0x17
*/
#define CBACK   0x0B
#define CSHADOW 0x08
#define CTOP    0x1E
#define CDLGF   0x03
#define CDLGI   0x0E
#define CDLGTIT 0x0F
#define CDLGBOT 0x0F
#define CDLGT   0x0E
#define CDLGB   0x70
#define CDLGTIP 0x17

// ================================================
// Basic screen support.

typedef volatile byte TXS_TScr[25][80][2];

#define TXS_ScrMem   (*((TXS_TScr far *)0xB8000000UL))

PUBLIC int TXS_MinX,
           TXS_MaxX,
           TXS_MinY,
           TXS_MaxY;

PUBLIC void TXS_SetCursor(int x, int y);

PUBLIC void TXS_DrawChar(int x, int y, char c, int col);

PUBLIC void TXS_FillBox(int x1, int y1, int x2, int y2,
                        char c, int col);

PUBLIC void *TXS_SaveBox(int x1, int y1, int x2, int y2);

PUBLIC void TXS_RestoreBox(void *back);

PUBLIC void TXS_DrawBox(int x1, int y1, int x2, int y2,
                        const char *frame, int cf, int ci);

PUBLIC void TXS_DrawShadedBox(int x1, int y1, int x2, int y2,
                              const char *frame, int cf, int ci);

enum {
    TXS_STRJUST,
    TXS_STLJUST,
    TXS_STCENTER,
};

PUBLIC void TXS_DrawString(int x1, int x2, int y,
                           const char *s, int col, int style);

// ================================================
// Dialog and menu functions.

enum {
    TXS_MAXOPTIONS  = 24,
    TXS_DLGS_LEFT   = 0x0001,
    TXS_DLGS_NOSAVE = 0x0002,
};

PUBLIC int TXS_DoDialog(int x1, int y1, int *popt, int style,
                        const char *dlgt);

PUBLIC bool TXS_InputString(int x1, int x2, int y,
                            const char *caption,
                            char *s, int maxlen,
                            int col);

#endif

// ------------------------ TEXTSCR.H ----------------------
