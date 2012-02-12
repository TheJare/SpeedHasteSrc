// ------------------------ TEXTSCR.C ----------------------
// Text screen handy functions. Must get something done for the
// color support, thou...

#include "textscr.h"

#include <conio.h>
#include <llkey.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include <i86.h>

PUBLIC int TXS_MinX = 0,
           TXS_MaxX = 79,
           TXS_MinY = 0,
           TXS_MaxY = 24;

PUBLIC void TXS_SetCursor(int x, int y) {
    union REGS r;
    r.h.ah = 2;
    r.h.bh = 0;
    r.h.dh = y;
    r.h.dl = x;
    int386(0x10, &r, &r);
}

PUBLIC void TXS_DrawChar(int x, int y, char c, int col) {
    if (x >= TXS_MinX && x <= TXS_MaxX && y >= TXS_MinY && y <= TXS_MaxY) {
        if (c != '\xFF')
            TXS_ScrMem[y][x][0] = c;
        if (col > 0)
            TXS_ScrMem[y][x][1] = col;
    }
}

PUBLIC void TXS_FillBox(int x1, int y1, int x2, int y2,
                     char c, int col) {
    int i, j;
    for (i = y1; i <= y2; i++)
        for (j = x1; j <= x2; j++)
            TXS_DrawChar(j, i, c, col);
}

PUBLIC void *TXS_SaveBox(int x1, int y1, int x2, int y2) {
    int i, j;
    int *pi;
    byte *p;

    if (x1 < TXS_MinX) x1 = TXS_MinX;
    if (x2 > TXS_MaxX) x2 = TXS_MaxX;
    if (y1 < TXS_MinY) y1 = TXS_MinY;
    if (y2 > TXS_MaxY) y2 = TXS_MaxY;
    if (x1 > x2 || y1 > y2)
        return NULL;

    p = NEW(4*sizeof(int) + 2*(x2-x1+1)*(y2-y1+1));
    if (p == NULL)
        return NULL;
    pi = (void *)p;
    pi[0] = x1;
    pi[1] = x2;
    pi[2] = y1;
    pi[3] = y2;
    p = (void*)(pi+4);

    for (i = y1; i <= y2; i++)
        for (j = x1; j <= x2; j++) {
            *p++ = TXS_ScrMem[i][j][0];
            *p++ = TXS_ScrMem[i][j][1];
        }
    return pi;
}

PUBLIC void TXS_RestoreBox(void *back) {
    int i, j;
    byte *p;
    int *pi;
    int x1, x2, y1, y2;

//    VGA_VSync();

    if (back == NULL)
        return;
    pi = back;
    x1 = pi[0];
    x2 = pi[1];
    y1 = pi[2];
    y2 = pi[3];
    p = (void*)(pi+4);

    for (i = y1; i <= y2; i++)
        for (j = x1; j <= x2; j++) {
            TXS_ScrMem[i][j][0] = *p++;
            TXS_ScrMem[i][j][1] = *p++;
        }
    DISPOSE(back);
}

PUBLIC void TXS_DrawBox(int x1, int y1, int x2, int y2,
                        const char *frame, int cf, int ci) {
    int i;

    TXS_DrawChar(x1, y1, frame[0], cf);
    for (i = x1+1; i < x2; i++)
        TXS_DrawChar(i, y1, frame[1], cf);

    TXS_DrawChar(x2, y1, frame[2], cf);
    for (i = y1+1; i < y2; i++)
        TXS_DrawChar(x2, i, frame[3], cf);

    TXS_DrawChar(x2, y2, frame[4], cf);
    for (i = x2-1; i > x1; i--)
        TXS_DrawChar(i, y2, frame[5], cf);

    TXS_DrawChar(x1, y2, frame[6], cf);
    for (i = y2-1; i > y1; i--)
        TXS_DrawChar(x1, i, frame[7], cf);

    if (frame[8] != '\0')
        TXS_FillBox(x1+1, y1+1, x2-1, y2-1, frame[8], ci);
}

PUBLIC void TXS_DrawShadedBox(int x1, int y1, int x2, int y2,
                              const char *frame, int cf, int ci) {
    TXS_FillBox(x2+1, y1+1, x2+2, y2+1, '\xFF', CSHADOW);
    TXS_FillBox(x1+2, y2+1, x2+2, y2+1, '\xFF', CSHADOW);
    TXS_DrawBox(x1,   y1,   x2,   y2,   frame, cf, ci);
}

PUBLIC void TXS_DrawString(int x1, int x2, int y,
                           const char *s, int col, int style) {
    int l, x, i;

    if (s == NULL)
        s = "";

    x = x1;
    l = strlen(s);
    switch (style) {
        case TXS_STCENTER:
            if ((x2 - x1) > l)
                x = x1+((x2-x1+1)-l)/2;
            break;
        case TXS_STRJUST:
            if ((x2 - x1) > l)
                x = x2-l+1;
            break;
    }
    for (i = x1; i <= x2; i++) {
        if (*s != '\0' && i >= x) {
            TXS_DrawChar(i, y, *s, col);
            s++;
        } else
            TXS_DrawChar(i, y, ' ', col);
    }
}

