
#include "intro.h"

#include <bitmap.h>
#include <llscreen.h>
#include <llkey.h>
#include <jclib.h>
#include <vga.h>
#include <vbl.h>
#include <stdlib.h>
#include <string.h>
#include <is2code.h>

#include <text.h>
#include <sincos.h>

// --------------------------------

typedef dword FireType;
#define FIREW 160
#define FIREH 115
#define FIRES   (sizeof(FireType))
#define MAXC  (255-16-1)


extern void FIRE_Move(FireType *dest, const FireType *src, FireType dec);
#pragma aux FIRE_Move modify [ECX EBX EAX EDX] parm [EDI] [ESI] [EDX]

extern void FIRE_Dump(byte *dest, const FireType *src);
#pragma aux FIRE_Dump modify [ECX EAX EDX] parm [EDI] [ESI]

PRIVATE void CreateFire(FireType *dest, int nflames) {
    int i, s;
    for (i = 0; i < nflames; i++) {
        s = RND_GetNum()&255;
        if (s < FIREW-0)
            dest[FIREW*(FIREH-4)+s+0] = MAXC*256;
    }
}

PRIVATE void DumpIs2(FireType *fd, IS2_PSprite sp, int x, int y, int n) {
    int i;
    const byte *s;

    s = ((byte*)sp) + sp->offsets[0];
    for (i = 0; i < n && i < sp->w; i++) {
        byte *dest;
        byte c;
        int dy;

        VGA_SetPlanes(1 << (x&3));
        dest = LLS_Screen[0]+y*LLS_SizeX/4 + x/4;
        dy = y%200;
        if ( (c = *s++) == 0)
            goto skipfirst;
        do {
            dest += (LLS_SizeX/4)*c;
            dy += c;
          skipfirst:
            if ( (c = *s++) == 0) break;
            else {
                if (i == n-1) {
                    FireType *pf;
                    int k;
                    pf = fd + FIREW*(dy/2) + x/2;
                    k = (dy+c)/2 - (dy)/2;
                    while (k-- > 0) {
                        *pf = (MAXC-(RND_GetNum()&0x3F))*256;
                        pf += FIREW;
                    }
                }
                dy += c;
                while (c-- > 0) {
                    *dest = *s++;
                    dest += LLS_SizeX/4;
                }

            }
            c = *s++;
        } while (c > 0);
        x++;
    }
}

