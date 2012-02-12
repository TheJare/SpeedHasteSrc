
#include <vga.h>
#include <conio.h>
#include <llkey.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "config.h"

#include <textscr.h>

//#define DoubleFrame "ÖÄ·º½ÄÓº"
//#define DownFrame   "ÇÄ¶º½ÄÓº"

#define DoubleFrame "ÉÍ»º¼ÍÈº"
#define DownFrame   "ÇÄ¶º¼ÍÈº"

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


// ================================================
// Basic screen support.

typedef byte TScr[25][80][2];

#define ScrMem   (*((TScr*)0xB8000))

PRIVATE int MinX = 0,
            MaxX = 79,
            MinY = 0,
            MaxY = 24;

PRIVATE void DrawChar(int x, int y, char c, int col) {
    if (x >= MinX && x <= MaxX && y >= MinY && y <= MaxY) {
        if (c != '\xFF')
            ScrMem[y][x][0] = c;
        if (col > 0)
            ScrMem[y][x][1] = col;
    }
}

PRIVATE void FillBox(int x1, int y1, int x2, int y2,
                     char c, int col) {
    int i, j;
    for (i = y1; i <= y2; i++)
        for (j = x1; j <= x2; j++)
            DrawChar(j, i, c, col);
}

PRIVATE void *SaveBox(int x1, int y1, int x2, int y2) {
    int i, j;
    int *pi;
    byte *p;

    if (x1 < MinX) x1 = MinX;
    if (x2 > MaxX) x2 = MaxX;
    if (x1 < MinX) y1 = MinY;
    if (x2 > MaxY) y2 = MaxY;
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
            *p++ = ScrMem[i][j][0];
            *p++ = ScrMem[i][j][1];
        }
    return pi;
}

PRIVATE void RestoreBox(void *back) {
    int i, j;
    byte *p;
    int *pi;
    int x1, x2, y1, y2;

    VGA_VSync();

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
            ScrMem[i][j][0] = *p++;
            ScrMem[i][j][1] = *p++;
        }
    DISPOSE(back);
}

PRIVATE void DrawBox(int x1, int y1, int x2, int y2,
                     const char *frame, int cf, int ci) {
    int i;

    DrawChar(x1, y1, frame[0], cf);
    for (i = x1+1; i < x2; i++)
        DrawChar(i, y1, frame[1], cf);

    DrawChar(x2, y1, frame[2], cf);
    for (i = y1+1; i < y2; i++)
        DrawChar(x2, i, frame[3], cf);

    DrawChar(x2, y2, frame[4], cf);
    for (i = x2-1; i > x1; i--)
        DrawChar(i, y2, frame[5], cf);

    DrawChar(x1, y2, frame[6], cf);
    for (i = y2-1; i > y1; i--)
        DrawChar(x1, i, frame[7], cf);

    if (frame[8] != '\0')
        FillBox(x1+1, y1+1, x2-1, y2-1, frame[8], ci);
}

PRIVATE void DrawShadedBox(int x1, int y1, int x2, int y2,
                           const char *frame, int cf, int ci) {
    FillBox(x2+1, y1+1, x2+2, y2+1, '\xFF', CSHADOW);
    FillBox(x1+2, y2+1, x2+2, y2+1, '\xFF', CSHADOW);
    DrawBox(x1,   y1,   x2,   y2,   frame, cf, ci);
}

enum {
    STRJUST,
    STLJUST,
    STCENTER,
};

PRIVATE void DrawString(int x1, int x2, int y,
                        const char *s, int col, int style) {
    int l, x, i;

    if (s == NULL)
        s = "";

    x = x1;
    l = strlen(s);
    switch (style) {
        case STCENTER:
            if ((x2 - x1) > l)
                x = x1+((x2-x1+1)-l)/2;
            break;
        case STRJUST:
            if ((x2 - x1) > l)
                x = x2-l+1;
            break;
    }
    for (i = x1; i <= x2; i++) {
        if (*s != '\0' && i >= x) {
            DrawChar(i, y, *s, col);
            s++;
        } else
            DrawChar(i, y, ' ', col);
    }
}

