// ----------------------------- LLVESA.H ---------------------------
// For use with Watcom C.
// (C) Copyright 1995 by Jare & JCAB of Iguana.

// Vesa 1.2 basic code.

// Note that the structures below are plain VESA structures.
// Therefore, pointers are real-mode and must be converted with
// LLV_RFP2PTR() to be accessed.

#ifndef _LLVESA_H_
#define _LLVESA_H_

#include <vga.h>

#define LLV_RFP2PTR(p)  ((void *)(((((dword)(p)&0xFFFF0000)) >> 12) + (word)(p)))


#pragma pack(1)

typedef union {
    struct {
      char  Signature[4];   // 4 signature bytes: 'VESA'
      byte  Minor,          // VESA version number
            Major;
      dword OEMStringPtr;   // Pointer to OEM string
      dword Capabilities;   // Capabilities of the video environment
      dword VideoModePtr;   // Pointer to supported Super VGA modes
      word  TotalMemory;    // Number of 64kb memory blocks on board
    };
    byte fill[256];
} LLV_TVBEInfo, *LLV_PVBEInfo;

enum {
        // Capabilities.
    LLVC_DACSWITCHABLE = 0x00000001,
};

typedef union {
    struct {
        word  ModeAttributes     ; // mode attributes
        byte  WinAAttributes     ; // window A attributes
        byte  WinBAttributes     ; // window B attributes
        word  WinGranularity     ; // window granularity
        word  WinSize            ; // window size
        word  WinASegment        ; // window A start segment
        word  WinBSegment        ; // window B start segment
        dword WinFuncPtr         ; // pointer to window function
        word  BytesPerScanLine   ; // bytes per scan line

        word  XResolution        ; // horizontal resolution
        word  YResolution        ; // vertical resolution
        byte  XCharSize          ; // character cell width
        byte  YCharSize          ; // character cell height
        byte  NumberOfPlanes     ; // number of memory planes
        byte  BitsPerPixel       ; // bits per pixel
        byte  NumberOfBanks      ; // number of banks
        byte  MemoryModel        ; // memory model type
        byte  BankSize           ; // bank size in kb
        byte  NumberOfImagePages ; // number of images
        byte  Reserved           ; // reserved for page function

        byte  RedMaskSize        ; // size of direct color red mask in bits
        byte  RedFieldPosition   ; // bit position of LSB of red mask
        byte  GreenMaskSize      ; // size of direct color green mask in bits
        byte  GreenFieldPosition ; // bit position of LSB of green mask
        byte  BlueMaskSize       ; // size of direct color blue mask in bits
        byte  BlueFieldPosition  ; // bit position of LSB of blue mask
        byte  RsvdMaskSize       ; // size of direct color reserved mask in bits
        byte  DirectColorModeInfo; // Direct Color mode attributes
    };
    byte fill[256];
} LLV_TModeInfo, *LLV_PModeInfo;

enum {
        // Mode attributes.
    LLVMA_HARDSUPPORTED = 0x0001,
    LLVMA_BIOSSUPPORTED = 0x0004,
    LLVMA_SUPPORTED     = LLVMA_BIOSSUPPORTED|LLVMA_HARDSUPPORTED,
    LLVMA_COLOR         = 0x0008,
    LLVMA_GRAPHICS      = 0x0010,

        // Window attributes.
    LLVWA_SUPPORTED = 0x01,
    LLVWA_READABLE  = 0x02,
    LLVWA_WRITEABLE = 0x04,

        // Memory model.
    LLVMM_TEXT      = 0,
    LLVMM_VGA       = 1,
    LLVMM_HERCULES  = 2,
    LLVMM_4PLANE    = 3,
    LLVMM_PACKED    = 4,
    LLVMM_UNCHAINED = 5,
    LLVMM_DIRECT    = 6,
    LLVMM_YUV       = 7,

};

