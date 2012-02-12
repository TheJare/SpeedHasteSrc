
#include <llvesa.h>
#include <dpmi.h>

#include <string.h>

PUBLIC bool LLV_VesaInt(int fn) {
    DPMI_rmi.b.AL = fn;
    DPMI_rmi.b.AH = 0x4F;
    DPMI_RealModeInt(0x10);
    return (DPMI_rmi.w.AX == 0x4F);
}

// ------------------------------------------------

    // Automatically assigned by SetWindow.
PUBLIC int LLV_WinPos[2] = {0, 0};

    // By GetModeInfo
PUBLIC byte *LLV_WinAddr[2] = { (byte *)0xA0000, (byte *)0xA0000 };
PUBLIC dword LLV_WinSize  = 0x10000, LLV_BankRatio = 0x10000;
PUBLIC int LLV_WriteWindow = 0, LLV_ReadWindow = 0;

    // By the others:
PUBLIC int LLV_ScanLength = 0,
           LLV_DispX = 0,
           LLV_DispY = 0;

PUBLIC bool LLV_GetVesaInfo(LLV_PVBEInfo vi) {
    if (vi == NULL || ((dword)vi) >= 0x100000)
        return FALSE;
    DPMI_rmi.w.ES = DPMI_PTR_SEG(vi);
    DPMI_rmi.w.DI = DPMI_PTR_OFS(vi);
    return LLV_VesaInt(LLV_GETVESAINFO);
}

PUBLIC bool LLV_GetModeInfo(LLV_PModeInfo mi, word mode) {
    if (mi == NULL || ((dword)mi) >= 0x100000)
        return FALSE;
    DPMI_rmi.w.ES = DPMI_PTR_SEG(mi);
    DPMI_rmi.w.DI = DPMI_PTR_OFS(mi);
    DPMI_rmi.w.CX = mode;
    if (!LLV_VesaInt(LLV_GETMODEINFO))
        return FALSE;
    LLV_WinSize  = mi->WinSize * 1024;
    if (mi->WinGranularity > 0 && mi->WinGranularity <= mi->WinSize)
        LLV_BankRatio = mi->WinSize/mi->WinGranularity;
    else
        LLV_BankRatio = 0;  // No banking poosible.
    LLV_WinAddr[0] = (byte*)(mi->WinASegment << 4);
    LLV_WinAddr[1] = (byte*)(mi->WinBSegment << 4);
        // If both support read & write, at least make them different.
    if (LLV_ModeInfo->WinAAttributes & (LLVWA_SUPPORTED | LLVWA_WRITEABLE))
        LLV_WriteWindow = 0;
    else
        LLV_WriteWindow = 1;
    if (LLV_ModeInfo->WinBAttributes & (LLVWA_SUPPORTED | LLVWA_READABLE))
        LLV_ReadWindow = 1;
    else
        LLV_ReadWindow = 0;
    LLV_WinPos[0] = LLV_GetWindow(0);
    LLV_WinPos[1] = LLV_GetWindow(1);
    LLV_ScanLength = mi->BytesPerScanLine;
    LLV_DispX = 0;
    LLV_DispY = 0;
    if (LLV_WinPos[LLV_ReadWindow] < 0 || LLV_WinPos[LLV_WriteWindow] < 0)
        return FALSE;
    return TRUE;
}

PUBLIC bool LLV_SetMode(word mode) {
    if (!LLV_GetModeInfo(LLV_ModeInfo, mode))
        return FALSE;
    DPMI_rmi.w.BX = mode;
    if (!LLV_VesaInt(LLV_SETMODE))
        return FALSE;
    return TRUE;
}

PUBLIC bool LLV_SetModeRez(int w, int h, int depth) {
    word *vmodeptr = LLV_RFP2PTR(LLV_VBEInfo->VideoModePtr);
    while (*vmodeptr != 0xFFFF) {
        if (LLV_GetModeInfo(LLV_ModeInfo, *vmodeptr)) {
            if (w == LLV_ModeInfo->XResolution
             && h == LLV_ModeInfo->YResolution
             && depth == LLV_ModeInfo->BitsPerPixel*LLV_ModeInfo->NumberOfPlanes) {
                DPMI_rmi.w.BX = *vmodeptr;
                if (LLV_VesaInt(LLV_SETMODE))
                    return TRUE;
            }
        }
        vmodeptr++;
    }
    return FALSE;
}

