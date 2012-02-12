
#include "menus.h"

#include <llscreen.h>
#include <llkey.h>
#include <keynames.h>
#include <joystick.h>
#include <jclib.h>
#include <is2code.h>
#include <vertdraw.h>
//#include <timer.h>
#include <vbl.h>
#include <sincos.h>
#include <string.h>
#include <text.h>
#include <serial.h>
#include <gamevtal.h>

#include "graphics.h"

#include <object3d.h>

#include "config.h"
#include "globals.h"
#include "models.h"
#include "records.h"

#include "net.h"

#define NLOGOSPRS 12
PRIVATE IS2_PSprite LogoSprs[NLOGOSPRS];
PRIVATE int LogoPos[NLOGOSPRS][2] = {
    { 63, 31},
    { 83, 31},
    {104, 31},
    {141, 31},
    {174, 31},
    {194, 31},
    {219, 31},

    { 88, 60},
    {117, 65},
    {155, 60},
    {185, 60},
    {217, 60},
};
PRIVATE IS2_PSprite SprHelmet, SprCredits, SprTM;

byte *MENU_Back = NULL;
FONT_TFont MENU_FontRed, MENU_FontYellow, MENU_FontWhite;

PUBLIC bool MENU_Init(const char *fback, const byte *copy) {
    int i;
    const byte *b;

    b = MENU_Back;
    if (MENU_Back == NULL && (MENU_Back = NEW(LLS_Size)) == NULL)
        return FALSE;
    if (copy != NULL) {
        memcpy(MENU_Back, copy, LLS_Size);
    } else if (fback != NULL && JCLIB_Load(fback, MENU_Back, LLS_Size) == LLS_Size) {
        // Todo OK.
    } else {
        int j;
        memset(MENU_Back, 127, LLS_Size);
        for (j = 110, i = 126; i > 112; i -= 1, j += 2)
            memset(MENU_Back+LLS_SizeX*j, i, LLS_SizeX*2);
        for (; i <= 126; i += 1, j += 2)
            memset(MENU_Back+LLS_SizeX*j, i, LLS_SizeX*2);
    }
    if (b != NULL)      // If memory was already initialized.
        return TRUE;

    O3DM_OrderByZ = TRUE;

    FONT_Load(&MENU_FontRed, "mfontred.fnt");
    FONT_Load(&MENU_FontYellow, "mfontyel.fnt");
    FONT_Load(&MENU_FontWhite, "mfontwhi.fnt");

    REQUIRE( (SprHelmet  = IS2_Load("mmhelmet.is2")) != NULL);
    REQUIRE( (SprCredits = IS2_Load("mmcred.is2")) != NULL);
    REQUIRE( (SprTM      = IS2_Load("mmtm.is2")) != NULL);

        // Init font arrays.
    for (i = 0; i < SIZEARRAY(LogoSprs); i++)
        LogoSprs[i] = NULL;

    for (i = 0; i < SIZEARRAY(LogoSprs); i++) {
        char buf[30];
        sprintf(buf, "sphl%i.IS2", i);
        if ( (LogoSprs[i] = IS2_Load(buf)) == NULL) {
            MENU_End();
            return FALSE;
        }
    }
    return TRUE;
}

PUBLIC void MENU_End(void) {
    int i;
    if (MENU_Back == NULL)
        return;
    for (i = 0; i < SIZEARRAY(LogoSprs); i++)
        DISPOSE(LogoSprs[i]);
    DISPOSE(SprTM);
    DISPOSE(SprCredits);
    DISPOSE(SprHelmet);

    FONT_End(&MENU_FontWhite);
    FONT_End(&MENU_FontYellow);
    FONT_End(&MENU_FontRed);

    DISPOSE(MENU_Back);
}

// -------------------------------

PRIVATE void SetBackgroundScreen(void) {
    int i;

        // Set background.
    MENU_Init("mbg_prin.pix", NULL);
    RepMovsb(LLS_Screen[0], MENU_Back, LLS_Size);

    IS2_Draw(SprHelmet, 200, 20, SprHelmet->w, SprHelmet->h);
    IS2_Draw(SprCredits, 160, 200-2-SprCredits->h, SprCredits->w, SprCredits->h);
        // Draw logo
    for (i = 0; i < SIZEARRAY(LogoSprs); i++) {
        IS2_PSprite p;
        p = LogoSprs[i];
        IS2_Draw(p, LogoPos[i][0], LogoPos[i][1], p->w, p->h);
    }
    IS2_Draw(SprTM, 255, 45, SprTM->w, SprTM->h);
        // Set the composed screen as new background.
    MENU_Init(NULL, LLS_Screen[0]);
}

// -------------------------------

PUBLIC bool MENU_DoInput(const char *msg, char *buf, int maxlen, const char *charset) {
    int l;
    bool leave = FALSE, ret = TRUE;
    const char *keyn;

    LLK_LastScan = 0;
    l = strlen(buf);
    while (!leave) {
        int w, w1, w2;

        memcpy(LLS_Screen[0], MENU_Back, LLS_Size);
        TEXT_GetExtent(&GL_WFont, 0, 0, msg, &w1, NULL);
        TEXT_GetExtent(&GL_YFont, 0, 0, buf, &w2, NULL);
        if (w1 < w2) w = w2;
        else         w = w1;
        w += 3;
        GFX_Rectangle(159-w/2, 80, w, 40, 0, 8);
        GFX_Rectangle(159-w/2, 80, w-1, 1, 7, -1);
        GFX_Rectangle(159-w/2, 80, 1, 40-1, 7, -1);
        TEXT_Write(&GL_WFont, 160-w1/2, 85, msg, 15);
        TEXT_Write(&GL_YFont, 160-w2/2, 105, buf, 15);

        LLS_Update();

        while (LLK_LastScan == 0);
        GL_Capture(LLK_LastScan);
        switch (LLK_LastScan) {
            case kESC:
                leave = TRUE; ret = FALSE; break;
            case kENTER:
            case kKEYPADENTER:
                leave = TRUE; ret = TRUE; break;
            case kBACKSPACE:
                if (l > 0) buf[--l] = '\0';
                break;
            default:
                if (l >= maxlen-1)
                    break;
                keyn = KEYN_FindKey(LLK_LastScan);
                if (keyn == NULL || keyn[1] != '\0'
                 || (charset != NULL && strchr(charset, keyn[0]) == NULL))
                    break;
                buf[l++] = keyn[0];
                buf[l] = '\0';
                break;
        }
        LLK_LastScan = 0;
    }
    LLK_LastScan = 0;
    return ret;
}

PRIVATE int MainDemoMenu(void) {
    F3D_TCamera c;
    dword clk;
    int i;
//    IS2_PSprite letters[26];
//    static char *menu = "Press any key";
//    int mm, ms, mdx, mdy;
    int lm, ls, ldx, ldy;
    int cy, cv;
    int by, bv;

    c.data.radius = 0x60000000UL;
    c.data.h      = 120000;
    c.data.focus  = 0x1800;
    c.data.horizon = 75;
    c.data.flags   = 0;
    c.dx = 1 << (32-3);
    c.dy = 1 << (32-3);
    c.angle = 0;

    DRW_Tile = FALSE;
    DRW_TranslatePtr = GL_ClrTable + 256*15; //Map.trans + 256*15;
    DRW_SetClipZone(0, 0, LLS_SizeX, LLS_SizeY, NULL);
    lm = 0;
//    mm = -1;
    ls = 20*(1 << 8);
//    ms = 20*(1 << 8);
    cv = 0;
    bv = 0;
//    clk = TIMER_Clock;
    clk = 0;
    LLK_LastScan = 0;
    while (LLK_LastScan == 0) {
        int h, nf;
        c.x = c.dx - FPMult(c.data.radius, Cos(c.angle));
        c.y = c.dy - FPMult(c.data.radius, Sin(c.angle));
        if (cv < 0x4000)
            cy = (LLS_SizeY << 4) - FPMult((LLS_SizeY-80)<< 4, FPMult(Sin(cv), Sin(cv)));
        else
            cy = 80 << 4;
        if (bv < 0x4000)
            by = FPMult(LLS_SizeY, FPMult(Sin(bv), Sin(bv)));
        else
            by = LLS_SizeY;
        if ((cy >> 4) >= LLS_SizeY)
            h = LLS_SizeY;
        else
            h = (cy >> 4);

        MemSetD(LLS_Screen[0], 0x7F7F7F7F, LLS_SizeY*LLS_SizeX);
        if (by > 0)
            RepMovsb(LLS_Screen[0], MENU_Back+LLS_SizeX*(LLS_SizeY-by), by*LLS_SizeX);
        if (h < LLS_SizeY)
            F3D_Draw3DNoTile(LLS_Screen[h], Map.map, LLS_SizeX, LLS_SizeY - h, &c);

            // Draw logo
        ldx = 80;
        ldy = 20;
        for (i = 0; i < lm; i++) {
            IS2_PSprite p;
            p = LogoSprs[i];
            IS2_Draw(p, LogoPos[i][0], LogoPos[i][1], p->w, p->h);
            ldx += p->w - 3;
            if (i == 6) {
                ldy += p->h + 2;
                ldx = 90;
            }
        }

/*
            // Draw menu
        mdx = 40;
        mdy = 130;
        for (i = 0; i < mm; i++) {
            IS2_PSprite p;
            if (menu[i] != ' ') {
                p = letters[toupper(menu[i])-'A'];
                IS2_Draw(p, mdx, mdy, p->w, p->h);
            }
            mdx += 17;
        }
*/

        if (lm >= 0 && ls > 0 && lm < NLOGOSPRS) {
            sint32 w, h;
            IS2_PSprite p;

            p = LogoSprs[lm];
            w = FPnMult(ls+(1 << 8), p->w, 8);
            h = FPnMult(ls+(1 << 8), p->h, 8);
            IS2_Draw(p, LogoPos[lm][0], LogoPos[lm][1], w, h);
        }
/*
        if (mm >= 0 && ms > 0 && menu[mm] != '\0') {
            sint32 x, y, w, h;
            IS2_PSprite p;

            p = letters[toupper(menu[mm])-'A'];
            w = FPnMult(ms+(1 << 8), p->w, 8);
            h = FPnMult(ms+(1 << 8), p->h, 8);
            IS2_Draw(p, mdx, mdy, w, h);
        }
*/
        LLS_Update();
        nf = VBL_VSync(1);
        while (nf-- > 0) {
            c.angle += 100;
            if (lm > 2 && bv < 0x4000)
                bv += 100;
            if (lm > 5 && cv < 0x4000)
                cv += 100;
/*
            if (mm >= 0 && menu[mm] != '\0') {
                ms -= (1 << 9);
                if (ms < 0) {
                    do mm++; while (menu[mm] == ' ');
                    if (menu[mm] != '\0')
                        ms = 20*(1 << 8);
                }
            }
*/
            if (lm < NLOGOSPRS) {
                ls -= (1 << 9);
                if (ls < 0) {
                    lm++;
                    if (lm < NLOGOSPRS)
                        ls = 20*(1 << 8);
                    else
;//                        mm = 0;     // Launch the menu text.
                }
            }
            clk++;
        }
    }
    LLK_BIOSFlush();
    LLK_LastScan = 0;
    MENU_Return();
    return 1;
}