PRIVATE bool DoFire(void) {
    FireType *b1, *b2;
    int i, j;
    FireType dec;
    int  nflames;
    int f, nf;
    int page, l;
    IS2_PSprite t1, t2;
    int w1, w2;
    byte *pal;

    b1 = NEW(FIREW*FIREH*FIRES);
    if (b1 == NULL)
        return FALSE;
    b2 = NEW(FIREW*FIREH*FIRES);
    if (b2 == NULL) {
        DISPOSE(b1);
        return FALSE;
    }
    memset(b1, 0, FIREW*FIREH*FIRES);
    memset(b2, 0, FIREW*FIREH*FIRES);
    t1 = IS2_Load("firet1.is2");
    if (t1 == NULL) {
        DISPOSE(b2);
        DISPOSE(b1);
        return FALSE;
    }
    t2 = IS2_Load("firet2.is2");
    if (t2 == NULL) {
        DISPOSE(t1);
        DISPOSE(b2);
        DISPOSE(b1);
        return FALSE;
    }
    pal = NEW(768);
    if (pal == NULL) {
        DISPOSE(t2);
        DISPOSE(t1);
        DISPOSE(b2);
        DISPOSE(b1);
        return FALSE;
    }

    LLK_LastScan = 0;
    LLS_Init(LLSM_DIRECT, LLSVM_MODEY);
    VGA_ZeroPalette();
    memset(VGA800x80, 0, 65536);
    for (i = 0, j = 0; i < 10; i++, j++) {
//        VGA_PutColor(i, 0, 0, 2*j);
        pal[3*i+0] = 0;
        pal[3*i+1] = 0;
        pal[3*i+2] = 2*j;
    }
    for (j = 0; j < 8; i++, j++) {
//        VGA_PutColor(i, j*8, 0, 0);
        pal[3*i+0] = j*8;
        pal[3*i+1] = 0;
        pal[3*i+2] = 0;
    }
    for (j = 0; j < 64; i++, j += 2) {
//        VGA_PutColor(i, 63, j, 0);
        pal[3*i+0] = 63;
        pal[3*i+1] = j;
        pal[3*i+2] = 0;
    }
    for (j = 0; j < 64; i++, j++) {
//        VGA_PutColor(i, 63, 63, j);
        pal[3*i+0] = 63;
        pal[3*i+1] = 63;
        pal[3*i+2] = j;
    }
    for (j = 0; i <= MAXC; i++, j++) {
//        VGA_PutColor(i, 63, 63, 63);
        pal[3*i+0] = 63;
        pal[3*i+1] = 63;
        pal[3*i+2] = 63;
    }
    for (j = 0; j < 16; i++, j++) {
//        VGA_PutColor(i, j*4+3, j*4+3, j*4+3);
        pal[3*i+0] = j*4+3;
        pal[3*i+1] = j*4+3;
        pal[3*i+2] = j*4+3;
    }

    VBL_Init(0);
    VBL_FullHandler = VBL_FadeHandler;
    dec = 256*MAXC;
    f = 1;
    w1 = w2 = 0;
    page = 0;
    l = 0;
    nf = 0;
    nflames = 200;
    VBL_ZeroPalette();
    VBL_VSync(0);
    VBL_VSync(2);
    {   // Start VBL fading.
        VBL_FadeStartColor = 0;
        VBL_FadeNColors = 256;
        VBL_DestPal = pal;
        VBL_FadeSpeed = 1;
        VBL_FadeMode = VBL_FADEFAST;
        VBL_FadePos = 1;    // Go!
    }
    while (LLK_LastScan == 0) {
        if (f > 3)
            f = 3;
        while (f-- > 0) {
            {void *t; t = b1; b1 = b2; b2 = t;}
            if (!LLK_Keys[kSPACE])
                CreateFire(b1, nflames);
            FIRE_Move(b2, b1, dec);
        }
        FIRE_Dump(VGA200x320[0] + ((l&0)/2+200*page)*LLS_SizeX/4, b2);

//        DumpIs2(b2, t1, 160-t1->w/2, 200*page+40, w1);
        DumpIs2(b2, t1, 160-t1->w/2, 200*page+40, t1->w+1);
        if (w1 <= t1->w)
            w1 += 3;
        if (nf > 70*9/3) {
            break;
        } else if (nflames > 0 && nf > 70*7/3) {
            nflames -= 10;
            //dec = dec + (dec+16)/16;
        } else if (1/*nf > 70*1/3*/ /* && w2 <= t2->w */) {
            if (dec > 380)
                dec = dec - dec/16;
        }
        if (nf > 70*1/3) {
            DumpIs2(b2, t2, 160-t2->w/2, 200*page+80, w2);
            if (w2 <= t2->w)
                w2 += 3;
        }

        VBL_ChangePage(80*200*page);
        page = !page;
        l++;
        f = VBL_VSync(3);
        f = f/3;
        nf += f;
    }
    VGA_SetPlanes(0x0F);
    VGA_SetBitMask(0xFF);
    VBL_ZeroPalette();
    VBL_VSync(0);
    VBL_VSync(2);

    VBL_Done();
    DISPOSE(pal);
    DISPOSE(t2);
    DISPOSE(t1);
    DISPOSE(b2);
    DISPOSE(b1);
    return (LLK_LastScan == kESC);
}

// --------------------------------

PRIVATE void FadeIn(const byte *pal, int speed) {
    byte pz[768];
    int i, j;
    VGA_GetPalette(pz, 0, 256);
    for (i = 0; i < 64; i += speed) {
        for (j = 0; j < speed; j++)
            VGA_FadePalette(pz, pz, 256, pal);
        VGA_VSync();
        VGA_DumpPalette(pz, 0, 256);
        if (LLK_LastScan != 0)
            break;
    }
    VGA_DumpPalette(pal, 0, 256);
}