enum {
    LLVM_640x400x256  = 0x100,
    LLVM_640x480x256  = 0x101,
    LLVM_800x600x16   = 0x102,
    LLVM_800x600x256  = 0x103,
    LLVM_1024x768x16  = 0x104,
    LLVM_1024x768x256 = 0x105,
};

    // ----------------------------

    // Allocated by Init(). VBEInfo also read into.
PUBLIC LLV_PVBEInfo  LLV_VBEInfo;
PUBLIC LLV_PModeInfo LLV_ModeInfo;

PUBLIC bool LLV_Init(void);
PUBLIC void LLV_End(void);

    // ----------------------------

enum {
    LLV_GETVESAINFO,        // ES:DI ptr to 256-byte buffer.
    LLV_GETMODEINFO,        // CX mode, ES:DI ptr to 256-byte buffer.
    LLV_SETMODE,            // BX mode.
    LLV_GETMODE,            // BX <- mode.
    LLV_VESASTATE,          // DL get/save/restore, CX -> state, ES:BX buf.
    LLV_WINDOWCONTROL,      // BH set/get, BL window, DX pos.
    LLV_SCANLINELENGTH,     // BL set/get, CX pixperscan, DX maxnumscans.
    LLV_DISPLAYSTART,       // BL set/get, BH 0, CX x, DX y
    LLV_DACCONTROL,
};

PUBLIC bool LLV_VesaInt(int fn);

PUBLIC bool LLV_GetVesaInfo(LLV_PVBEInfo vi);

PUBLIC bool LLV_GetModeInfo(LLV_PModeInfo mi, word mode);

PUBLIC bool LLV_SetMode(word mode);

PUBLIC bool LLV_SetModeRez(int w, int h, int depth);

PUBLIC int  LLV_GetMode(void);

PUBLIC bool LLV_SetWindow(int w, int pos);

PUBLIC int  LLV_GetWindow(int w);

PUBLIC int  LLV_SetScanlineLength(int len, int *maxscans);

PUBLIC int  LLV_GetScanlineLength(int *maxscans);

PUBLIC bool LLV_SetDisplayStart(int x, int y);

PUBLIC bool LLV_GetDisplayStart(int *x, int *y);

    // Automatically assigned by SetWindow.
PUBLIC int LLV_WinPos[2];

    // By GetModeInfo
PUBLIC byte *LLV_WinAddr[2];
PUBLIC dword LLV_WinSize, LLV_BankRatio;
PUBLIC int LLV_WriteWindow, LLV_ReadWindow;

    // By the others:
PUBLIC int LLV_ScanLength, LLV_DispX, LLV_DispY;

    // ----------------------------
    // Ultra-fast macros for easy mem banking. Under development...

PUBLIC int   LLV_LastScan;
PUBLIC dword LLV_Offset;
PUBLIC bool  LLV_Split, LLV_Banked;

#define LLV_BANK(a) ((((dword)(a))/LLV_WinSize)*LLV_BankRatio)
//#define LLV_OFFS(a) ((((dword)(a))%LLV_WinSize)*LLV_BankRatio) ??
#define LLV_OFFS(a) ((((dword)(a))%LLV_WinSize))

#define LLV_SPLITSCAN(s) (((s)==LLV_LastScan)?  \
                             LLV_Split:         \
                             (LLV_Split=        \
    ((LLV_Offset = (((s)*LLV_ModeInfo->BytesPerScanLine)%LLV_WinSize)) \
     + LLV_ModeInfo->BytesPerScanLine > LLV_WinSize)))

// ------------------------------------------------
// A-bit-higher level functions.

PUBLIC void LLV_FillRectangle(int x0, int y0, int x1, int y1, byte c);

PUBLIC void LLV_PutRectangle(int x0, int y0, int x1, int y1, const byte *org, int olw);

PUBLIC void LLV_PutBuffer(const byte *org, int miny, int maxy);

#pragma pack()

#endif

// ----------------------------- LLVESA.H ---------------------------