PUBLIC int LLV_GetMode(void) {
    if (!LLV_VesaInt(LLV_GETMODE))
        return -1;
    return DPMI_rmi.w.BX;
}

PUBLIC bool LLV_SetWindow(int w, int pos) {
    bool ret;
    assert(w == 0 || w == 1);
    LLV_WinPos[w] = pos;
    DPMI_rmi.b.BH = 0;
    DPMI_rmi.b.BL = w;
    DPMI_rmi.w.DX = pos;
    ret = LLV_VesaInt(LLV_WINDOWCONTROL);
    return ret;
}

PUBLIC int  LLV_GetWindow(int w) {
    assert(w == 0 || w == 1);
    DPMI_rmi.b.BH = 1;
    DPMI_rmi.b.BL = w;
    if (!LLV_VesaInt(LLV_WINDOWCONTROL))
        return -1;
    return DPMI_rmi.w.DX;
}

PUBLIC int LLV_SetScanlineLength(int len, int *maxscans) {
    DPMI_rmi.b.BL = 0;
    DPMI_rmi.w.CX = len;
    if (!LLV_VesaInt(LLV_SCANLINELENGTH))
        return -1;
    if (maxscans != NULL)
        *maxscans = DPMI_rmi.w.DX;
    LLV_ScanLength = DPMI_rmi.w.CX*LLV_ModeInfo->BytesPerScanLine/LLV_ModeInfo->XResolution;
    return DPMI_rmi.w.CX;
}

PUBLIC int LLV_GetScanlineLength(int *maxscans) {
    DPMI_rmi.b.BL = 1;
    if (!LLV_VesaInt(LLV_SCANLINELENGTH))
        return -1;
    if (maxscans != NULL)
        *maxscans = DPMI_rmi.w.DX;
    LLV_ScanLength = DPMI_rmi.w.CX*LLV_ModeInfo->BytesPerScanLine/LLV_ModeInfo->XResolution;
    return DPMI_rmi.w.CX;
}

PUBLIC bool LLV_SetDisplayStart(int x, int y) {
    DPMI_rmi.b.BH = 0;
    DPMI_rmi.b.BL = 0;
    DPMI_rmi.w.CX = x;
    DPMI_rmi.w.DX = y;
    if (!LLV_VesaInt(LLV_DISPLAYSTART))
        return FALSE;
    LLV_DispX = x;
    LLV_DispY = y;
    return TRUE;
}

PUBLIC bool LLV_GetDisplayStart(int *x, int *y) {
    DPMI_rmi.b.BL = 1;
    if (!LLV_VesaInt(LLV_SCANLINELENGTH))
        return FALSE;
    if (x != NULL)
        *x = DPMI_rmi.w.CX;
    if (y != NULL)
        *y = DPMI_rmi.w.DX;
    LLV_DispX = DPMI_rmi.w.CX;
    LLV_DispY = DPMI_rmi.w.DX;
    return TRUE;
}


// --------------------

PUBLIC LLV_PVBEInfo  LLV_VBEInfo  = NULL;
PUBLIC LLV_PModeInfo LLV_ModeInfo = NULL;

PUBLIC bool LLV_Init(void) {
    if (LLV_VBEInfo != NULL)
        return TRUE;
    if ( (LLV_VBEInfo  = DPMI_DOS_GetMem(sizeof(*LLV_VBEInfo)
                                       + sizeof(*LLV_ModeInfo))) == NULL)
        return FALSE;
    memset(LLV_VBEInfo, 0, sizeof(*LLV_VBEInfo) + sizeof(*LLV_ModeInfo));
//    memcpy(LLV_VBEInfo->Signature, "VBE2", 4);
    LLV_ModeInfo = (LLV_PModeInfo)(LLV_VBEInfo + 1);
        // 1.2 required.
    if (!LLV_GetVesaInfo(LLV_VBEInfo)
      || strncmp(LLV_VBEInfo->Signature, "VESA", 4) != 0
      || LLV_VBEInfo->Major < 1 || (LLV_VBEInfo->Minor == 1 && LLV_VBEInfo->Major < 2)) {
        LLV_End();
        return FALSE;
    }
    return TRUE;
}

PUBLIC void LLV_End(void) {
    if (LLV_VBEInfo != NULL)
        DPMI_DOS_FreeMem(LLV_VBEInfo);
    LLV_VBEInfo  = NULL;
    LLV_ModeInfo = NULL;
}