PRIVATE void FadeOut(int r, int g, int b, int speed) {
    byte pz[768];
    int i, j;
    VGA_GetPalette(pz, 0, 256);
    for (i = 0; i < 64; i += speed) {
        for (j = 0; j < speed; j++)
            VGA_FadeOutPalette(pz, pz, 256, r, g, b);
        VGA_VSync();
        VGA_DumpPalette(pz, 0, 256);
        if (LLK_LastScan != 0)
            break;
    }
    for (i = 0; i < 768; i+=3) {
        pz[i+0] = r;
        pz[i+1] = g;
        pz[i+2] = b;
    }
    VGA_DumpPalette(pz, 0, 256);
}

PRIVATE void Intro1(const char *name, int w, int h) {
    BM_PBitmap bl;
    byte pal[768];
    char buf[200];

    sprintf(buf, "%s.pal", name);
    JCLIB_Load(buf, pal, 768);
    sprintf(buf, "%s.pix", name);
    bl = BM_Make(w, h, 0, buf);
    REQUIRE(bl != NULL);

//    FadeOut(0, 0, 0, 6);
    VGA_ClearPage(0, 65536);
    BM_Draw((LLS_SizeX-bl->Header.Width)/2-10, (LLS_SizeY-bl->Header.Height)/2, bl);
    DISPOSE(bl);
    FadeIn(pal, 3);
}

PRIVATE bool Wait(int n) {
    bool ret;
    while (n-- > 0 && LLK_LastScan == 0)
        VGA_VSync();
    ret = (LLK_LastScan == kESC);
    LLK_LastScan = 0;
    LLK_BIOSFlush();
    return ret;
}

PUBLIC void INTRO_DoIntro(void) {
    LLK_LastScan = 0;
    LLS_Init(LLSM_DIRECT, LLSVM_MODE13);
    VGA_ZeroPalette();

    Intro1("wwsplash", 320, 200);
    if (Wait(200)) goto bye;
    FadeOut(0, 0, 0, 6);
    VGA_ZeroPalette();

    Intro1("friend", 320, 200);
    if (Wait(200)) goto bye;
    FadeOut(0, 0, 0, 6);
    VGA_ZeroPalette();
/*
    LLS_Init(LLSM_DIRECT, LLSVM_MODEZ);
    VGA_ZeroPalette();
    Intro1("ivtl360", 224, 414);
    if (Wait(200)) goto bye;
    FadeOut(0, 0, 0, 6);
    VGA_ZeroPalette();
    LLS_Init(LLSM_DIRECT, LLSVM_MODE13);
    VGA_ZeroPalette();
*/
    Intro1("noria", 320, 200);
    if (Wait(200)) goto bye;
    FadeOut(0, 0, 0, 6);
    if (DoFire()) goto bye;
    FadeOut(0, 0, 0, 6);
  bye:
    LLS_Init(LLSM_DIRECT, LLSVM_MODE13);
    VGA_ZeroPalette();
}

// ----------------------------------------------
// Greetings effect.
// ----------------------------------------------

#define MAXDOTS 12000

typedef struct {
    byte *addr;
    byte  c;
} TDotsPlane, *PDotsPlane;

PRIVATE PDotsPlane DotPlanes[4];
PRIVATE int        NDotPlanes[4];

PRIVATE int       *AddTbl;
PRIVATE sint32    *InvTbl;

PRIVATE void InitDotPlanes(void) {
    NDotPlanes[0] = 0;
    NDotPlanes[1] = 0;
    NDotPlanes[2] = 0;
    NDotPlanes[3] = 0;
}

#define AddDot(x,y,c) {                                   \
    int p;                                                \
    PDotsPlane d;                                         \
                                                          \
    p = x & 3;                                            \
    d = DotPlanes[p] + (NDotPlanes[p]++);                 \
    d->addr = VGA800x80[0] + AddTbl[y] + x/4;             \
    d->c = c;                                             \
}