PUBLIC byte MENU_DoControlMenu(const char *menu[], int nopts, int *opt) {
    byte s;
    int i;

        // Copy background.
    RepMovsb(LLS_Screen[0], MENU_Back, LLS_Size);

    if (NET_ModemOn) {
//        TEXT_GetExtent(&MENU_FontRed, 0, 0, "ON LINE", &w, NULL);
        TEXT_Write(&MENU_FontRed, 0, 190, "ON LINE", 15);
    }

    for (i = 0; i < nopts; i++) {
        int w;
        int base;

        base = (nopts == 3)? 105:(nopts == 4)? 97:90;
        TEXT_GetExtent(&MENU_FontWhite, 0, 0, menu[i], &w, NULL);
        if (i == *opt) {
            TEXT_Write(&MENU_FontWhite, (320-w)/2, 12*i+base, menu[i],
                        11);
        } else {
            TEXT_Write(&MENU_FontYellow, (320-w)/2, 12*i+base, menu[i],
                       11);
        }
    }
    LLS_Update();
    s = LLK_LastScan;
    LLK_LastScan = 0;
    GL_Capture(s);
    switch (s) {
        case kUARROW:
        case kKEYPAD8:
            (*opt)--; if ((*opt) < 0) (*opt) = nopts - 1;
            break;
        case kDARROW:
        case kKEYPAD2:
            (*opt)++; if ((*opt) >= nopts) (*opt) = 0;
            break;
        case kPGUP:
            (*opt) -= 4; if ((*opt) < 0) (*opt) = 0;
            break;
        case kHOME:
            (*opt) = 0;
            break;
        case kPGDN:
            (*opt) += 4; if ((*opt) >= nopts) (*opt) = nopts - 1;
            break;
        case kEND:
            (*opt) = nopts - 1;
            break;
    }
    return s;
}

PRIVATE int SoundConfigMenu(void) {
    bool leave = FALSE;
    int ret = 0;
    static int opt = 0;     // So it stays from call to call.
    static char musopt[] = "Music volume:   000%",
                fxopt[]  = "Effects volume: 000%";
    static char *menu[] = {
        musopt,
        fxopt,
        "Jukebox",
        "Exit",
    };

    if (MENU_Back == NULL)
        SetBackgroundScreen();

    do {
        sprintf(musopt, "Music volume:   %d%%", (CFG_Config.MusicVolume*100)/511);
        sprintf(fxopt,  "Effects volume: %d%%", (CFG_Config.FXVolume*100)/511);
        switch (MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt)) {
            case kLARROW:
            case kKEYPAD4:
            case kKEYPADMINUS:
                if (opt == 0) {
                    if (CFG_Config.MusicVolume >= 10)
                        CFG_Config.MusicVolume -= 10;
                    else
                        CFG_Config.MusicVolume = 0;
                    GAME_MUS_ChangeVolume(SH_Vtal, CFG_Config.MusicVolume);
                } else if (opt == 1) {
                    if (CFG_Config.FXVolume >= 10)
                        CFG_Config.FXVolume -= 10;
                    else
                        CFG_Config.FXVolume = 0;
                    GAME_EFF_ChangeVolume(SH_Vtal, CFG_Config.FXVolume);
                }
                break;
            case kRARROW:
            case kKEYPAD6:
            case kKEYPADPLUS:
                if (opt == 0) {
                    if (CFG_Config.MusicVolume < 501)
                        CFG_Config.MusicVolume += 10;
                    else
                        CFG_Config.MusicVolume = 511;
                    GAME_MUS_ChangeVolume(SH_Vtal, CFG_Config.MusicVolume);
                } else if (opt == 1) {
                    if (CFG_Config.FXVolume < 501)
                        CFG_Config.FXVolume += 10;
                    else
                        CFG_Config.FXVolume = 511;
                    GAME_EFF_ChangeVolume(SH_Vtal, CFG_Config.FXVolume);
                }
                break;

            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                switch (opt) {
                    case 2:
                        MENU_Enter(MENU_JUKEBOX);
                        ret = -1; leave = TRUE; break;
                    case SIZEARRAY(menu)-1:
                        MENU_Return();
                        ret = -1; leave = TRUE; break;
                }
                break;
            case kESC:
                MENU_Return();
                ret = -1; leave = TRUE;
                break;
        }
    } while (!leave);
    CFG_Save("speed.cfg");
    return ret;
}

PRIVATE int DetailConfigMenu(void) {
    bool leave = FALSE;
    int ret = 0;
    static int opt = 0;     // So it stays from call to call.
    static char decopt[] = "Decoration density: 000%";
    static char bakopt[] = "Background Depth: 000%";
    static char *menu[] = {
        decopt,
        NULL,
        NULL,
        NULL,
        bakopt,
        NULL,
        "Exit",
    };

    if (MENU_Back == NULL)
        SetBackgroundScreen();

    do {
        if (PolygonDetail) menu[1] = "Shaded Polygons";
        else               menu[1] = "Flat Polygons";
        if (FloorDetail) menu[2] = "Detailed floor";
        else             menu[2] = "Half detail floor";
        if (WallsDetail) menu[3] = "Textured walls";
        else             menu[3] = "Flat walls";
/*
        if (BackgroundDetail) menu[4] = "Parallax Background";
        else                  menu[4] = "Fixed background";
*/
        sprintf(bakopt, "Background Depth: %d%%", BackgroundDetail*10);
        if (SVGAOn) menu[5] = "SVGA 640x480";
        else        menu[5] = "VGA 320x200";
        sprintf(decopt, "Decoration density: %d%%", (DecorationDetail*100)/16);
        switch (MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt)) {
            case kLARROW:
            case kKEYPAD4:
            case kKEYPADMINUS:
                if (opt == 0) {
                    if (DecorationDetail > 0) DecorationDetail--;
                    else                      DecorationDetail = 0;
                } else if (opt == 1) {
                    PolygonDetail = !PolygonDetail;
                } else if (opt == 2) {
                    FloorDetail = !FloorDetail;
                } else if (opt == 3) {
                    WallsDetail = !WallsDetail;
                } else if (opt == 4) {
                    if (BackgroundDetail > 0)
                        BackgroundDetail--;
                } else if (opt == 5) {
                    SVGAOn = !SVGAOn;
                }
                break;
            case kRARROW:
            case kKEYPAD6:
            case kKEYPADPLUS:
                if (opt == 0) {
                    if (DecorationDetail < 16) DecorationDetail++;
                    else                       DecorationDetail = 16;
                } else if (opt == 1) {
                    PolygonDetail = !PolygonDetail;
                } else if (opt == 2) {
                    FloorDetail = !FloorDetail;
                } else if (opt == 3) {
                    WallsDetail = !WallsDetail;
                } else if (opt == 4) {
                    if (BackgroundDetail < 10)
                        BackgroundDetail++;
                } else if (opt == 5) {
                    SVGAOn = !SVGAOn;
                }
                break;

            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                switch (opt) {
                    case 1:
                        PolygonDetail = !PolygonDetail; break;
                    case 2:
                        FloorDetail = !FloorDetail; break;
                    case 3:
                        WallsDetail = !WallsDetail; break;
                    case 4:
                        break;
                    case 5:
                        SVGAOn = !SVGAOn; break;
                    case SIZEARRAY(menu)-1:
                        MENU_Return();
                        ret = -1; leave = TRUE; break;
                }
                break;
            case kESC:
                MENU_Return();
                ret = -1; leave = TRUE;
                break;
        }
    } while (!leave);

    CFG_Config.DecorationDetail = DecorationDetail;
    CFG_Config.FloorDetail      = FloorDetail     ;
    CFG_Config.WallsDetail      = WallsDetail     ;
    CFG_Config.PolygonDetail    = PolygonDetail   ;
    CFG_Config.BackgroundDetail = BackgroundDetail;
    CFG_Config.SVGAOn           = SVGAOn;

    CFG_Save("speed.cfg");
    return ret;
}

// -----------------------