// ================================================
// Dialog and menu functions.

PUBLIC int TXS_DoDialog(int x1, int y1, int *popt, int style,
                         const char *dlgt) {
    static char buf[4000];
    char *p, *g;
    char *tit = NULL, *bottom = NULL,
         *options[TXS_MAXOPTIONS], *helps[TXS_MAXOPTIONS];
    int noptions = 0;
    int i, y, opt;
    int w, x2, y2;
    void *back;
    int strst;

    if (style & TXS_DLGS_LEFT)
        strst = TXS_STLJUST;
    else
        strst = TXS_STCENTER;

    for (i = 0; i < TXS_MAXOPTIONS; i++) {
        options[i] = NULL;
        helps[i] = NULL;
    }
    strcpy(buf, dlgt);
    g = buf;
    p = strchr(g, '|');
    if (p != NULL) {
        tit = g;
        if (strlen(tit) + 4 > w)
            w = strlen(tit) + 4;
        *p = '\0';
        g = p + 1;
    }
    p = strchr(g, '|');
    if (p != NULL) {
        bottom = g;
        if (strlen(bottom) + 4 > w)
            w = strlen(bottom) + 4;
        *p = '\0';
        g = p + 1;
    }
    while (noptions < TXS_MAXOPTIONS && g != NULL && *g != '\0') {
        options[noptions] = g;
        p = strchr(g, '@');
        if (p != NULL) {
            *p = '\0';
            g = p + 1;
            helps[noptions] = g;
        }
        noptions++;
        p = strchr(g, '#');
        if (p != NULL) {
            *p = '\0';
            g = p + 1;
        } else
            break;
    }

    w = 0;
    if (tit != NULL && strlen(tit) > w)
        w = strlen(tit);
    if (bottom != NULL && strlen(bottom) > w)
        w = strlen(bottom);
    for (i = 0; i < noptions; i++)
        if (options[i] != NULL && strlen(options[i]) > w)
            w = strlen(options[i]);
    if (noptions <= 0)
        return -2;

    x2 = x1+w+4-1;
    y2 = y1 + noptions + 2 + 2*(tit != NULL) + 2*(bottom != NULL) - 1;

    if (style & TXS_DLGS_NOSAVE)
        back = NULL;
    else
        back = TXS_SaveBox(x1, y1, x2+2, y2+1);

    TXS_DrawShadedBox(x1,   y1,   x2,   y2, TXS_DoubleFrame, CDLGF, CDLGI);
    y = y1 + 1;
    if (tit != NULL) {
        TXS_DrawString(x1+1, x2-1, y, tit, CDLGTIT, TXS_STCENTER);
        y += 2;
        TXS_DrawBox(x1, y-1,   x2,   y2, TXS_DownFrame, CDLGF, CDLGI);
    }
    if (bottom != NULL) {
        TXS_DrawString(x1+1, x2-1, y2-1, bottom, CDLGBOT, TXS_STCENTER);
        TXS_DrawBox(x1, y2-2, x2,   y2, TXS_DownFrame, CDLGF, CDLGI);
    }

    if (popt != NULL)
        opt = (*popt) % noptions;
    else
        opt = 0;
//    LLK_LastScan = 0;
//    LLK_BIOSFlush();
    while (kbhit()) getch();
    do {
        byte key;
        for (i = 0; i < noptions; i++)
            if (i == opt)
                TXS_DrawString(x1+1, x2-1, y+i, options[i], CDLGB, strst);
            else
                TXS_DrawString(x1+1, x2-1, y+i, options[i], CDLGT, strst);
        {
            int my;
            my = TXS_MaxY;
            TXS_MaxY = 24;
            if (helps[opt] != NULL)
                TXS_DrawString(1, 79, 24, helps[opt], CDLGTIP, TXS_STLJUST);
            TXS_MaxY = my;
        }
        key = getch();
        switch (key) {
            case '\r':
            case '\n':
            case ' ':
                if (popt != NULL)
                    *popt = opt;
                TXS_RestoreBox(back);
                return opt;
            case '\033':
                if (popt != NULL)
                    *popt = opt;
                TXS_RestoreBox(back);
                return -1;
            case 0:
                key = getch();
                switch (key) {
                    case 0x48:
                        opt--; break;
                    case 0x50:
                        opt++; break;
                    case 0x47:
                    case 0x49:
                        opt = 0; break;
                    case 0x4F:
                    case 0x51:
                        opt = noptions - 1; break;
                }
                break;
            default:
                key = toupper(key);
                for (i = 0; i < noptions; i++)
                    if (options[i] != NULL && key == toupper(options[i][0])) {
                        if (popt != NULL)
                            *popt = i;
                        TXS_RestoreBox(back);
                        return i;
                    }
        }
        if (opt >= noptions)
            opt = 0;
        else if (opt < 0)
            opt = noptions - 1;
    } while (TRUE);
}

// ------------------------ TEXTSCR.C ----------------------