PRIVATE bool InputString(int x1, int x2, int y,
                        char *s, int maxlen,
                        int col) {
    int l;
    bool leave = FALSE, ret = TRUE;
    const char *keyn;
    char buf[200];
    void *back;

    back = SaveBox(x1, y, x2+2, y+3+1);
    DrawShadedBox(x1,   y,   x2,   y+3,   DoubleFrame, CDLGF, CDLGI);
    FillBox(x1+1,   y+1,   x2-1,   y+3-1,  ' ', CDLGI);
    DrawString(x1+1, x2-1, y+1, "Enter string:", CDLGTIT, STCENTER);
    strncpy(buf, s, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    l = strlen(buf);
    while (!leave) {
        byte c;
        DrawString(x1+2, x1+2+l-1, y+2, buf, col, STRJUST);
        FillBox(x1+2+l, y+2, x2-2, y+2, '°', col);
        TXS_SetCursor(x1+2+l, y+2);

        c = getch();
        switch (c) {
            case '\r':
            case '\n':
                strncpy(s, buf, maxlen);
                s[maxlen-1] = '\0';
                ret = TRUE; leave = TRUE; break;
            case '\033':
                ret = FALSE; leave = TRUE; break;
            case 8:
                if (l > 0) buf[--l] = '\0';
                break;
            case 0:
                getch();
                break;
            default:
                if (c < 32)
                    break;
                if (l >= maxlen-1)
                    break;
                buf[l++] = c;
                buf[l] = '\0';
                break;
        }
    }
    TXS_SetCursor(0, 0);
    RestoreBox(back);
    return ret;
}


// ================================================
// Dialog and menu functions.

enum {
    MAXOPTIONS  = 24,
    DLGS_LEFT = 0x0001,
    DLGS_NOSAVE = 0x0002,
};

PRIVATE int DoDialog(int x1, int y1, int *popt, int style,
                     const char *dlgt) {
    static char buf[4000];
    char *p, *g;
    char *tit = NULL, *bottom = NULL,
         *options[MAXOPTIONS], *helps[MAXOPTIONS];
    int noptions = 0;
    int i, y, opt;
    int w, x2, y2;
    void *back;
    int strst;

    if (style & DLGS_LEFT)
        strst = STLJUST;
    else
        strst = STCENTER;

    for (i = 0; i < MAXOPTIONS; i++) {
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
    while (noptions < MAXOPTIONS && g != NULL && *g != '\0') {
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

    if (style & DLGS_NOSAVE)
        back = NULL;
    else
        back = SaveBox(x1, y1, x2+2, y2+1);

    DrawShadedBox(x1,   y1,   x2,   y2,   DoubleFrame, CDLGF, CDLGI);
    y = y1 + 1;
    if (tit != NULL) {
        DrawString(x1+1, x2-1, y, tit, CDLGTIT, STCENTER);
        y += 2;
        DrawBox(x1, y-1,   x2,   y2,   DownFrame, CDLGF, CDLGI);
    }
    if (bottom != NULL) {
        DrawString(x1+1, x2-1, y2-1, bottom, CDLGBOT, STCENTER);
        DrawBox(x1, y2-2, x2,   y2, DownFrame, CDLGF, CDLGI);
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
                DrawString(x1+1, x2-1, y+i, options[i], CDLGB, strst);
            else
                DrawString(x1+1, x2-1, y+i, options[i], CDLGT, strst);
        MaxY = 24;
        if (helps[opt] != NULL)
            DrawString(1, 79, 24, helps[opt], CDLGTIP, STLJUST);
        MaxY = 23;
        key = getch();
        switch (key) {
            case '\r':
            case '\n':
            case ' ':
                if (popt != NULL)
                    *popt = opt;
                RestoreBox(back);
                return opt;
            case '\033':
                if (popt != NULL)
                    *popt = opt;
                RestoreBox(back);
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
                        RestoreBox(back);
                        return i;
                    }
        }
/*
        if (LLK_LastScan != 0) {
            switch (LLK_LastScan) {
                case kENTER:
                case kKEYPADENTER:
                    if (popt != NULL)
                        *popt = opt;
                    RestoreBox(back);
                    return opt;
                case kESC:
                    if (popt != NULL)
                        *popt = opt;
                    RestoreBox(back);
                    return -1;
                case kUARROW:
                    opt--; break;
                case kDARROW:
                    opt++; break;
                case kHOME:
                case kPGUP:
                    opt = 0; break;
                case kEND:
                case kPGDN:
                    opt = noptions - 1; break;
                default:
                    if (kbhit()) {
                        int c;
                        if ( (c = getch()) == 0)
                            getch();
                        else {
                            c = toupper(c);
                            for (i = 0; i < noptions; i++)
                                if (options[i] != NULL && c == toupper(options[i][0])) {
                                    if (popt != NULL)
                                        *popt = i;
                                    RestoreBox(back);
                                    return i;
                                }
                        }
                    }
            }
            LLK_LastScan = 0;
            LLK_BIOSFlush();
        }
*/
        if (opt >= noptions)
            opt = 0;
        else if (opt < 0)
            opt = noptions - 1;
    } while (TRUE);
}

// ================================================

PRIVATE void DoFadeOut(void) {
    byte pz[768];
    int i, j;
    LLK_LastScan = 0;
    VGA_GetPalette(pz, 0, 256);
    for (i = 0; i < 64; i += 3) {
        for (j = 0; j < 3; j++)
            VGA_FadeOutPalette(pz, pz, 256, 0, 0, 0);
        VGA_VSync();
        VGA_DumpPalette(pz, 0, 256);
        if (LLK_LastScan != 0)
            break;
    }
    for (i = 0; i < 768; i+=3) {
        pz[i+0] = 0;
        pz[i+1] = 0;
        pz[i+2] = 0;
    }
    VGA_DumpPalette(pz, 0, 256);
}



// ================================================
// ================================================
// ================================================

PRIVATE bool DetectBlaster(int *port, int *irq, int *dma, int *hdma) {
    const char *p;
    if ( (p = getenv("BLASTER")) == NULL)
        return FALSE;

    while (*p != '\0') {
        if (isalpha(*p)) {
            switch(toupper(*p)) {
                case 'A': if (port != NULL) sscanf(p+1, "%x", port); break;
                case 'I': if (irq != NULL)  sscanf(p+1, "%d", irq); break;
                case 'D': if (dma != NULL)  sscanf(p+1, "%d", dma); break;
                case 'H': if (hdma != NULL)  sscanf(p+1, "%d", hdma); break;
            }
            while (*p != '\0' && *p != ' ')
                p++;
        } else
            p++;
    }
    return TRUE;
}

PRIVATE bool DetectGUS(int *port, int *irq, int *dma) {
    const char *p;
    if ( (p = getenv("ULTRASND")) == NULL)
        return FALSE;

    if (port != NULL)
        sscanf(p, "%x", port);
    while (*p != '\0' && *p != ',') p++;
    if (*p == '\0') return FALSE;
    p++;    // Skip comma.
    if (dma!= NULL)
        sscanf(p, "%d", dma);
    while (*p != '\0' && *p != ',') p++;
    if (*p == '\0') return FALSE;
    p++;    // Skip comma.
    while (*p != '\0' && *p != ',') p++;
    if (*p == '\0') return FALSE;
    p++;    // Skip comma.
    if (irq != NULL)
        sscanf(p, "%d", irq);
    return TRUE;
}

// ================================================

PRIVATE bool Changed = FALSE;

PRIVATE void ShowConfig(void) {
    int y = 8;
    DrawShadedBox(42, 5, 75, 19, DoubleFrame " ", CDLGF, CDLGI);
    DrawBox(42, 7, 75, 19, DownFrame, CDLGF, CDLGI);
    DrawString(43, 74, 6, "Current Settings", CDLGTIT, STCENTER);
    DrawString(43, 74, y++, " Soundcard:", CDLGT, STLJUST);
    DrawString(48, 74, y++, CFG_FindCard(CFG_Config.MusicDevice), CDLGTIT, STLJUST);
    if (CFG_Config.MusicDevice[0] != '\0') {
        char buf[200];

        sprintf(buf, "PORT %Xh, IRQ %d, DMA %d",
                CFG_Config.MusicPort, CFG_Config.MusicIRQ, CFG_Config.MusicDMA);
        DrawString(48, 74, y++, buf, CDLGTIT, STLJUST);
        sprintf(buf, "Volume: %d",
                CFG_Config.MusicVolume);
        DrawString(48, 74, y++, buf, CDLGTIT, STLJUST);
    }
/*
    DrawString(43, 74, y++, " Effects Soundcard:", CDLGT, STLJUST);
    DrawString(48, 74, y++, CFG_FindCard(CFG_Config.FXDevice), CDLGTIT, STLJUST);
    if (CFG_Config.FXDevice[0] != '\0') {
        char buf[200];

        sprintf(buf, "PORT %Xh, IRQ %d, DMA %d",
                CFG_Config.FXPort, CFG_Config.FXIRQ, CFG_Config.FXDMA);
        DrawString(48, 74, y++, buf, CDLGTIT, STLJUST);
        sprintf(buf, "Volume: %d",
                CFG_Config.FXVolume);
        DrawString(48, 74, y++, buf, CDLGTIT, STLJUST);
    }
    DrawString(43, 74,y++, " Controls", CDLGT, STLJUST);
*/
}

// ================================================

PRIVATE void DoSoundCard(int *scopt) {
    int i, opt;
    static char buf[4000];
    int port = -1, irq = -1, dma = -1, hdma = -1;
    int speed, musvol, fxvol;
    int sopt = 0;
    void *back;
    bool parmchg;

    sprintf(buf,
        "Soundcard setup|"
        "Select card|"
    );
    for (i = 0; i < CFG_NCARDS; i++) {
        strcat(buf, CFG_Soundcards[i][1]);
        strcat(buf, "@");
        strcat(buf, CFG_Soundcards[i][1]);
        strcat(buf, "#");
    }
    strcat(buf, "Change card parameters@Change port address, IRQ and DMA#");
    opt = DoDialog(13, 3, scopt, 0,
                   buf);
    if (opt == -1)
        return;
    if (opt < CFG_NCARDS) {
        Changed = TRUE;
        strcpy(CFG_Config.MusicDevice, CFG_Soundcards[opt][0]);
        strcpy(CFG_Config.FXDevice, CFG_Config.MusicDevice);
        if (strnicmp(CFG_Soundcards[opt][1], "Gravis", 6) == 0) {
            if (DetectGUS(&port, &irq, &dma)) {
                CFG_Config.MusicPort = CFG_Config.FXPort = port;
                CFG_Config.MusicIRQ = CFG_Config.FXIRQ = irq;
                CFG_Config.MusicDMA = CFG_Config.FXDMA = dma;
                return;
            }
            CFG_Config.MusicPort = CFG_Config.FXPort = 0x220;
            CFG_Config.MusicIRQ = CFG_Config.FXIRQ = 11;
            CFG_Config.MusicDMA = CFG_Config.FXDMA = 1;
        } else if (strnicmp(CFG_Soundcards[opt][1], "Soundblaster", 12) == 0) {
            if (DetectBlaster(&port, &irq, &dma, &hdma)) {
                CFG_Config.MusicPort = CFG_Config.FXPort = port;
                CFG_Config.MusicIRQ = CFG_Config.FXIRQ = irq;
                if (opt >= 6)
                    CFG_Config.MusicDMA = CFG_Config.FXDMA = hdma;
                else
                    CFG_Config.MusicDMA = CFG_Config.FXDMA = dma;
                return;
            }
            CFG_Config.MusicPort = CFG_Config.FXPort = 0x220;
            CFG_Config.MusicIRQ = CFG_Config.FXIRQ = 5;
            if (opt >= 7)
                CFG_Config.MusicDMA = CFG_Config.FXDMA = 5;
            else
                CFG_Config.MusicDMA = CFG_Config.FXDMA = 1;
        } else
            return;
    }
    if (CFG_Config.MusicDevice[0] == '\0')
        return;
    port = CFG_Config.MusicPort;
    irq = CFG_Config.MusicIRQ;
    dma = CFG_Config.MusicDMA;
    musvol = CFG_Config.MusicVolume;
    fxvol = CFG_Config.FXVolume;
    speed = CFG_Config.MusicRate;
    parmchg = FALSE;
    back = SaveBox(0, 0, 79, 24);
    do {
        if (strnicmp(CFG_FindCard(CFG_Config.MusicDevice), "Soundblaster", 12) == 0) {
            sprintf(buf,
                "Change card parameters|"
                "Set as installed on your card|"
                "Done@Accept and return to main menu#"
                "Forget changes@Do not keep changes and return to main menu#"
                "Increase port address (current: %Xh)@Increase port address (current: %Xh)#"
                "Decrease port address@Decrease port address (current: %Xh)#"
                "Increase IRQ number (current: %d)@Increase IRQ number (current: %d)#"
                "Decrease IRQ number@Decrease IRQ number (current: %d)#"
                "Increase DMA channel (current: %d)@Increase DMA channel (current: %d)#"
                "Decrease DMA channel@Decrease DMA channel (current: %d)#"
                "Increase music volume (current: %d)@Increase the volume of the soundtracks (current: %d)#"
                "Decrease music volume@Decrease the volume of the soundtracks (current: %d)#"
                "Increase effects volume (current: %d)@Increase the volume of the sound special effects (current: %d)#"
                "Decrease effects volume@Decrease the volume of the sound special effects (current: %d)#"
                "Increase mixing speed (current: %d)@Increase the mixing speed and, therefore, the sound quality (current: %d)#"
                "Decrease mixing speed@Decrease the mixing speed and, therefore, the sound quality (current: %d)#"
                , port, port, port, irq, irq, irq, dma, dma, dma,
                musvol, musvol, musvol, fxvol, fxvol, fxvol,
                speed, speed, speed
                );
            } else {
            sprintf(buf,
                "Change card parameters|"
                "Set as installed on your card|"
                "Done@Accept and return to main menu#"
                "Forget changes@Do not keep changes and return to main menu#"
                "Increase port address (current: %Xh)@Increase port address (current: %Xh)#"
                "Decrease port address@Decrease port address (current: %Xh)#"
                "Increase IRQ number (current: %d)@Increase IRQ number (current: %d)#"
                "Decrease IRQ number@Decrease IRQ number (current: %d)#"
                "Increase DMA channel (current: %d)@Increase DMA channel (current: %d)#"
                "Decrease DMA channel@Decrease DMA channel (current: %d)#"
                "Increase music volume (current: %d)@Increase the volume of the soundtracks (current: %d)#"
                "Decrease music volume@Decrease the volume of the soundtracks (current: %d)#"
                "Increase effects volume (current: %d)@Increase the volume of the sound special effects (current: %d)#"
                "Decrease effects volume@Decrease the volume of the sound special effects (current: %d)#"
                , port, port, port, irq, irq, irq, dma, dma, dma,
                musvol, musvol, musvol, fxvol, fxvol, fxvol
                );
            }
        opt = DoDialog(7, 3, &sopt, DLGS_NOSAVE | DLGS_LEFT,
                       buf);
        if (opt == -1 || opt == 1) {
            if (!parmchg || DoDialog(28, 8, NULL, 0,
                "Lose changes to settings|"
                "Are you sure?|"
                "Yes@Quit to main menu without keeping changes values#"
                "No@Stay in this menu#"
                ) == 0)
                break;
        } else if (opt == 0) {
            CFG_Config.MusicPort = CFG_Config.FXPort = port;
            CFG_Config.MusicIRQ = CFG_Config.FXIRQ = irq;
            CFG_Config.MusicDMA = CFG_Config.FXDMA = dma;
            CFG_Config.MusicRate = CFG_Config.FXRate = speed;
            CFG_Config.MusicVolume = musvol;
            CFG_Config.FXVolume = fxvol;
            Changed = TRUE;
            break;
        } else {
            parmchg = TRUE;
            if (opt == 2)
              port += 0x10;
            else if (opt == 3)
                port -= 0x10;
            else if (opt == 4)
                irq += 1;
            else if (opt == 5)
                irq -= 1;
            else if (opt == 6)
                dma += 1;
            else if (opt == 7)
                dma -= 1;
            else if (opt == 8)
                musvol += 1;
            else if (opt == 9)
                musvol -= 1;
            else if (opt == 10)
                fxvol += 1;
            else if (opt == 11)
                fxvol -= 1;
            else if (opt == 12)
                speed += 1000;
            else if (opt == 13)
                speed -= 1000;
            if (port < 0x200)
                port = 0x200;
            if (port > 0x300)
                port = 0x300;
            if (speed < 8000)
                speed = 8000;
            if (speed > 44000)
                speed = 44000;
            if (musvol < 0)
                musvol = 0;
            if (musvol > 256)
                musvol = 256;
            if (fxvol < 0)
                fxvol = 0;
            if (fxvol > 256)
                fxvol = 256;
        }
    } while (TRUE);
    RestoreBox(back);
}

PRIVATE void DoCredits(void) {
    void *b;

    b = SaveBox(0, 0, 79, 24);

    DrawShadedBox(2, 1, 75, 22, DoubleFrame " ", CDLGF, CDLGI);
    DrawBox(2, 3, 75, 22, DownFrame, CDLGF, CDLGI);

    DrawString(3, 74, 2, "Speed Haste (preview) - The Authors", CDLGTIT, STCENTER);

    DrawString(3, 74, 4, " From IV Team:", CTOP, STLJUST);
    DrawString(3, 74, 5, "   Javier Ar‚valo Baeza", CDLGT, STLJUST);
    DrawString(3, 74, 6, "      Original concept, design and game engine programming", CDLGTIT, STLJUST);
    DrawString(3, 74, 7, "   Juan Carlos Ar‚valo Baeza", CDLGT, STLJUST);
    DrawString(3, 74, 8, "      VTAL Sound System and additional programming", CDLGTIT, STLJUST);
    DrawString(3, 74, 9, " From Noriaworks Entertainment:", CTOP, STLJUST);
    DrawString(3, 74,10, "   Alejandro Luengo G¢mez", CDLGT, STLJUST);
    DrawString(3, 74,11, "      Additional programming and design", CDLGTIT, STLJUST);
    DrawString(3, 74,12, "   C‚sar Valencia Perell¢", CDLGT, STLJUST);
    DrawString(3, 74,13, "      Main design and graphics", CDLGTIT, STLJUST);
    DrawString(3, 74,14, "   Jorge Rosado de Alvaro", CDLGT, STLJUST);
    DrawString(3, 74,15, "      Game design, circuits", CDLGTIT, STLJUST);
    DrawString(3, 74,16, "   Jose Mar¡a San Antonio Alvarez", CDLGT, STLJUST);
    DrawString(3, 74,17, "      Additional graphics", CDLGTIT, STLJUST);
    DrawString(3, 74,18, "   Rafael S nchez Gago", CDLGT, STLJUST);
    DrawString(3, 74,19, "      3D modelling", CDLGTIT, STLJUST);
    DrawString(3, 74,20, "   Victor Segura Carrillo", CDLGT, STLJUST);
    DrawString(3, 74,21, "      Musical score", CDLGTIT, STLJUST);
/*
    DrawString(3, 74, 3, "Speed Haste (preview) - The Authors", CDLGTIT, STCENTER);
    DrawString(3, 74, 5, " Original concept and game engine programming:", CDLGT, STLJUST);
    DrawString(3, 74, 6, "   Javier Ar‚valo Baeza", CDLGTIT, STLJUST);
    DrawString(3, 74, 7, " Additional programming:", CDLGT, STLJUST);
    DrawString(3, 74, 8, "   Juan Carlos Ar‚valo Baeza", CDLGTIT, STLJUST);
    DrawString(3, 74, 9, "   Alejandro Luengo", CDLGTIT, STLJUST);
    DrawString(3, 74,10, " VTAL Sound system:", CDLGT, STLJUST);
    DrawString(3, 74,11, "   Juan Carlos Ar‚valo Baeza", CDLGTIT, STLJUST);
    DrawString(3, 74,12, " Game design:", CDLGT, STLJUST);
    DrawString(3, 74,13, "   C‚sar Valencia", CDLGTIT, STLJUST);
    DrawString(3, 74,14, "   Jorge Rosado", CDLGTIT, STLJUST);
    DrawString(3, 74,15, " Graphics:", CDLGT, STLJUST);
    DrawString(3, 74,16, "   C‚sar Valencia", CDLGTIT, STLJUST);
    DrawString(3, 74,17, "   Rafael .......", CDLGTIT, STLJUST);
    DrawString(3, 74,18, " Music:", CDLGT, STLJUST);
    DrawString(3, 74,19, "   Er Vistor", CDLGTIT, STLJUST);
*/
    while(LLK_kbhit());
    LLK_PressAnyKey();
    LLK_BIOSFlush();

    RestoreBox(b);
}

PRIVATE void DoOrder(void) {
    void *b;

    b = SaveBox(0, 0, 79, 24);

    DrawShadedBox(2, 2, 75, 21, DoubleFrame " ", CDLGF, CDLGI);
    DrawBox(2, 4, 75, 21, DownFrame, CDLGF, CDLGI);
    DrawString(3, 74, 3, "Speed Haste Preview", CDLGTIT, STCENTER);
    DrawString(3, 74,  5, "Can't order yet", 0x80|CTOP, STCENTER);
    DrawString(3, 74,  7, " Contact Noriaworks Entertainment at:", CDLGT, STLJUST);
    DrawString(9, 74,  8, " NoriaWorks Entertainment S.L.", CDLGTIT, STLJUST);
    DrawString(9, 74,  9, " C/ General Pardi¤as 34, 3§E", CDLGTIT, STLJUST);
    DrawString(9, 74, 10, " Madrid-28006 (SPAIN)", CDLGTIT, STLJUST);
    DrawString(9, 74, 11, " Phone: +32-1-431-57-55", CDLGTIT, STLJUST);
    DrawString(3, 74, 13, " Contact IV Team by the following means:", CDLGT, STLJUST);
    DrawString(9, 74, 14, " Internet: jarevalo@dit.upm.es (Juan Carlos Ar‚valo)", CDLGTIT, STLJUST);
    DrawString(9, 74, 15, "           jare@iguana.dit.upm.es (Javier Ar‚valo Baeza)", CDLGTIT, STLJUST);
    DrawString(9, 74, 16, " Snail-mail: Juan Carlos Ar‚valo", CDLGTIT, STLJUST);
    DrawString(9, 74, 17, "             C/Clara del Rey, 79; 3.C", CDLGTIT, STLJUST);
    DrawString(9, 74, 18, "             28002-Madrid (SPAIN)", CDLGTIT, STLJUST);
    DrawString(9, 74, 19, " Phone: +34-1-415-84-99 and ask for Javier or Juan Carlos", CDLGTIT, STLJUST);
    while(LLK_kbhit());
    LLK_PressAnyKey();
    LLK_BIOSFlush();

    RestoreBox(b);
}

PRIVATE void DoControls(void) {
    void *b;

    b = SaveBox(0, 0, 79, 24);

    DrawShadedBox(2, 2, 75, 21, DoubleFrame " ", CDLGF, CDLGI);
    DrawBox(2, 4, 75, 21, DownFrame, CDLGF, CDLGI);
    DrawString(3, 74, 3, "Speed Haste preview - controls", CDLGTIT, STCENTER);
    DrawString(5, 74,  7, "ARROWS for movement, SPACE to change gear.",             CDLGT, STLJUST);
    DrawString(5, 74,  8, "S to toggle two-view mode.",                             CDLGT, STLJUST);
    DrawString(5, 74,  9, "T for turning color trace on/off.",                      CDLGT, STLJUST);
    DrawString(5, 74, 10, "D for changing detail level and robot cars on/off.",     CDLGT, STLJUST);
    DrawString(5, 74, 11, "N/P to cycle thru the robot cars in the bottom window.", CDLGT, STLJUST);
    DrawString(5, 74, 12, "INS/DEL   change camera focus.",                         CDLGT, STLJUST);
    DrawString(5, 74, 13, "HOME/END  change horizon line.",                         CDLGT, STLJUST);
    DrawString(5, 74, 14, "PGUP/PGDN change height.",                               CDLGT, STLJUST);
    DrawString(5, 74, 15, "Q/A       change camera distance.",                      CDLGT, STLJUST);
    DrawString(5, 74, 16, "Gray +/-  change map zoom.",                             CDLGT, STLJUST);
    DrawString(5, 74, 17, "F1-F4 and F7-F10 select different standard cameras.",    CDLGT, STLJUST);
    DrawString(5, 74, 18, "\",\" and \".\" to rotate around camera target.",        CDLGT, STLJUST);

    while(LLK_kbhit());
    LLK_PressAnyKey();
    LLK_BIOSFlush();

    RestoreBox(b);
}

PRIVATE void DoModem(void) {
    static int opt = 0;
    for (;;) {
        opt = DoDialog( 2,  2, &opt, DLGS_NOSAVE,
            "Modem Setup|"
            "Select a string|"
            "Modem Init@Change modem initialization string#"
            "Dial Number@Change modem dialing string#"
            "Modem Hangup@Change modem hangup string#"
            "Exit@Back to main menu#"
            );
        if (opt == -1 || opt == 3)
            break;
        else if (opt == 0) {
            Changed |= InputString(10, 70, 12, CFG_Config.modeminit, CFG_STRSIZE, 2);
        } else if (opt == 1) {
            Changed |= InputString(10, 70, 12, CFG_Config.modemdial, CFG_STRSIZE, 2);
        } else if (opt == 2) {
            Changed |= InputString(10, 70, 12, CFG_Config.modemhang, CFG_STRSIZE, 2);
        }
    }
}

// ================================================

extern void FinishProgram(void) {
    VGA_SetMode(3);
    printf("Circuit Racer setup (C) Copyright 1995-1997 by Javier Arevalo.\n"
//           "All rights reserved. DO NOT DISTRIBUTE.\n"
           );
    LLK_End();
    LLK_BIOSFlush();
}

void main() {
    int mmopt = 0, scopt = 0, ccopt = 0;
    LLK_Init();
    LLK_ChainChange = FALSE;
    LLK_DoChain = TRUE;

    if (!CFG_Load("speed.cfg")) {
        int port = -1, irq = -1, dma = -1, hdma = -1;
        if (DetectGUS(&port, &irq, &dma)) {
            strcpy(CFG_Config.MusicDevice, CFG_FindCard("Gravis Ultrasound"));
            strcpy(CFG_Config.FXDevice, CFG_Config.MusicDevice);
            CFG_Config.MusicPort = CFG_Config.FXPort = port;
            CFG_Config.MusicIRQ = CFG_Config.FXIRQ = irq;
            CFG_Config.MusicDMA = CFG_Config.FXDMA = dma;
        } else {
            port = -1, irq = -1, dma = -1;
            if (DetectBlaster(&port, &irq, &dma, &hdma)) {
                if (hdma > 0) {
                    strcpy(CFG_Config.MusicDevice, CFG_FindCard("SoundBlaster 16"));
                    dma = hdma;
/*
                } else if (irq > 7 || dma != 1) {
                    strcpy(CFG_Config.MusicDevice, CFG_FindCard("SoundBlaster Pro"));
*/
                } else
                    strcpy(CFG_Config.MusicDevice, CFG_FindCard("SoundBlaster mono"));
                strcpy(CFG_Config.FXDevice, CFG_Config.MusicDevice);
                CFG_Config.MusicPort = CFG_Config.FXPort = port;
                CFG_Config.MusicIRQ = CFG_Config.FXIRQ = irq;
                CFG_Config.MusicDMA = CFG_Config.FXDMA = dma;
            }
        }
        Changed = TRUE;
    }

    DoFadeOut();

    VGA_SetMode(3);
    VGA_Set16c();
    {
        static byte pal[] = {
             0,  0,  0,
            198/4, 198/4,105/4,
            85/4, 128/4, 85/4,
            0/4, 61/4, 0/4,
            255/4, 255/4, 0/4,
            255/4, 255/4, 169/4,
            0/4, 255/4, 147/4,
            223/4, 223/4, 223/4,
        };

        VGA_DumpPalette(pal, 0, SIZEARRAY(pal));
    }

    FillBox( 0, 24, 79, 24, ' ', 0);
    FillBox(0, 0, 79, 0, ' ', 0x22);
    DrawString( 1, 39, 0, "Circuit Racer setup program", CTOP, STLJUST);
    DrawString(40, 78, 0, "(C) Copyright 1995/97 by J.Arevalo", CTOP, STRJUST);
    FillBox(0, 24, 79, 24, ' ', CDLGTIP);

    MinY = 1;
    MaxY = 23;

    FillBox( 0,  0, 79, 23, '°', CBACK);
    do {
        int opt;
        VGA_VSync();

        ShowConfig();
        opt = DoDialog( 2,  2, &mmopt, DLGS_NOSAVE,
            "Main Menu|"
            "Select an option|"
            "Sound setup@Change sound card or card settings#"
            "Modem strings@Configure modem strings#"
//            "Modify controls@Select keyboard/mouse/joystick#"
//            "How to order@Ordering info, prices, etc.#"
//            "Credits@Who made what in this mess#"
            "Accept settings and exit@Save current settings and return to MS-DOS#"
            "Quit without saving@Do not keep the changed settings#"
            );
        if (opt == -1 || opt == 3) {
            if (!Changed || DoDialog(28, 8, NULL, 0,
                "Quit without saving|"
                "Are you sure?|"
                "Yes@Quit without saving#"
                "No@Return to main menu#"
                ) == 0)
                break;
        } else if (opt == 1) {
            DoModem();
        } else if (opt == 2) {
            CFG_Save("speed.cfg");
            break;
/*
        } else if (opt == 1) {
            DoCredits();
        } else if (opt == 2) {
            DoOrder();
        } else if (opt == 1) {
            DoControls();
*/
        } else if (opt == 0) {
            DoSoundCard(&scopt);
        }
    } while (TRUE);

    DoFadeOut();
    FinishProgram();
    printf("\nType CR to play.");
}