PRIVATE bool DefineKeysStep(int n, byte *key, const char *text[], int ntext, int o) {
    int i, w;

        // Copy background.
    RepMovsb(LLS_Screen[0], MENU_Back, LLS_Size);
    TEXT_GetExtent(&MENU_FontRed, 0, 0, "Define keys - Player 1", &w, NULL);
    TEXT_Printf(&MENU_FontRed, (320-w)/2, 90, 11, "Define keys - Player %d", n+1);
    TEXT_GetExtent(&MENU_FontRed, 0, 0, "ESC to Exit", &w, NULL);
    TEXT_Write(&MENU_FontRed, (320-w)/2, 160, "ESC to Exit", 11);

    for (i = 0; i < ntext/2; i++)
        text[i*2+1] = KEYN_FindKey(key[i]);

    for (i = 0; i < ntext; i+=2) {
        TEXT_GetExtent(&MENU_FontWhite, 0, 0, text[i+1], &w, NULL);
        if (i/2 == o) {
            TEXT_Write(&MENU_FontWhite,  40, 103+5*i, text[i], 11);
            TEXT_Write(&MENU_FontWhite, 280-w, 103+5*i, text[i+1], 11);
        } else {
            TEXT_Write(&MENU_FontYellow,  40, 103+5*i, text[i], 11);
            TEXT_Write(&MENU_FontYellow, 280-w, 103+5*i, text[i+1], 11);
        }
    }
    LLS_Update();
    LLK_LastScan = 0;
    while (LLK_LastScan == 0);

    if (LLK_LastScan == kESC)
        return FALSE;
    key[o] = LLK_LastScan;
    return TRUE;
}

PRIVATE bool DefineKeys(int n) {
    static const char *texts[] = {
        "Accelerate", NULL,
        "Brake",      NULL,
        "Turn left",  NULL,
        "Turn right", NULL,
        "Change Gear",NULL,
    };
    byte pk[5];

    if (n == 0) {
        pk[0] = CFG_Config.P1KeyUp;
        pk[1] = CFG_Config.P1KeyDn;
        pk[2] = CFG_Config.P1KeyLt;
        pk[3] = CFG_Config.P1KeyRt;
        pk[4] = CFG_Config.P1KeyGe;
    } else {
        pk[0] = CFG_Config.P2KeyUp;
        pk[1] = CFG_Config.P2KeyDn;
        pk[2] = CFG_Config.P2KeyLt;
        pk[3] = CFG_Config.P2KeyRt;
        pk[4] = CFG_Config.P2KeyGe;
    }
    LLK_Autorepeat = FALSE;
    if (DefineKeysStep(n, pk, texts, SIZEARRAY(texts), 0)
     && DefineKeysStep(n, pk, texts, SIZEARRAY(texts), 1)
     && DefineKeysStep(n, pk, texts, SIZEARRAY(texts), 2)
     && DefineKeysStep(n, pk, texts, SIZEARRAY(texts), 3)
     && DefineKeysStep(n, pk, texts, SIZEARRAY(texts), 4)) {
        if (n == 0) {
            CFG_Config.P1KeyUp = pk[0];
            CFG_Config.P1KeyDn = pk[1];
            CFG_Config.P1KeyLt = pk[2];
            CFG_Config.P1KeyRt = pk[3];
            CFG_Config.P1KeyGe = pk[4];
        } else {
            CFG_Config.P2KeyUp = pk[0];
            CFG_Config.P2KeyDn = pk[1];
            CFG_Config.P2KeyLt = pk[2];
            CFG_Config.P2KeyRt = pk[3];
            CFG_Config.P2KeyGe = pk[4];
        }
        LLK_Autorepeat = TRUE;
        LLK_LastScan = 0;
        return TRUE;
    }
    LLK_Autorepeat = TRUE;
    LLK_LastScan = 0;
    return FALSE;
}


PRIVATE bool CalibrateJoystickStep(int n, word *jx, word *jy, const char *text) {
    int x, y, xval[8], yval[8], nval;
    int i, w;
    uint b;
    int f, nf;

        // Copy background.
    RepMovsb(LLS_Screen[0], MENU_Back, LLS_Size);
    TEXT_GetExtent(&MENU_FontRed, 0, 0, "Calibrate Joystick A", &w, NULL);
    TEXT_Printf(&MENU_FontRed, (320-w)/2, 90, 11, "Calibrate Joystick %c", 'A'+n);
    TEXT_GetExtent(&MENU_FontRed, 0, 0, "Press Button to Continue", &w, NULL);
    TEXT_Write(&MENU_FontRed, (320-w)/2, 150, "Press Button to Continue", 11);
    TEXT_GetExtent(&MENU_FontRed, 0, 0, "ESC to Exit", &w, NULL);
    TEXT_Write(&MENU_FontRed, (320-w)/2, 160, "ESC to Exit", 11);

    TEXT_GetExtent(&MENU_FontWhite, 0, 0, text, &w, NULL);
    TEXT_Write(&MENU_FontYellow, (320-w)/2, 120, text, 11);
    LLS_Update();
    b = 0;
    VBL_VSync(0);
    for (i = 0; i < SIZEARRAY(xval); i++)
        xval[i] = yval[i] = 0;
    nval = 0;
    i = 0;
    do {
        LLK_LastScan = 0;
        if (n == 0)
            JOY_Get(&x, &y, NULL, NULL, &b, NULL);
        else
            JOY_Get(NULL, NULL, &x, &y, NULL, &b);
        xval[i] = x;
        yval[i] = y;
        i = (i + 1)%SIZEARRAY(xval);
        if (nval < SIZEARRAY(xval))
            nval++;

//        GFX_Rectangle(0, 190, 80, 10, 0, 0);
//        TEXT_Printf(&MENU_FontWhite, 0, 190, 11, "%d,%d,%d", x, y, b);
//        LLS_Update();

        GL_Capture(LLK_LastScan);
        VBL_VSync(1);
    } while (b == 0 && LLK_LastScan != kESC);
    if (LLK_LastScan == kESC)
        return FALSE;
    f = 0;
    while (b != 0 || f < 40) {
        if (n == 0)
            JOY_Get(NULL, NULL, NULL, NULL, &b, NULL);
        else
            JOY_Get(NULL, NULL, NULL, NULL, NULL, &b);
        f += VBL_VSync(1);
    }
    x = y = 0;
    for (i = 0; i < nval; i++) {
        x += xval[i];
        y += yval[i];
    }
    x /= nval;
    y /= nval;
    if (jx != NULL) *jx = x;
    if (jy != NULL) *jy = y;
    return TRUE;
}

PRIVATE bool CalibrateJoystick(int n) {
    static const char *texts[] = {
        "Move to Upper-Left",
        "Center",
        "Move to Lower-Right",
        "Center again",
    };
    word *calx, *caly;
    if (n == 0) {
        calx = JOY_CalAX;
        caly = JOY_CalAY;
    } else {
        calx = JOY_CalBX;
        caly = JOY_CalBY;
    }

    if (CalibrateJoystickStep(n, calx+0, caly+0, texts[0])
     && CalibrateJoystickStep(n, calx+1, caly+1, texts[1])
     && CalibrateJoystickStep(n, calx+3, caly+3, texts[2])
     && CalibrateJoystickStep(n, calx+2, caly+2, texts[3])) {
        if (n == 0) {
            CFG_Config.JoyAX[0] = calx[0];
            CFG_Config.JoyAX[1] = calx[1];
            CFG_Config.JoyAX[2] = calx[2];
            CFG_Config.JoyAX[3] = calx[3];
            CFG_Config.JoyAY[0] = caly[0];
            CFG_Config.JoyAY[1] = caly[1];
            CFG_Config.JoyAY[2] = caly[2];
            CFG_Config.JoyAY[3] = caly[3];
        } else {
            CFG_Config.JoyBX[0] = calx[0];
            CFG_Config.JoyBX[1] = calx[1];
            CFG_Config.JoyBX[2] = calx[2];
            CFG_Config.JoyBX[3] = calx[3];
            CFG_Config.JoyBY[0] = caly[0];
            CFG_Config.JoyBY[1] = caly[1];
            CFG_Config.JoyBY[2] = caly[2];
            CFG_Config.JoyBY[3] = caly[3];
        }
        LLK_LastScan = 0;
        return TRUE;
    }
    LLK_LastScan = 0;
    return FALSE;
}

// -----------------------

PRIVATE bool ShowWarning(const char *msg) {
    int x, y;
    bool ret;

    LLK_LastScan = 0;
    x = LLS_SizeX/2 - 160;
    y = LLS_SizeY/2 - 100;
    GFX_Rectangle(x+20, y+70, 280,  60, 15, -1);
    GFX_Rectangle(x+21, y+71, 278,  58,  7, -1);
    GFX_Rectangle(x+22, y+72, 276,  56,  8, -1);
    GFX_Rectangle(x+23, y+73, 274,  54,  7,  8);

    TEXT_Write(&GL_YFont, x+25, y+ 75, "Attention", 15);
    TEXT_Write(&GL_YFont, x+40, y+115, "ESC to ignore", 15);
    if (msg != NULL)
        TEXT_Write(&GL_WFont, x+26, y+95, msg, 15);
    LLS_Update();
    while (LLK_LastScan == 0);
    ret = (LLK_LastScan == kESC);
    LLK_LastScan = 0;
    return ret;
}

PRIVATE bool CheckControls(void) {
    bool ignore = TRUE;
    int  i, j;
    bool bad;

        // Check if some key is duplicated.
    bad = FALSE;
    for (i = 0; !bad && i < 10; i++)
        for (j = i+1; !bad && j < 10; j++)
            if ((&CFG_Config.P1KeyUp)[i] == (&CFG_Config.P1KeyUp)[j])
                bad = TRUE;
    if (bad) {
        ignore &= ShowWarning("Some keys are duplicated");
    }

    if (!ignore) return ignore;

        // Check if joysticks are centered.
    if (CFG_Config.P1Control == 1 || CFG_Config.P2Control == 1) {
        int x, y;
        bool rez = TRUE;
        for (i = 0; rez && i < 10; i++)
            rez &= JOY_CalGet(16, 16, &x, &y, NULL, NULL, NULL, NULL);
        if (!rez)
            ignore &= ShowWarning("Joystick  A  Not Found");
        else if (x < -4 || x > 4 || y < -4 || y > 4)
            ignore &= ShowWarning("Joystick  A  is not centered");
    }

    if (!ignore) return ignore;

    if (CFG_Config.P1Control == 2 || CFG_Config.P2Control == 2) {
        int x, y;
        bool rez = TRUE;
        for (i = 0; rez && i < 10; i++)
            rez &= JOY_CalGet(16, 16, NULL, NULL, &x, &y, NULL, NULL);
        if (!rez)
            ignore &= ShowWarning("Joystick  B  Not Found");
        else if (x < -4 || x > 4 || y < -4 || y > 4)
            ignore &= ShowWarning("Joystick  B  is not centered");
    }
    return ignore;
}