PRIVATE void DumpDots(dword base) {
    int i, n;

    for (i = 0; i < 4; i++) {
        PDotsPlane p;
        p = DotPlanes[i];
        VGA_SetPlanes(1 << i);
        for (n = NDotPlanes[i]-1; n >= 0; n--) {
            p->addr[base] = p->c;
            p++;
        }
    }
}

// ---------------------------------

#define XRANGE (1 << 22)
#define YRANGE (XRANGE)

#define ZMIN   2
#define ZRANGE 16384
#define ZMAX   (ZMIN+ZRANGE)

typedef struct {
    sint32 x, y, z;
} TStar, *PStar;

PRIVATE TStar *Stars;

PRIVATE sint32 Pos;

typedef void (*StarGenFunc)(PStar st);

PRIVATE void AddStar1(PStar st) {
    st->x = (RND_GetNum() % XRANGE) - XRANGE/2;
    st->y = (RND_GetNum() % YRANGE) - YRANGE/2;
}

PRIVATE void AddStar2(PStar st) {
    int r;

    r = RND_GetNum();
    st->x = FPMult(XRANGE/16, Cos(r));
    st->y = FPMult(YRANGE/16, Sin(r));
}

PRIVATE void AddStar3(PStar st) {
    int r;

    r = RND_GetNum();
    st->x = (RND_GetNum() % XRANGE) - XRANGE/2;
    st->y = YRANGE/8;
}

PRIVATE void AddStar4(PStar st) {
    int r;

    r = RND_GetNum();
    st->x = (RND_GetNum() % XRANGE) - XRANGE/2;
    st->y = /*YRANGE/16 +*/ FPMult(YRANGE/16, Cos(Pos*8)/2 + Cos(st->x/16)/2);
//    st->y = YRANGE/8 + FPMult(YRANGE/16, Cos(Pos*16 + st->x/32));
}

PRIVATE void AddStar5(PStar st) {
    int r;

//    if ((Pos & 0x1FFF) > 0xFF0 && (Pos & 0x1FFF) < 0x1010) {
        r = RND_GetNum();
        if (r & 1) {
            st->x = ((RND_GetNum() % XRANGE) & ~0xFFFFF) - XRANGE/2 + 0x100000/2;
            st->y = ((RND_GetNum() % YRANGE)) - YRANGE/2;
        } else {
            st->x = ((RND_GetNum() % XRANGE)) - XRANGE/2;
            st->y = ((RND_GetNum() % YRANGE) & ~0xFFFFF) - YRANGE/2 + 0x100000/2;
        }
/*
    } else {
        st->x = ((RND_GetNum() % XRANGE) & ~0xFFFFF) - XRANGE/2;
        st->y = ((RND_GetNum() % YRANGE) & ~0xFFFFF) - YRANGE/2;
    }
*/
}


PRIVATE StarGenFunc StarGen[] = {
    AddStar1,
    AddStar2,
    AddStar3,
    AddStar4,
    AddStar5,
};

// ---------------------------------

PRIVATE FONT_TFont Font;

PRIVATE int Write(dword page, int x, int y, const char *str, int ink) {
    byte c;
    int width;
    byte *pc, *sc, *sb;
    int i, j;

    if (Font.data == NULL)
        return x;

    sb = VGA800x80[y] + page;
    while ( (c = (byte)*str++) != '\0') {
        if (c == ' ') {
            x += 6;
            continue;
        }
        width = FONT_Index(&Font, c, &pc);
        for (i = 0; i < width; i++) {
            sc = sb + x/4;
            VGA_SetPlanes(1 << (x&3));
/*
            for (j = 0; j < Font.height; j++) {
                if (*pc > 0)
                    *sc = *pc + ink;
                pc += 1;//width;
                sc += 80;
            }
*/
#define DOL(i) {if (pc[i] > 0) sc[80*i] = pc[i]+ink;}
            for (j = 0; j < 3; j++) {
                DOL(0);
                DOL(1);
                DOL(2);
                DOL(3);
                DOL(4);
                DOL(5);
                DOL(6);
                DOL(7);
                pc += 8;
                sc += 80*8;
            }
            DOL(0);
            DOL(1);
            DOL(2);
            DOL(3);
            DOL(4);
            DOL(5);
            DOL(6);
            pc += 7;
//            pc += Font.height;
//            pc -= width*Font.height - 1;
            x++;
        }
        x++;
    }
    x--;
    return x;
}

