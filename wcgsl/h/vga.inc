; -------------------------- VGA.INC --------------------------
; For use with WATCOM 9.5 + DOS4GW
; (C) Copyright 1993/4 by Jare & JCAB of Iguana.

        LOCALS @@
        .386P
        
; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
; Para ver los prototipos, mirar el VGA.H

_TEXT SEGMENT PARA PUBLIC USE32 'CODE'

GLOBAL VGA_Tweak_   : NEAR
GLOBAL VGA_ReTweak_ : NEAR
GLOBAL VGA_UnTweak_ : NEAR
GLOBAL VGA_Set80x50_: NEAR
GLOBAL VGA_SetMode_ : NEAR
GLOBAL VGA_GetMode_ : NEAR
GLOBAL VGA_Set16c_  : NEAR

GLOBAL VGA_WaitForRetrace_: NEAR
GLOBAL VGA_WaitForDisplay_: NEAR
GLOBAL VGA_VSync_         : NEAR

GLOBAL VGA_GetPalette_    : NEAR
GLOBAL VGA_DumpPalette_   : NEAR
GLOBAL VGA_FadeOutPalette_: NEAR
GLOBAL VGA_FadePalette_   : NEAR
GLOBAL VGA_ZeroPalette_   : NEAR

GLOBAL VGA_SetDisplayPage_   : NEAR
GLOBAL VGA_ClearPage_        : NEAR
GLOBAL VGA_FillPage_         : NEAR
GLOBAL VGA_CopyPage_         : NEAR
GLOBAL VGA_CopyVirtScr2ScrMX_: NEAR
GLOBAL VGA_CopyScrMX2VirtScr_: NEAR
GLOBAL VGA_CopyRAM2Scr_      : NEAR
GLOBAL VGA_CopyScr2RAM_      : NEAR

GLOBAL VGA_SetCharHeight_    : NEAR

SetBorder MACRO r,g,b
    LOCAL @@j
        MOV     DX,3C8h
        XOR     AL,AL
        OUT     DX,AL
        INC     DX
      IFDIF <&r>,<0>
        MOV     AL,&r
      ENDIF
        OUT     DX,AL
      IFDIF <&g>,<&r>
        MOV     AL,&g
      ENDIF
        OUT     DX,AL
      IFDIF <&b>,<&g>
        MOV     AL,&b
      ENDIF
        OUT     DX,AL
    @@j:
ENDM

SetPlanes MACRO p
        MOV     DX,3C4h
        MOV     AH,p
        MOV     AL,02h      ; planes
        OUT     DX,AX
ENDM

SetBitMask MACRO b
        MOV     DX,3CEh                 ; Bitmask...
        MOV     AH,b
        MOV     AL,08h
        OUT     DX,AX
ENDM

SetReadPlane MACRO b
        MOV     DX,3CEh                 ; Bitmask...
        MOV     AH,b
        MOV     AL,04h
        OUT     DX,AX
ENDM

ENDS

; -------------------------- VGA.INC --------------------------