PRIVATE int ControlConfigMenu(void) {
    bool leave = FALSE;
    int ret = 0;
    static int opt = 0;     // So it stays from call to call.
    static char p1opt[] = "Player 1 - Joystick A";
    static char p2opt[] = "Player 2 - Joystick B";
    static char *menu[] = {
        p1opt,
        p2opt,
        "Player 1 - Define Keys",
        "Player 2 - Define Keys",
        "Calibrate Joystick A",
        "Calibrate Joystick B",
        "Exit",
    };
    static const char *ctls[] = {
        "Keyboard",
        "Joystick A",
        "Joystick B",
        "Mouse",
    };
    int i, j;
    bool bad;

    if (MENU_Back == NULL)
        SetBackgroundScreen();

    do {
        sprintf(p1opt, "Player 1 - %s", ctls[CFG_Config.P1Control]);
        sprintf(p2opt, "Player 2 - %s", ctls[CFG_Config.P2Control]);
        switch (MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt)) {
            case kLARROW:
            case kKEYPAD4:
            case kKEYPADMINUS:
                if (opt == 0) {
                    if (CFG_Config.P1Control > 0)
                        CFG_Config.P1Control--;
                    else
                        CFG_Config.P1Control = 0;
                } else if (opt == 1) {
                    if (CFG_Config.P2Control > 0)
                        CFG_Config.P2Control--;
                    else
                        CFG_Config.P2Control = 0;
                }
                break;
            case kRARROW:
            case kKEYPAD6:
            case kKEYPADPLUS:
                if (opt == 0) {
                    if (CFG_Config.P1Control < 2)
                        CFG_Config.P1Control++;
                    else
                        CFG_Config.P1Control = 2;
                } else if (opt == 1) {
                    if (CFG_Config.P2Control < 2)
                        CFG_Config.P2Control++;
                    else
                        CFG_Config.P2Control = 2;
                }
                break;

            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                switch (opt) {
                    case 2:
                        DefineKeys(0); break;
                    case 3:
                        DefineKeys(1); break;
                    case 4:
                        CalibrateJoystick(0); break;
                    case 5:
                        CalibrateJoystick(1); break;
                    case SIZEARRAY(menu)-1:
                        if (CheckControls()) {
                            MENU_Return();
                            ret = -1; leave = TRUE;
                        }
                        break;
                }
                break;
            case kESC:
                if (CheckControls()) {
                    MENU_Return();
                    ret = -1; leave = TRUE;
                }
                break;
        }
    } while (!leave);

    CFG_Save("speed.cfg");
    return ret;
}

// ---------------

PRIVATE int RaceTypeMenu(void) {
    bool leave = FALSE;
    int ret = 0;
    static int opt = 0;     // So it stays from call to call.
    static char npopt[] = "2 Players";
    static char *menu[] = {
        "Championship",
        "Single Race",
        "Practice",
        npopt,
        "Select race mode...",
        "Select car model...",
        "Exit",
    };

    if (MENU_Back == NULL)
        SetBackgroundScreen();

    do {
        sprintf(npopt, "%d Player%s", GL_ConsolePlayers, GL_ConsolePlayers==1?"":"s");
        switch (MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt)) {
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                switch (opt) {
                    case 0:
                        if (!GL_Registered) {
                            MENU_Enter(MENU_SHAREWARE);
                            leave = TRUE; break;
                        }
                    case 1:
                    case 2:
                        MENU_Return();  // Quit this menu.
                        MENU_Return();  // Quit main menu.
                        ret = opt; leave = TRUE; break;

                    case 3:
                        GL_ConsolePlayers = 3 - GL_ConsolePlayers;
                        break;

                    case 4:
                        MENU_Enter(MENU_TIMEORLAPS);  // next menu.
                        leave = TRUE; break;

                    case 5:
                        MENU_Enter(MENU_CARTYPE);  // next menu.
                        leave = TRUE; break;

                    case SIZEARRAY(menu)-1:
                        MENU_Return();
                        ret = -1; leave = TRUE; break;
                }
                break;
            case kLARROW:
            case kKEYPAD4:
                if (opt == 3 && GL_ConsolePlayers == 2)
                    GL_ConsolePlayers = 1;
                break;
            case kRARROW:
            case kKEYPAD6:
                if (opt == 3 && GL_ConsolePlayers == 1)
                    GL_ConsolePlayers = 2;
                break;
            case kESC:
                MENU_Return();
                ret = -1; leave = TRUE;
                break;
        }
    } while (!leave);
    return ret;
}

PRIVATE int TimeOrLapsMenu(void) {
    bool leave = FALSE;
    int ret = 0;
    int nlaps =  CFG_Config.NumLaps;
    static int opt = 0;     // So it stays from call to call.
    static char rlaps[] = "Race for 00 laps";
    static char *menu[] = {
        rlaps,
        NULL,
        "Exit",
    };

    if (MENU_Back == NULL)
        SetBackgroundScreen();

    if (GL_TimedRace)
        opt = 1;
    else
        opt = 2;

    do {
        sprintf(rlaps, "Race for %2d laps", nlaps);
        if (GL_TimedRace)
            menu[1] = "Race Against Time";
        else
            menu[1] = "No Time Limit";

        switch (MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt)) {
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                switch (opt) {
                    case 1:
                        GL_TimedRace = !GL_TimedRace;
                        break;
                    case SIZEARRAY(menu)-1:
                        ret = -1; leave = TRUE; break;
                }
                break;

            case kLARROW:
            case kKEYPAD4:
            case kKEYPADMINUS:
                if (opt == 0) {
                    if (nlaps > 1) nlaps--;
                    else           nlaps = 1;
                } else if (opt == 1)
                    GL_TimedRace = !GL_TimedRace;
                break;
            case kRARROW:
            case kKEYPAD6:
            case kKEYPADPLUS:
                if (opt == 0) {
                    if (nlaps < 20) nlaps++;
                    else            nlaps = 20;
                } else if (opt == 1)
                    GL_TimedRace = !GL_TimedRace;
                break;
            case kESC:
                ret = -1; leave = TRUE; break;
        }
    } while (!leave);
    CFG_Config.NumLaps = nlaps;
    GL_RaceLaps = nlaps;
    CFG_Config.TimedRace = GL_TimedRace;
    CFG_Save("speed.cfg");
    MENU_Return();  // Quit this menu.
    return ret;
}

PRIVATE int ConfigMenu(void) {
    bool leave = FALSE;
    int ret = 0;
    static int opt = 0;     // So it stays from call to call.
    static char *menu[] = {
        "Sound Options",
        "Graphics Options",
        "Set Controls",
        "Multiplayer...",
        "Exit",
    };

    if (MENU_Back == NULL)
        SetBackgroundScreen();

    do {
        switch (MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt)) {
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                switch (opt) {
                    case 0:
                        MENU_Enter(MENU_SOUNDCONFIG);
                        leave = TRUE; break;
                    case 1:
                        MENU_Enter(MENU_DETAILCONFIG);
                        leave = TRUE; break;
                    case 2:
                        MENU_Enter(MENU_CONTROLCONFIG);
                        leave = TRUE; break;
                    case 3:
//                        MENU_Enter(MENU_SHAREWARE);
                        MENU_Enter(MENU_NETOPT);
                        leave = TRUE; break;
                    case SIZEARRAY(menu)-1:
                        MENU_Return();
                        ret = -1; leave = TRUE; break;
                }
                break;
            case kESC:
                MENU_Return();
                ret = -1; leave = TRUE;
                break;
        }
    } while (!leave);
    return ret;
}

PRIVATE int SharewareMenu(void) {
    int w;
    int f;
    if (MENU_Back == NULL)
        SetBackgroundScreen();
    LLK_LastScan = 0;
    f = 0;
    VBL_VSync(0);
    do {
        RepMovsb(LLS_Screen[0], MENU_Back, LLS_Size);
        if (f & 0x10) {
            TEXT_GetExtent(&MENU_FontWhite, 0, 0, "REGISTERED VERSION ONLY", &w, NULL);
            TEXT_Write(&MENU_FontWhite, (320-w)/2, 120, "REGISTERED VERSION ONLY", 11);
        }
        f += VBL_VSync(1);
        LLS_Update();
    } while (LLK_LastScan == 0);
    MENU_Return();
    return 0;
}

PRIVATE int CarTypeMenu(void) {
    bool leave = FALSE;
    int ret = 0;
    static int opt = 0;     // So it stays from call to call.
    static char *menu[] = {
        "Formula 1",
        "Stock",
//        "Prototypes",
        "Exit",
    };

    if (MENU_Back == NULL)
        SetBackgroundScreen();

    opt = CFG_Config.CarType;

    do {
        switch (MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt)) {
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                switch (opt) {
                    case 0:
                    case 1:
//                    case 2:
                        CFG_Config.CarType = opt;
                        GL_CarType = opt;
                        CFG_Save("speed.cfg");
                        MENU_Return();
                        ret = -1; leave = TRUE; break;
                    case SIZEARRAY(menu)-1:
                        MENU_Return();
                        ret = -1; leave = TRUE; break;
                }
                break;
            case kESC:
                MENU_Return();
                ret = -1; leave = TRUE;
                break;
        }
    } while (!leave);
    return ret;
}