// ------------------------------------------------
// A-bit-higher level functions.

PUBLIC void LLV_FillRectangle(int x0, int y0, int x1, int y1, byte c) {
    int w, h, lw;
    int i, o, b;
    byte *p;

    c = (byte)c;
    c = c | (c << 8) | (c << 16) | (c << 24);
    lw = LLV_ScanLength;
    if (x0 > x1) { x0 ^= x1; x1 ^= x0; x0 ^= x1;}
    if (y0 > y1) { y0 ^= y1; y1 ^= y0; y0 ^= y1;}

    w = x1-x0;
    h = y1-y0;
    if (w <= 0 || h <= 0)
        return;

    o = LLV_OFFS(x0 + y0*lw);
    b = LLV_BANK(x0 + y0*lw);
    LLV_SetWindow(LLV_WriteWindow, b);
    p = LLV_WinAddr[LLV_WriteWindow] + o;
    for (i = 0; i < h; i++) {
        if ((o + lw) >= LLV_WinSize) {
            int n1, n2;
            n1 = LLV_WinSize - o;
            if (n1 > w)
                n1 = w;
            n2 = w - n1;
            if (n1 > 0)
                MemSetD(p, c, n1);
            b += LLV_BankRatio;
            LLV_SetWindow(LLV_WriteWindow, b);
            p -= LLV_WinSize;
            if (n2 > 0)
                MemSetD(p+n1, c, n2);
            o = o + lw - LLV_WinSize;
            p = p + lw;
        } else {
            MemSetD(p, c, w);
            p += lw;
            o += lw;
        }
    }
}

PUBLIC void LLV_PutRectangle(int x0, int y0, int x1, int y1, const byte *org, int olw) {
    int w, h, lw;
    int i, o, b;
    byte *p;

    lw = LLV_ScanLength;
    if (x0 > x1) { x0 ^= x1; x1 ^= x0; x0 ^= x1;}
    if (y0 > y1) { y0 ^= y1; y1 ^= y0; y0 ^= y1;}

    w = x1-x0;
    h = y1-y0;
    if (w <= 0 || h <= 0)
        return;

    o = LLV_OFFS(x0 + y0*lw);
    b = LLV_BANK(x0 + y0*lw);
    LLV_SetWindow(LLV_WriteWindow, b);
    p = LLV_WinAddr[LLV_WriteWindow] + o;
    for (i = 0; i < h; i++) {
        if ((o + lw) >= LLV_WinSize) {
                // Bank break between this scan and the next?
            int n1, n2;
            n1 = LLV_WinSize - o;   // Bytes until bank break.
            if (n1 > w)             // Clamp.
                n1 = w;
            n2 = w - n1;            // Bytes after bank break.
            if (n1 > 0)
                RepMovsb(p, org, n1);
            b += LLV_BankRatio;
            LLV_SetWindow(LLV_WriteWindow, b);
            p -= LLV_WinSize;
            if (n2 > 0)
                RepMovsb(p+n1, org+n1, n2);
            o = o + lw - LLV_WinSize;
            p = p + lw;
        } else {
            RepMovsb(p, org, w);
            p += lw;
            o += lw;
        }
        org += olw;
    }
}

PUBLIC void LLV_PutBuffer(const byte *org, int miny, int maxy) {
    int lw;
    int o, b, r;
    byte *p, *end;

    if (miny >= maxy)
        return;
    if (miny < 0)
        miny = 0;
    if (maxy > LLV_ModeInfo->YResolution)
        maxy = LLV_ModeInfo->YResolution;

    o  = LLV_ScanLength*miny;
    lw = LLV_ScanLength*(maxy - miny);
    b  = LLV_BANK(o);
    p  = LLV_WinAddr[LLV_WriteWindow] + LLV_OFFS(o);
    end = LLV_WinAddr[LLV_WriteWindow] + LLV_WinSize;
    while (lw > 0) {
        r = end - p;
        if (r > lw)
            r = lw;
        if (r < 0)
            BASE_Abort("r == %d, lw = %d, end = 0x%X, p = 0x%X, BankRatio = %d\n",
                       r, lw, end, p, LLV_BankRatio);
        LLV_SetWindow(LLV_WriteWindow, b);
        RepMovsb(p, org, r);
        p = LLV_WinAddr[LLV_WriteWindow];
        org += r;
        lw  -= r;
        b   += LLV_BankRatio;
    }
}


