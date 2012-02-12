// ------------------------------ CREDITS.C ----------------------------
// Bye Jare of Iguana (Javier Ar‚valo Baeza) in 1995.
// Copyright (C) 1994-1995 by the author.

#include "credits.h"

#include <llscreen.h>
#include <vbl.h>
#include <llkey.h>
#include <jclib.h>
#include <sincos.h>
#include <text.h>

#include <string.h>

#include "globals.h"

PRIVATE sint32 (*dtable)[1024];      // 32
PRIVATE sint32 (*stable)[1024];      // 32
PRIVATE byte (*image)[512];         // 256
PRIVATE byte (*clr)[256];           // 32
PRIVATE byte (*pix)[320];           // 200

PRIVATE FONT_TFont Fonts[2];

PRIVATE void DrawFlag(int a, int b, int idt) {
    sint32 *dt, *st;
    int i, j;
    int ia, jb;

    dt = dtable[idt];
    st = stable[idt];
    for (i = 0, ia = a; i < 200; i++, ia = (ia+2)&255) {
        sint32 *pki, *psi;
        sint32 kj;
        byte *src;
        byte *p;

        p = LLS_Screen[0] + i*LLS_SizeX;
        jb = (b+2*i)&255;
        pki = dt + jb;
        psi = st + jb;
        kj = dt[ia];
        src = image[i+(256-200)/2] + kj + (512-320)/2;
        for (j = 0; j < 320; j++, jb = jb+2, psi += 2, pki += 2) {
            *p++ = clr[31-*psi][src[(*pki) << 9]];
            src++;
        }
    }
}

PRIVATE void DrawFlagNoShade(int a, int b, int idt) {
    sint32 *dt, *st;
    int i, j;
    int ia, jb;

    dt = dtable[idt];
    st = stable[idt];
    for (i = 0, ia = a; i < 200; i++, ia = (ia+2)&255) {
        sint32 *pki;
        sint32 kj;
        byte *src;
        byte *p;

        p = LLS_Screen[0] + i*LLS_SizeX;
        jb = (b+2*i)&255;
        pki = dt + jb;
        kj = dt[ia];
        src = image[i+(256-200)/2] + kj + (512-320)/2;
        for (j = 0; j < 320; j++, jb = jb+2, pki += 2) {
            *p++ = src[(*pki) << 9];
            src++;
        }
    }
}

typedef struct {
    int font;
    int y;
    const char *text;
} TCredit, *PCredit;

PRIVATE int PrintCredits(PCredit c) {
    int k;
    k = 1;
    while (c->text != NULL) {
        int i;
        TEXT_GetExtent(Fonts + c->font, 0, 0, c->text, &i, 0);
        TEXT_Write(Fonts + c->font, (LLS_SizeX-i)/2, c->y, c->text, 15);
        c++;
        k++;
    }
    return k;
}

TCredit Credits[] = {
    {0,  40, NULL},

    {0,  60, "Game Design"},
    {1, 120, "Javier Arevalo"},
    {1, 140, "Noriaworks"},
    {0,  40, NULL},

    {0,  40, NULL},

    {0,  60, "Programming"},
    {1, 130, "Javier Arevalo"},
    {0,  40, NULL},

    {0,  40, "Additional programming"},
    {1,  80, "Juan Carlos Arevalo"},
    {1, 100, "Alejandro Luengo"},
    {0, 150, "VTAL Sound System"},
    {1, 170, "Juan Carlos Arevalo"},
    {0,  40, NULL},

    {0,  60, "Graphics"},
    {1, 120, "Cesar Valencia"},
    {1, 140, "Josete"},
    {0,  40, NULL},

    {0,  50, "Modelling"},
    {1, 115, "Jorge Rosado"},
    {1, 135, "Josete"},
    {1, 155, "Rafael Sanchez"},
    {0,  40, NULL},

    {0,  50, "Map Design"},
    {1,  80, "Leticia Krahe"},
    {0, 130, "Map Decoration"},
    {1, 160, "Jorge Rosado"},
    {0,  40, NULL},

    {0,  50, "Music and Sound Effects"},
    {1,  80, "Victor Segura"},
    {0, 130, "Voices"},
    {1, 160, "Emilio Mahoney"},
    {0,  40, NULL},

    {0,  50, "Moral Support"},
    {1, 110, "Friendware"},
    {1, 130, "Our families"},
    {1, 150, "Our friends"},
    {0,  40, NULL},

    {0,  50, "Moral Support"},
    {1, 110, "Friendware"},
    {1, 130, "Our families"},
    {1, 150, "Our friends"},
    {1, 175, "And, of course, you"},
    {0,  40, NULL},
};