PRIVATE int ChooseCircMenu(void) {
    byte *bar;
    IS2_PSprite level[3];
    int i, j;
    long l;
    int ret;
    static int sel = 0;
    int cpos, destpos, posvel;
    bool leave;
    GAME_PEffect voz;
    int frames = 0;

    REQUIRE(MDL_InitCircuits());
    if ( (i = BASE_CheckArg("track")) > 0) {
        i = atoi(ArgV[i]);
        if (!GL_Registered && i > 2)
            i = 1;
        if (i >= 1 && i <= MDL_NCircuits) {
            MENU_Return();
            return MDL_Circuits[i-1].nmap;
        }
    }
    REQUIRE(MDL_LoadCircuits());

    {   // Start VBL fading.
        VBL_DestRed = VBL_DestGreen = VBL_DestBlue = 0;
        VBL_DestPal = NULL;
        VBL_FadeSpeed = 2;
        VBL_FadeMode = VBL_FADEFAST;
        VBL_FadePos = 1;    // Go!
    }
    LLK_LastScan = 0;
    while (VBL_FadePos > 0 && LLK_LastScan == 0);
    VBL_FadePos = 0;
    VBL_ZeroPalette();
    LLK_LastScan = 0;

    voz = GL_LoadEffect("circuit.raw", 13000, FALSE);

    MENU_Init("mbg_circ.pix", NULL);
    memcpy(LLS_Screen[0], MENU_Back, LLS_Size);

    l = JCLIB_FileSize("mscbar.pix");
    bar = NEW(l);
    REQUIRE(bar != NULL);
    JCLIB_Load("mscbar.pix", bar, l);

    for (i = 48; i < 152; i++)
        for (j = 180; j < 300; j++)
            LLS_Screen[i][j] = GL_ClrTable[22*256 + LLS_Screen[i][j]];

    for (i = 0; i < l; i++)
        if (bar[i] != 0)
            LLS_Screen[10][i] = bar[i];

    TEXT_GetExtent(&MENU_FontYellow,  0, 0, "Arrow Keys to select", &i, NULL);
    TEXT_Write(&MENU_FontYellow,  (320-i)/2, 170, "Arrow Keys to select", 1);
    TEXT_GetExtent(&MENU_FontYellow,  60, 185, "Enter to confirm", &i, NULL);
    TEXT_Write(&MENU_FontYellow,  (320-i)/2, 185, "Enter to confirm", 1);

    memcpy(MENU_Back, LLS_Screen[0], LLS_Size);
    LLS_Update();

    REQUIRE( (level[0] = IS2_Load("msfbeg.is2")) != NULL);
    REQUIRE( (level[1] = IS2_Load("msfama.is2")) != NULL);
    REQUIRE( (level[2] = IS2_Load("msfpro.is2")) != NULL);

    {   // Start VBL fading.
        VBL_DestPal = GamePal;
        VBL_FadeSpeed = 4;
        VBL_FadeMode = VBL_FADEFULL;
        VBL_FadePos = 1;    // Go!
    }

    GAME_EFF_Start(SH_Vtal, -1, 256, 350, 0, voz);

    VBL_VSync(0);
    ret = 1;
    LLK_LastScan = 0;
    leave = FALSE;
    cpos    = (MDL_Circuits[0].spr->h + 20)*(sel);
    posvel = 0;
    while (!leave) {
        int nf;

        LLK_LastScan = 0;
        memcpy(LLS_Screen[0], MENU_Back, LLS_Size);

        destpos = (MDL_Circuits[0].spr->h + 20)*(sel);
        {
            int my, My;

            my = DRW_MinY; My = DRW_MaxY;
            DRW_MinY = 48; DRW_MaxY = 152;
            for (i = 0; i < MDL_NCircuits; i++) {
                IS2_Draw(MDL_Circuits[i].spr,
                         240 - MDL_Circuits[i].spr->w/2,
                         60 - cpos
                            + (MDL_Circuits[0].spr->h + 20)*(i),
                         MDL_Circuits[i].spr->w, MDL_Circuits[i].spr->h);
            }
            DRW_MinY = my; DRW_MaxY = My;
        }

        TEXT_GetExtent(&MENU_FontWhite, 0, 0, MDL_Circuits[sel].name, &i, 0);
        i = (180 - i)/2;
        if (i < 0) i = 3;
        TEXT_Write(&MENU_FontYellow, i, 48, MDL_Circuits[sel].name, 4);

        TEXT_Printf(&MENU_FontWhite,  20, 107, 15, "Length");
        TEXT_Printf(&MENU_FontYellow, 100, 107, 15,
                    "%d.%03d",
                    MDL_Circuits[sel].length/1000,
                    MDL_Circuits[sel].length%1000);
        TEXT_Printf(&MENU_FontWhite,  20, 120, 15, "Best");
        TEXT_Printf(&MENU_FontYellow, 100, 120, 15,
                    "%d'%02d\"%02d",
                    MDL_Circuits[sel].best/70/60,
                    (MDL_Circuits[sel].best/70)%60,
                    MDL_Circuits[sel].best%70*100/70);
        TEXT_Printf(&MENU_FontWhite,  20, 133, 15, "Record");
        TEXT_Printf(&MENU_FontYellow, 100, 133, 15,
                    "%d'%02d\"%02d",
                    REC_Circ[sel].time[GL_CarType]/70/60,
                    (REC_Circ[sel].time[GL_CarType]/70)%60,
                    (REC_Circ[sel].time[GL_CarType]%70)*100/70);
        if (MDL_Circuits[sel].level < SIZEARRAY(level))
            IS2_DrawHorizontal(level[MDL_Circuits[sel].level],
                               42, 80);//+3*MDL_Circuits[sel].level);

        if (sel >= 2 && !GL_Registered && (frames & 0x20))
            TEXT_Write(&MENU_FontWhite, 10, 150, "Registered Version", 15);

        LLS_Update();
        nf = VBL_VSync(1);
        frames += nf;
        while (nf-- > 0) {
            if (cpos < destpos) {
                if (posvel < 0) posvel = 0;
                else if (posvel < 7) posvel++;
            } else if (cpos > destpos) {
                if (posvel > 0) posvel = 0;
                else if (posvel > -7) posvel--;
            } else
                posvel = 0;
            cpos += posvel;
        }

        GL_Capture(LLK_LastScan);
        switch (LLK_LastScan) {
            case kLARROW:
            case kKEYPAD4:
            case kUARROW:
            case kKEYPAD8:
                if (sel <= 0) {
//                    sel = MDL_NCircuits - 1;
                } else {
                    sel--;
                }
                break;
            case kRARROW:
            case kKEYPAD6:
            case kDARROW:
            case kKEYPAD2:
                if (sel >= MDL_NCircuits - 1) {
//                    sel = 0;
                } else {
                    sel++;
                }
                break;
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                if (sel < 2 || GL_Registered)
                    leave = TRUE;
                break;
            case kESC:
                leave = TRUE;
                break;
            default:
                break;
        }
    }
    GAME_EFF_StopAll(SH_Vtal);
    GAME_EFF_Unload(SH_Vtal, voz);

    VBL_FadePos = 0;    // Stop fading.
    VBL_VSync(0);   // Ensure you have it.
    VBL_VSync(1);
    DISPOSE(bar);
    DISPOSE(level[2]);
    DISPOSE(level[1]);
    DISPOSE(level[0]);
    if (LLK_LastScan == kESC)
        ret = -1;
    else
        ret = MDL_Circuits[sel].nmap;
    LLK_LastScan = 0;
    MENU_Return();
    MDL_UnloadCircuits();
    return ret;
}