// ---------------------------------

#define CTABLEW 380

PRIVATE void GenTables(int *ct, int *st, sint32 angle) {
    int i;
    for (i = -CTABLEW; i < CTABLEW; i++) {
        *ct++ = FPMult(2*i, Cos(angle));
        *st++ = FPMult(2*i, Sin(angle));
    }
}


PUBLIC void INTRO_DoGreets(void) {
    dword page;
    int i;
    int loops, f, prevnf;
    StarGenFunc sf;
    int starsv = 1, gentime;
    int CosTb[CTABLEW*2], SinTb[CTABLEW*2];
    sint32 angle, va;
    int NDots = MAXDOTS;
    sint32 destsv = 100;
    sint32 destva = 200;

    DotPlanes[0] = NEW(MAXDOTS*sizeof(*DotPlanes[0])*4);
    if (DotPlanes[0] == NULL)
        return;
    DotPlanes[1] = DotPlanes[0] + MAXDOTS;
    DotPlanes[2] = DotPlanes[1] + MAXDOTS;
    DotPlanes[3] = DotPlanes[2] + MAXDOTS;
    AddTbl = NEW(sizeof(*AddTbl)*200);
    if (AddTbl == NULL) {
        DISPOSE(DotPlanes[0]);
        return;
    }
    for (i = 0; i < 200; i++)
        AddTbl[i] = 80*i;
    Stars = NEW(MAXDOTS*sizeof(*Stars));
    if (Stars == NULL) {
        DISPOSE(AddTbl);
        DISPOSE(DotPlanes[0]);
        return;
    }
    InvTbl = NEW(ZRANGE*sizeof(*InvTbl));
    if (InvTbl == NULL) {
        DISPOSE(Stars);
        DISPOSE(AddTbl);
        DISPOSE(DotPlanes[0]);
        return;
    }
    for (i = 0; i < ZRANGE; i++)
        InvTbl[i] = (1 << 30)/(ZMIN+i);

    FONT_Load(&Font, "fontgrt.fnt");
    printf("Font height %d\n", Font.height);

    LLS_Init(LLSM_DIRECT, LLSVM_MODEY);
    VGA_ZeroPalette();
    memset(VGA800x80, 0, 65536);

    for (i = 0; i < 16; i++)
        VGA_PutColor(i, i*4, i*4, i*4);
    for (i = 0; i < 16; i++)
        VGA_PutColor(i+16, 16+3*i, i*2, 0);
    for (i = 0; i < 16; i++)
        VGA_PutColor(i+32, 0, 2*i, 16+i*2);

    sf = AddStar5;
    gentime = 4*70;

    for (i = 0; i < MAXDOTS; i++) {
        Stars[i].z = RND_GetNum() % ZRANGE + ZMIN;
        sf(Stars + i);
    }

    prevnf = 1;
    Pos = 0;
    angle = 0;
    va = 0;
    page = 0;
    loops = 0;
    f = 0;
    LLK_LastScan = 0;
    while (LLK_LastScan != kESC) {
        int nf;

        if (LLK_Keys[kT]) VGA_SetBorder(63, 0, 0);
        if (LLK_Keys[kUARROW])
            starsv++;
        if (LLK_Keys[kDARROW])
            starsv--;
        if (LLK_Keys[kLARROW])
            va--;
        if (LLK_Keys[kRARROW])
            va++;

        if (LLK_Keys[k1] && SIZEARRAY(StarGen) > 0)
            sf = StarGen[0];
        if (LLK_Keys[k2] && SIZEARRAY(StarGen) > 1)
            sf = StarGen[1];
        if (LLK_Keys[k3] && SIZEARRAY(StarGen) > 2)
            sf = StarGen[2];
        if (LLK_Keys[k4] && SIZEARRAY(StarGen) > 3)
            sf = StarGen[3];
        if (LLK_Keys[k5] && SIZEARRAY(StarGen) > 4)
            sf = StarGen[4];

        angle += va;
        GenTables(CosTb, SinTb, angle);

        page = (page + 80*200);
        if (page >= 4*80*200)
            page = 0;

        VGA_SetPlanes(0x0F);
        memset(VGA800x80[0] + page, 0, 80*200);
        InitDotPlanes();
        {
            PStar ps;
            ps = Stars;
            for (i = 0; i < NDots; i++, ps++) {
                int x, y, c;
                sint32 z;

                z = InvTbl[ps->z-ZMIN];
                x = CTABLEW + FPMult(2*ps->x, z);
                if (x < 0 || x >= (CTABLEW*2)) {
/*
                    if (starsv > 0) {
                        ps->z = ZMAX - RND_GetNum()%1024;
                        sf(ps);
                    }
*/
                } else {
                    y = CTABLEW + FPMult(2*ps->y, z);
                    if (y < 0 || y >= (CTABLEW*2)) {
/*
                        if (starsv > 0) {
                            ps->z = ZMAX - RND_GetNum()%1024;
                            sf(ps);
                        }
*/
                    } else {
                        int rx, ry;
                        rx = 160 + (CosTb[x] + SinTb[y])/4;
                        ry = 100 + (CosTb[y] - SinTb[x])/4;
                        if (rx >= 0 && rx < 320 && ry >= 0 && ry < 200) {
                            if (ps->z < (ZMIN+ZRANGE/2))
                                c = 15;
                            else
                                c = 15 - (ps->z-ZMIN-ZRANGE/2)*16/(ZRANGE/2);
                            AddDot(rx, ry, c);
                        }
                    }
                }
                ps->z -= starsv;
                if (ps->z < ZMIN) {
                    ps->z = ZMAX - RND_GetNum()%1024;
                    sf(ps);
                } else if (ps->z > ZMAX) {
                    ps->z = ZMIN + RND_GetNum()%1024;
                    sf(ps);
                }
            }
        }
        Pos += starsv;
        DumpDots(page);

        if (f > 1*70) {
            Write(page, 0, 0, "Greets from Jare to", 16);
//            if (f & 128)
            {
                Write(page, 0,  60, "All my friends and", 32);
                Write(page, 0, 100, "everyone in the scene", 32);
            }
        }

        if (starsv < destsv)      starsv++;
        else if (starsv > destsv) starsv--;
        else {
            destsv = RND_GetNum()%200 - 100;
            destsv = Sgn(destsv)*200 + destsv;
        }
        if (va < destva)          va++;
        else if (va > destva)     va--;
        else {
            destva = RND_GetNum()%200 - 100;
            destva = Sgn(destva)*100 + destva;
        }

        if (gentime-- <= 0) {
            gentime = 70 + RND_GetNum()%200;
            sf = StarGen[RND_GetNum()%SIZEARRAY(StarGen)];
        }

        VBL_ChangePage(page);
        VGA_SetBorder(0, 0, 0);
        nf = VBL_VSync(1);
        if (NDots > MAXDOTS/10 && nf > 1 && prevnf > 1)
            NDots = NDots - NDots/16;
        else if (NDots < MAXDOTS && nf == 1 && prevnf == 1) {
            NDots = NDots + NDots/64;
            if (NDots > MAXDOTS)
                NDots = MAXDOTS;
        }
        prevnf = nf;
        f += nf;
        loops++;
    }

    FONT_End(&Font);
    DISPOSE(InvTbl);
    DISPOSE(Stars);
    DISPOSE(AddTbl);
    DISPOSE(DotPlanes[0]);

    LLS_Init(LLSM_VIRTUAL, LLSVM_MODE13);
    VBL_ChangePage(0);
    VGA_ZeroPalette();
    VBL_ZeroPalette();
    VGA_SetPlanes(0x0F);
    memset(VGA800x80[0], 0, 65536);
    VBL_VSync(0);
    VBL_VSync(2);
}