PUBLIC int MENU_DoCredits(void) {
    int i, j;
    int a, b;
    int idt;
    int ncr = 0;
    int clock = 0;

    VBL_ZeroPalette();
    VBL_VSync(1);

    if (!((dtable = NEW(32*sizeof(*dtable))) != NULL))
        BASE_Abort("Out of memory for dtable");
    if (!((stable = NEW(32*sizeof(*stable))) != NULL))
        BASE_Abort("Out of memory for stable");
    if (!((image  = NEW(256*sizeof(*image))) != NULL))
        BASE_Abort("Out of memory for image");
    if (!((clr    = NEW(32*sizeof(*clr))) != NULL))
        BASE_Abort("Out of memory for clr");
    if (!((pix    = NEW(200*sizeof(*pix))) != NULL))
        BASE_Abort("Out of memory for pix");

    if (!(FONT_Load(&Fonts[0], "fontcr_1.fnt")))
        BASE_Abort("Credits font 1 not found");
    if (!(FONT_Load(&Fonts[1], "fontcr_2.fnt")))
        BASE_Abort("Credits font 2 not found");
    memset(image, 0, 256*sizeof(*image));

    for (i = 0; i < 32; i++)
        for (j = 0; j < SIZEARRAY(*dtable); j++)
            dtable[i][j] = FPMult(i, Cos(j*256+128));
    for (i = 0; i < 32; i++)
        for (j = 0; j < SIZEARRAY(*stable); j++)
            stable[i][j] = FPMult(3*i, FPMult(Cos(j*256+128),Cos(j*256+128)));

    JCLIB_Load("credflag.pix", pix[0], 64000);
    JCLIB_Load("credflag.clr", clr[0], 8192);
    for (i = 0; i < 200; i++)
        memcpy(image[i+(256-200)/2] + (512-320)/2, pix[i], 320);

    VBL_DestPal = GamePal;
    VBL_FadeSpeed = 1;
    VBL_FadeMode = VBL_FADEFAST;
    VBL_FadePos = 1;    // Go!
    a = 17; b = 93;
    idt = 6;
    clock = 0;
    ncr = 0;
    LLK_LastScan = 0;
    VBL_VSync(0);
    while (LLK_LastScan == 0 && ncr < SIZEARRAY(Credits)) {
        int nc;

        nc = ncr;
        DrawFlag(a, b, idt);
        nc = ncr + PrintCredits(Credits + ncr);
        clock += VBL_VSync(3);
        if (clock > (1+4*(Credits[ncr].text != NULL))*70) {
            ncr = nc;
            clock = 0;
        }
        LLS_Update();
        GL_Capture(LLK_LastScan);
        a = (a+5)&255;
        b = (b+9)&255;
    }
    LLK_LastScan = 0;
    idt = 5;
    for (j = 0; LLK_LastScan == 0 && j < 40; j++) {
        for (i = 0; i < 200; i++)
            memcpy(image[i+(256-200)/2] + (512-320)/2, LLS_Screen[i], 320);
        DrawFlagNoShade(a, b, idt);
        VBL_VSync(3);
        LLS_Update();
        GL_Capture(LLK_LastScan);
        if (j == (40-22)) {
            VBL_DestRed = VBL_DestGreen = VBL_DestBlue = 0;
            VBL_DestPal = NULL;
            VBL_FadeSpeed = 1;
            VBL_FadeMode = VBL_FADEFULL;
            VBL_FadePos = 1;    // Go!
        }
        a = (a+5)&255;
        b = (b+9)&255;
    }
    memset(LLS_Screen[0], 0, LLS_Size);
    LLS_Update();

    FONT_End(&Fonts[1]);
    FONT_End(&Fonts[0]);

    DISPOSE(pix);
    DISPOSE(clr);
    DISPOSE(image);
    DISPOSE(stable);
    DISPOSE(dtable);
    if (LLK_LastScan == kESC)
        return -1;
    return 0;
}