PRIVATE int ChooseCarMenu(void) {
    byte *bar;
    IS2_PSprite level[3], manual[2], maxspeed;
    int i;
    long l;
    int ret;
    bool leave;
    static int sel = 0;
    int selpos;
    R3D_TAngles objrot;
    R3D_TPosVector objpos;
    R3D_TRotMatrix rotm;
    word selrot;
    sint32 lx;
    GAME_PEffect voz, vozauto, vozmanu;
    int vozch = -1;
    bool first = TRUE;
    int frames = 0;
    byte *back;

    REQUIRE(MDL_InitCars());
    if ( (i = BASE_CheckArg("car")) > 0) {
        i = atoi(ArgV[i]);
        if (!GL_Registered)
            i = 1;
        if (i >= 1 && i <= 6) {
            MENU_Return();
            return MDL_Cars[i-1].n3d;
        }
    }
    if ( (back = NEW(LLS_Size)) == NULL)
        BASE_Abort("Out of memory for menu background");
    REQUIRE(MDL_LoadCars());
    {   // Start VBL fading.
        VBL_DestRed = VBL_DestGreen = VBL_DestBlue = 0;
        VBL_DestPal = NULL;
        VBL_FadeSpeed = 2;
        VBL_FadeMode = VBL_FADEFAST;
        VBL_FadePos = 1;    // Go!
    }
    LLK_LastScan = 0;

    voz = GL_LoadEffect("vihecul.raw", 13000, FALSE);
    vozauto = GL_LoadEffect("automati.raw", 13000, FALSE);
    vozmanu = GL_LoadEffect("manual.raw", 13000, FALSE);

    MENU_Init("mbg_car.pix", NULL);
    memcpy(LLS_Screen[0], MENU_Back, LLS_Size);

    l = JCLIB_FileSize("msfbar.pix");
    bar = NEW(l);
    REQUIRE(bar != NULL);
    JCLIB_Load("msfbar.pix", bar, l);
    for (i = 0; i < l; i++)
        if (bar[i] != 0)
            LLS_Screen[10][i] = bar[i];
    DISPOSE(bar);

    TEXT_GetExtent(&MENU_FontYellow,  0, 0, "Arrow Keys to select", &i, NULL);
    TEXT_Write(&MENU_FontYellow,  (320-i)/2, 170, "Arrow Keys to select", 1);
    TEXT_GetExtent(&MENU_FontYellow,  60, 185, "Enter to confirm", &i, NULL);
    TEXT_Write(&MENU_FontYellow,  (320-i)/2, 185, "Enter to confirm", 1);

    TEXT_Printf(&MENU_FontWhite, 15, 6, 0, "Player %d", MENU_SelCarNumPlayer+1);

    REQUIRE( (level[0] = IS2_Load("msfbeg.is2")) != NULL);
    REQUIRE( (level[1] = IS2_Load("msfama.is2")) != NULL);
    REQUIRE( (level[2] = IS2_Load("msfpro.is2")) != NULL);
    REQUIRE( (manual[0] = IS2_Load("msfaut.is2")) != NULL);
    REQUIRE( (manual[1] = IS2_Load("msfman.is2")) != NULL);
    REQUIRE( (maxspeed = IS2_Load("msfmax.is2")) != NULL);

    memcpy(MENU_Back, LLS_Screen[0], LLS_Size);
    while (VBL_FadePos > 0 && LLK_LastScan == 0);
    VBL_FadePos = 0;
    LLK_LastScan = 0;
    LLS_Update();

    {   // Start VBL fading.
        VBL_DestPal = GamePal;
        VBL_FadeSpeed = 2;
        VBL_FadeMode = VBL_FADEFULL;
        VBL_FadePos = 1;    // Go!
    }

    GAME_EFF_Start(SH_Vtal, -1, 256, 350, 0, voz);

    R3D_FocusX = 256;
    R3D_FocusY = 256*5/6;
    R3D_CenterX = LLS_SizeX/2;
    R3D_CenterY = 30;
    lx = -1000*(MDL_NCars-1)/2;
    VBL_VSync(0);
    selpos = 0;
    ret = 1;
    LLK_LastScan = 0;
    leave = FALSE;
    selrot = RND_GetNum();
    LLK_Autorepeat = FALSE;
    while (!leave) {
        int nf;
        LLK_LastScan = 0;
//        memcpy(LLS_Screen[0], MENU_Back, LLS_Size);
        if (selpos == 0) {
            memcpy(LLS_Screen[0], MENU_Back, LLS_Size);
            TEXT_GetExtent(&MENU_FontRed, 0, 0, MDL_Cars[sel].name, &i, 0);
            TEXT_Write(&MENU_FontRed, (320-i)/2, 38, MDL_Cars[sel].name, 4);

            if (MDL_Cars[sel].level < SIZEARRAY(level))
                IS2_DrawHorizontal(level[MDL_Cars[sel].level],
                                   208, 93);// + 3*MDL_Cars[sel].level);
            if (MDL_Cars[sel].automatic < SIZEARRAY(manual))
                IS2_DrawHorizontal(manual[MDL_Cars[sel].automatic],
                                   160, 100+level[0]->h);
            IS2_DrawHorizontal(maxspeed, 200, 100+level[0]->h+manual[0]->h+1);
            TEXT_Printf(&MENU_FontWhite, 220, 100+level[0]->h+manual[0]->h+1 + 11, 15,
                        "%d", MDL_Cars[sel].maxSpeed);
            for (i = 0; i < MDL_NCars; i++) {
                if (i != sel) {
                    objpos[0] = lx+900*i;
                    objpos[1] = -900;
                    objpos[2] = 5000;
                    objrot[0] = 0;
                    objrot[1] = 0;
                    objrot[2] = 0; //13457*i;
                    MDL_Cars[i].obj->pos = objpos;
                    MDL_Cars[i].obj->rot = objrot;
                    O3DM_MaxDetail = O3DD_TEXGOURAUD;
                    R3D_Gen3DMatrix(rotm, objrot);
                    O3DM_RotateObjVerts(MDL_Cars[i].obj, rotm);
                    O3DM_TranslateObj(MDL_Cars[i].obj, objpos);
                    O3DM_ProjectObject(MDL_Cars[i].obj);
                    O3DM_ParseVisibility(MDL_Cars[i].obj);
                    O3DM_CalcLights(MDL_Cars[i].obj, objrot[2]);
                    O3DM_Draw(MDL_Cars[i].obj);
                }
            }
            memcpy(back, LLS_Screen[0], LLS_Size);
            VBL_VSync(0);
        } else {
            memcpy(LLS_Screen[0], back, LLS_Size);
        }
/*
        TEXT_GetExtent(&MENU_FontRed, 0, 0, MDL_Cars[sel].name, &i, 0);
        TEXT_Write(&MENU_FontRed, (320-i)/2, 38, MDL_Cars[sel].name, 4);

        if (MDL_Cars[sel].level < SIZEARRAY(level))
            IS2_DrawHorizontal(level[MDL_Cars[sel].level],
                               208, 93);// + 3*MDL_Cars[sel].level);
        if (MDL_Cars[sel].automatic < SIZEARRAY(manual))
            IS2_DrawHorizontal(manual[MDL_Cars[sel].automatic],
                               160, 100+level[0]->h);
        IS2_DrawHorizontal(maxspeed, 200, 100+level[0]->h+manual[0]->h+1);
        TEXT_Printf(&MENU_FontWhite, 220, 100+level[0]->h+manual[0]->h+1 + 11, 15,
                    "%d", MDL_Cars[sel].maxSpeed);
        for (i = 0; i < MDL_NCars; i++) {
            if (i != sel) {
                objpos[0] = lx+900*i;
                objpos[1] = -900;
                objpos[2] = 5000;
                objrot[0] = 0;
                objrot[1] = 0;
                objrot[2] = 0; //13457*i;
                MDL_Cars[i].obj->pos = objpos;
                MDL_Cars[i].obj->rot = objrot;
                O3DM_MaxDetail = O3DD_TEXGOURAUD;
                R3D_Gen3DMatrix(rotm, objrot);
                O3DM_RotateObjVerts(MDL_Cars[i].obj, rotm);
                O3DM_TranslateObj(MDL_Cars[i].obj, objpos);
                O3DM_ProjectObject(MDL_Cars[i].obj);
                O3DM_ParseVisibility(MDL_Cars[i].obj);
                O3DM_CalcLights(MDL_Cars[i].obj, objrot[2]);
                O3DM_Draw(MDL_Cars[i].obj);
            }
        }
*/
        if (selpos >= 40)
            selpos = 40;

        {
#define INTERP(base,dest) ((base) + FPMultDiv((dest)-(base), 40-selpos, 40))
            objpos[0] = INTERP(-550, lx+900*sel);
            objpos[1] = INTERP(-800, -900);
            objpos[2] = INTERP(2000, 5000);
            objrot[0] = 0;
            objrot[1] = 0;
            objrot[2] = INTERP((sint16)selrot, 0);
            MDL_Cars[sel].obj->pos = objpos;
            MDL_Cars[sel].obj->rot = objrot;
            O3DM_MaxDetail = O3DD_TEXGOURAUD;
            R3D_Gen3DMatrix(rotm, objrot);
            O3DM_RotateObjVerts(MDL_Cars[sel].obj, rotm);
            O3DM_TranslateObj(MDL_Cars[sel].obj, objpos);
            O3DM_ProjectObject(MDL_Cars[sel].obj);
            O3DM_ParseVisibility(MDL_Cars[sel].obj);
            O3DM_CalcLights(MDL_Cars[sel].obj, objrot[2]);
            O3DM_Draw(MDL_Cars[sel].obj);
        }

        if (sel != 0 && !GL_Registered && (frames & 0x20))
            TEXT_Write(&MENU_FontWhite, 10, 145, "Registered Version", 15);

        LLS_Update();
        nf = VBL_VSync(1);
        frames += nf;
        if (selpos >= 40)
            selrot += nf*112;
        if (selpos < 39 && (selpos+nf) >= 39) {
            if (first)
                first = FALSE;
            else if (MDL_Cars[sel].automatic)
                vozch = GAME_EFF_Start(SH_Vtal, vozch, 256, 350, 0, vozmanu);
            else
                vozch = GAME_EFF_Start(SH_Vtal, vozch, 256, 350, 0, vozauto);
        }
        selpos += nf;
        GL_Capture(LLK_LastScan);
        switch (LLK_LastScan) {
            case kLARROW:
            case kKEYPAD4:
                if (sel <= 0) {
                    sel = MDL_NCars - 1;
                } else {
                    sel--;
                }
                first = FALSE;
                selpos = 0;
                selrot = RND_GetNum();
                break;
            case kRARROW:
            case kKEYPAD6:
                if (sel >= MDL_NCars - 1) {
                    sel = 0;
                } else {
                    sel++;
                }
                first = FALSE;
                selpos = 0;
                selrot = RND_GetNum();
                break;
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                if (sel == 0 || GL_Registered)
                    leave = TRUE;
                break;
            case kESC:
                leave = TRUE;
                break;
            default:
                break;
        }
    }
    LLK_Autorepeat = TRUE;

    GAME_EFF_StopAll(SH_Vtal);
    GAME_EFF_Unload(SH_Vtal, voz);
    GAME_EFF_Unload(SH_Vtal, vozauto);
    GAME_EFF_Unload(SH_Vtal, vozmanu);

    VBL_FadePos = 0;    // Stop fading.
    VBL_DumpPalette(GamePal, 0, 256);
    VBL_VSync(0);   // Ensure you have it.
    VBL_VSync(1);
    DISPOSE(maxspeed);
    DISPOSE(manual[1]);
    DISPOSE(manual[0]);
    DISPOSE(level[2]);
    DISPOSE(level[1]);
    DISPOSE(level[0]);
    if (LLK_LastScan == kESC)
        ret = -1;
    else
        ret = MDL_Cars[sel].n3d;
    LLK_LastScan = 0;
    MENU_Return();
    MDL_UnloadCars();
    DISPOSE(back);
    return ret;
}


PRIVATE int MainMenu(void) {
    bool leave = FALSE;
    int ret = 0;
    int i;
    static int opt = 0;     // So it stays from call to call.
    static char *menu[] = {
        "New Race",
        "Multiplayer Race",
        "Setup",
        "Quit",
    };
    byte pal[768];
    dword clock;

    NET_State = NETS_NONE;
    NET_NumNodes = 0;
    memset(NET_NodeData, 0, sizeof(NET_NodeData));
    memset(NET_NodeNum,  0, sizeof(NET_NodeNum));

    GL_NoCollide = FALSE;

        // If first time that this menu is run, do something nice.
        // Will do it when the menusystem is entered.
    if (MENU_Back == NULL) {
        byte *back;

        back = NEW(LLS_Size);
        if (back == NULL)
            BASE_Abort("Out of memory for menu background");
            // Set background.
        MENU_Init("mbg_prin.pix", NULL);
        memcpy(pal, GamePal, 768);
        for (i = 0; i < 15; i++)
            VGA_FadeOutPalette(pal, pal, 256, 63, 63, 63);

        VBL_VSync(0);
        {
            dword clk;
            int i;
            int lm, ls, ldx, ldy;
            int cy, cv;
            int by, bv;

            RepMovsb(back, MENU_Back, LLS_Size);

            DRW_Tile = FALSE;
            DRW_TranslatePtr = GL_ClrTable + 256*15; //Map.trans + 256*15;
            DRW_SetClipZone(0, 0, LLS_SizeX, LLS_SizeY, NULL);
            lm = 0;
            ls = 20*(1 << 8);
            cv = 0;
            bv = 0;
            clk = 0;
            LLK_LastScan = 0;
            while (lm < SIZEARRAY(LogoPos)) {
                int h, nf;

                if (cv < 0x4000)
                    cy = (LLS_SizeY << 4) - FPMult((LLS_SizeY-80)<< 4, FPMult(Sin(cv), Sin(cv)));
                else
                    cy = 80 << 4;
                if (bv < 0x4000)
                    by = FPMult(LLS_SizeY, FPMult(Sin(bv), Sin(bv)));
                else
                    by = LLS_SizeY;
                if ((cy >> 4) >= LLS_SizeY)
                    h = LLS_SizeY;
                else
                    h = (cy >> 4);

//                RepMovsb(LLS_Screen[0], MENU_Back, LLS_Size);
                RepMovsb(LLS_Screen[0], back, LLS_Size);

/*
                    // Draw logo
                ldx = 80;
                ldy = 20;
                for (i = 0; i < lm; i++) {
                    IS2_PSprite p;
                    p = LogoSprs[i];
                    IS2_Draw(p, LogoPos[i][0], LogoPos[i][1], p->w, p->h);
                    ldx += p->w - 3;
                    if (i == 4) {
                        ldy += p->h + 2;
                        ldx = 90;
                    }
                }
*/
                if (lm >= 0 && ls > 0 && lm < NLOGOSPRS) {
                    sint32 w, h;
                    IS2_PSprite p;

                    p = LogoSprs[lm];
                    w = FPnMult(ls+(1 << 8), p->w, 8);
                    h = FPnMult(ls+(1 << 8), p->h, 8);
                    IS2_Draw(p, LogoPos[lm][0], LogoPos[lm][1], w, h);
                }
                LLS_Update();
                GL_Capture(LLK_LastScan);
                if (LLK_LastScan != 0)
                    break;
                nf = VBL_VSync(1);
                while (nf-- > 0) {
                    if (lm > 2 && bv < 0x4000)
                        bv += 100;
                    if (lm > 5 && cv < 0x4000)
                        cv += 100;
                    if (lm < NLOGOSPRS) {
                        ls -= (1 << 9);
                        if (ls < 0) {
                            {   // Start VBL fading.
                                VBL_DumpPalette(pal, 0, 256);
                                VBL_DestPal = GamePal;
                                VBL_FadeSpeed = 5;
                                VBL_FadeMode = VBL_FADEFAST;
                                VBL_FadePos = 1;    // Go!
                            }
                            lm++;
                            if (lm < NLOGOSPRS) {
                                ls = 20*(1 << 8);
                                    // Draw logo
                                RepMovsb(LLS_Screen[0], MENU_Back, LLS_Size);
                                ldx = 80;
                                ldy = 20;
                                for (i = 0; i < lm; i++) {
                                    IS2_PSprite p;
                                    p = LogoSprs[i];
                                    IS2_Draw(p, LogoPos[i][0], LogoPos[i][1], p->w, p->h);
                                    ldx += p->w - 3;
                                    if (i == 4) {
                                        ldy += p->h + 2;
                                        ldx = 90;
                                    }
                                }
                                RepMovsb(back, LLS_Screen[0], LLS_Size);
                                VBL_VSync(0);
                            }
                        }
                    }
                    clk++;
                }
            }
            LLK_BIOSFlush();
            LLK_LastScan = 0;
        }
        LLS_Update();
        DISPOSE(back);
        SetBackgroundScreen();
            // Start VBL fading.
        memset(pal, 63, 768);
        VBL_DumpPalette(pal, 0, 256);
        VBL_DestPal = GamePal;
        VBL_FadeSpeed = 2;
        VBL_FadeMode = VBL_FADEFAST;
        VBL_FadePos = 1;    // Go!

    }

    clock = 0;
    VBL_VSync(0);
//    VGA_DumpPalette(GamePal, 0, 256);
    do {
        int key;

        key = MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt);
        switch (key) {
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                switch (opt) {
                    case 0:
                        if (LLK_Keys[kC])
                            GL_NoCollide = TRUE;
                        MENU_Enter(MENU_RACETYPE);
                        leave = TRUE; break;
                    case 1:
                        MENU_Enter(MENU_NETGAME);
                        leave = TRUE; break;
                    case 2:
                        MENU_Enter(MENU_CONFIG);
                        leave = TRUE; break;
                    case SIZEARRAY(menu)-1:
                        if (BASE_CheckArg("noquit") <= 0
                         || (LLK_Keys[kRIGHTCTRL] && LLK_Keys[kKEYPADMINUS] && LLK_Keys[kF5])
                        ) {
                            MENU_Return();
                            ret = -1; leave = TRUE;
                        }
                        break;
                }
                break;
            case kESC:
                if (opt == SIZEARRAY(menu)-1
                 && (BASE_CheckArg("noquit") <= 0
                  || (LLK_Keys[kRIGHTCTRL] && LLK_Keys[kKEYPADMINUS] && LLK_Keys[kF5])
                  )) {
                    MENU_Return();
                    ret = -1; leave = TRUE;
                }
                opt = SIZEARRAY(menu)-1;
                break;
        }
        if (key != 0)
            clock = 0;
        if (LLK_Keys[kI] && LLK_Keys[kG] && LLK_Keys[kN]) {
            MENU_Return();
            ret = -4;
            leave = TRUE;
        }
        clock += VBL_VSync(1);
    } while (!leave && clock < 20*70);
    if (!leave) {
        MENU_Return();
        return -2;
    }
    return ret;
}

// ----------------------------

PUBLIC char MENU_SongName[] = "12345678.123";

PUBLIC void MENU_InitMusic(const char *fname) {
    static PLAY_TInitData pid;

    if (fname == NULL || strncmp(fname, MENU_SongName, sizeof(MENU_SongName)) != 0) {
        if (fname != NULL)
            strncpy(MENU_SongName, fname, sizeof(MENU_SongName));
        else
            MENU_SongName[0] = '\0';

            // Wait for previous song to end its fade.
        if (SH_Vtal != NULL && SH_Vtal->SongPlaying && SH_Vtal->MusicDev != NULL)
            while (SH_Vtal->MusParams.Volume > 0 && !LLK_Keys[kESC])
                GAME_Poll(SH_Vtal, 20);   // Time up to 20 ticks (don't allow music to stop).

        if (fname == NULL) {
            GAME_MUS_Stop(SH_Vtal);
            GAME_MUS_Unload(SH_Vtal);
            return;
        }

        pid.LoopMod            = TRUE;
        pid.ForceLoopMod       = TRUE;
        pid.PermitFilterChange = TRUE;
        pid.FirstPattern       = 0;
        pid.RepStart           = 0;
        pid.SongLen            = 0;
        pid.RealTimerVal       = 0;

        GAME_MUS_Load(SH_Vtal, fname);
        GAME_MUS_Start(SH_Vtal, &pid);
    }
    GAME_MUS_ChangeVolume(SH_Vtal, CFG_Config.MusicVolume);
    GAME_MUS_SetFading(SH_Vtal, GAME_FADEIN, 0);
    GAME_Poll(SH_Vtal, 20);   // Time up to 20 ticks (don't allow music to stop).
}

PUBLIC void MENU_DoneMusic(void)
{
//    GAME_MUS_Stop(SH_Vtal);
//    GAME_MUS_Unload(SH_Vtal);
    GAME_MUS_SetFading(SH_Vtal, GAME_FADEOUT, 30);
}

PRIVATE int JukeBoxMenu(void) {
    int i, l;
    int ret;
    bool leave;
    static int cx = 0, cy = 0; //, ncirc = -1;
    int songs[8], nsongs;

    for (i = 0; i < 8; i++)
        songs[i] = (&CFG_Config.mus1)[i];
    nsongs = CFG_Config.nsongs;

    {   // Start VBL fading.
        VBL_DestRed = VBL_DestGreen = VBL_DestBlue = 63;
        VBL_DestPal = NULL;
        VBL_FadeSpeed = 3;
        VBL_FadeMode = VBL_FADEFAST;
        VBL_FadePos = 1;    // Go!
    }
    LLK_LastScan = 0;
    while (VBL_FadePos > 0 && LLK_LastScan == 0);
    VBL_FadePos = 0;
    LLK_LastScan = 0;

    MENU_Init("mbg_juke.pix", NULL);
    memcpy(LLS_Screen[0], MENU_Back, LLS_Size);

    memcpy(MENU_Back, LLS_Screen[0], LLS_Size);
    LLS_Update();

    {   // Start VBL fading.
        VBL_DestPal = GamePal;
        VBL_FadeSpeed = 2;
        VBL_FadeMode = VBL_FADEFULL;
        VBL_FadePos = 1;    // Go!
    }

    l = 0;
    VBL_VSync(0);
    ret = 1;
    LLK_LastScan = 0;
    leave = FALSE;
    while (!leave) {
        static const char *songnames[8] = {
            "Speed'em",
            "Machine rage",
            "Soul road",
            "Sakara bru",
            "Rock'n'roll",
            "Green brain",
            "Que mas da",
            "Over again",
        };
        int nf;

        LLK_LastScan = 0;
        memcpy(LLS_Screen[0], MENU_Back, LLS_Size);

        nsongs = 0;
        for (i = 0; i < 8; i++)
            if (songs[i] >= 0)
                nsongs++;

        for (i = 0; i < 8; i++)
            if (cx == 1 || cy != i || (l & 0x10))
                TEXT_Write(&MENU_FontYellow,  40, 84+13*i, songnames[i], 0);

        for (i = 0; i < nsongs; i++) {
            const char *p;
            if (songs[i] != -1)
                p = songnames[songs[i]];
            else
                p = "-------";
            if (cx == 0 || cy != i || (l & 0x10))
                TEXT_Write(&MENU_FontYellow,  184, 84+13*i, p, 0);
        }
        if (cx == 0) {
            char buf[200];
            sprintf(buf, "Enter to add - arrows to move");
            TEXT_GetExtent(&MENU_FontWhite,  0, 187, buf, &i, NULL);
            TEXT_Write(&MENU_FontWhite,  (320-i)/2, 188, buf, 1);
        } else {
            char buf[200];
            sprintf(buf, "Enter to delete - arrows to move");
            TEXT_GetExtent(&MENU_FontWhite,  0, 187, buf, &i, NULL);
            TEXT_Write(&MENU_FontWhite,  (320-i)/2, 188, buf, 1);
        }

        LLS_Update();
        nf = VBL_VSync(1);
        l += nf;
        GL_Capture(LLK_LastScan);
        switch (LLK_LastScan) {
            case kUARROW:
            case kKEYPAD8:
                if (cy > 0) cy--; break;
            case kDARROW:
            case kKEYPAD2:
                if (cy < 7 && (cx == 0 || cy < nsongs-1))
                    cy++;
                break;

            case kLARROW:
            case kKEYPAD4:
                if (cx > 0) cx--; break;
            case kRARROW:
            case kKEYPAD6:
                if (cx < 1 && nsongs > 0) {
                    if (cy > nsongs-1)
                        cy = nsongs-1;
                    cx++;
                }
                break;
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                if (cx == 1) {
                    for (i = cy; i < nsongs-1; i++)
                        songs[i] = songs[i+1];
                    songs[i] = -1;
                    if (cy == i)
                        if (cy > 0) cy--;
                        else        cx = 0;
                } else if (cx == 0) {
                    char buf[200];
                    sprintf(buf, "haste%d.s3m", cy+1);
                    MENU_DoneMusic();
                    MENU_InitMusic(buf);
                    for (i = 0; i < nsongs; i++)
                        if (songs[i] == cy)
                            break;
                    if (i >= nsongs) {
                        songs[nsongs++] = cy;
                    }
                }
                break;
            case kESC:
                leave = TRUE;
                break;
            default:
                break;
        }
    }

    VBL_FadePos = 0;    // Stop fading.
    VBL_DumpPalette(GamePal, 0, 256);
    VBL_VSync(0);   // Ensure you have it.
    VBL_VSync(1);
//    if (LLK_LastScan == kESC)
        ret = -3;               // Main menu reinit.
    LLK_LastScan = 0;
    MENU_Return();      // Back to previous menu
    MENU_Return();      // Back to previous menu
    MENU_Return();      // Back to main menu
    MENU_Return();      // Quit from main menu
    MENU_End();

    for (i = 0; i < 8; i++)
        (&CFG_Config.mus1)[i] = songs[i];
    CFG_Config.nsongs = nsongs;
    CFG_Save("speed.cfg");
    return ret;
}


PRIVATE int NetOptMenu(void) {
    bool leave = FALSE;
    int ret = 0;
    static int opt = 0;     // So it stays from call to call.
    static char ipxs[] = "IPX Socket 123456",
         comn[] = "COM 02",
         comp[] = "COM IO Address 02E8h",
         comi[] = "COM IRQ Line 07";
    int nipxs, ncomn, ncomp, ncomi;
    char *menu[] = {
        ipxs,
        comn,
        comp,
        comi,
        "Phonebook...",
        "Exit",
    };

    if (MENU_Back == NULL)
        SetBackgroundScreen();

    nipxs = CFG_Config.ipxsocket;
    ncomn = CFG_Config.comnum;
    ncomp = CFG_Config.comaddr;
    ncomi = CFG_Config.comirq;
    do {
        sprintf(ipxs, "IPX Socket %d", nipxs);
        sprintf(comn, "COM %.1d", ncomn);
        sprintf(comp, "COM IO Address %.3Xh", ncomp);
        sprintf(comi, "COM IRQ Line %d", ncomi);
        switch (MENU_DoControlMenu(menu, SIZEARRAY(menu), &opt)) {
            case kENTER:
            case kSPACE:
            case kKEYPADENTER:
                switch (opt) {
                    case 2: {
                        char buf[20];
                        sprintf(buf, "%X", ncomp);
                        if (MENU_DoInput("Enter port address", buf, sizeof(buf), "0123456789ABCDEF"))
                            sscanf(buf, "%x", &ncomp);
                        break;
                    }
                    case 4:
                        MENU_Enter(MENU_PHONES);
                        ret = 0; leave = TRUE; break;
                    case SIZEARRAY(menu)-1:
                        ret = -1; leave = TRUE; break;
                }
                break;
            case kLARROW:
            case kKEYPAD4:
                switch (opt) {
                    case 0: if (nipxs > 0) nipxs--; break;
                    case 1:
                        if (ncomn > 1) {
                            SER_TComPort p;
                            ncomn--;
                            SER_InitComInfo(&p, ncomn-1, -1, -1);
                            ncomp = p.port;
                            ncomi = p.irq;
                        }
                        break;
                    case 2: if (ncomp > 0x2E8) ncomp -= 0x10; break;
                    case 3: if (ncomi > 0) ncomi--; break;
                }
                break;
            case kRARROW:
            case kKEYPAD6:
                switch (opt) {
                    case 0: if (nipxs < 100) nipxs++; break;
                    case 1:
                        if (ncomn < 4) {
                            SER_TComPort p;
                            ncomn++;
                            SER_InitComInfo(&p, ncomn-1, -1, -1);
                            ncomp = p.port;
                            ncomi = p.irq;
                        }
                        break;
                    case 2: if (ncomp < 0x3F8) ncomp += 0x10; break;
                    case 3: if (ncomi < 7) ncomi++; break;
                }
                break;

            case kESC:
                ret = -1; leave = TRUE;
                break;
        }
    } while (!leave);
    CFG_Config.ipxsocket = nipxs;
    CFG_Config.comnum    = ncomn;
    CFG_Config.comaddr   = ncomp;
    CFG_Config.comirq    = ncomi;
    CFG_Save("speed.cfg");
    if (ret < 0)
        MENU_Return();
    return ret;
}

// -----------------------------------------------

#define MAXMENUS 20

PRIVATE int MenuStack[MAXMENUS];
PRIVATE int NumMenus = 0;

PUBLIC void MENU_Enter(int nmenu) {
    if (NumMenus >= MAXMENUS)
        MENU_Switch(nmenu);
    else
        MenuStack[NumMenus++] = nmenu;
}

PUBLIC void MENU_Switch(int nmenu) {
    if (NumMenus == 0)
        MENU_Enter(nmenu);
    else
        MenuStack[NumMenus] = nmenu;
}

PUBLIC void MENU_Return(void) {
    if (NumMenus > 0)
        NumMenus--;
}

PUBLIC int MENU_Run(void) {
    int rez = 0;
    while (NumMenus > 0) {
        LLK_LastScan = 0;
        LLK_Autorepeat = TRUE;
        switch (MenuStack[NumMenus-1]) {
            case MENU_SOUNDCONFIG:
                rez = SoundConfigMenu(); break;
            case MENU_DETAILCONFIG:
                rez = DetailConfigMenu(); break;
            case MENU_CONTROLCONFIG:
                rez = ControlConfigMenu(); break;

            case MENU_CHOOSECIRC:
                rez = ChooseCircMenu(); break;
            case MENU_CHOOSECAR:
                rez = ChooseCarMenu(); break;

            case MENU_CONFIG:
                rez = ConfigMenu(); break;
            case MENU_RACETYPE:
                rez = RaceTypeMenu(); break;
            case MENU_TIMEORLAPS:
                rez = TimeOrLapsMenu(); break;

            case MENU_JUKEBOX:
                rez = JukeBoxMenu(); break;

            case MENU_MAINDEMO:
                rez = MainDemoMenu(); break;

            case MENU_CARTYPE:
                rez = CarTypeMenu(); break;

            case MENU_SHAREWARE:
                rez = SharewareMenu(); break;

            case MENU_NETOPT:
                rez = NetOptMenu(); break;
            case MENU_PHONES:
                rez = NET_EditPhones(); break;
            case MENU_NETGAME:
//                rez = NetGameMenu(); break;
                rez = NET_GameMenu(); break;

            default:
                rez = MainMenu(); break;
        }
        LLK_Autorepeat = FALSE;
    }
    return rez;
}

// -------- Xapuzas varias

int MENU_SelCarNumPlayer = 0;

