; -------------------------- VGAA.ASM --------------------------
; For use with WATCOM 9.5 + DOS4GW
; (C) Copyright 1993/4 by Jare & JCAB of Iguana.

        LOCALS @@
        .386P
        
        INCLUDE VGA.INC

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

_TEXT SEGMENT PARA PUBLIC USE32 'CODE'
ASSUME CS:_TEXT

PUBLIC VGA_GetPalette_
VGA_GetPalette_:
        PUSH    EDI EDX
        CLD
        MOV     DX,3c7h                 ; Get palette.
        OUT     DX,AL
        ADD     EDX,2
@@pl1:
         IN     AL,DX
         STOSB
         IN     AL,DX
         STOSB
         IN     AL,DX
         STOSB
         LOOP   @@pl1
        POP     EDX EDI
        RET

; ==========================================================

PUBLIC VGA_DumpPalette_
VGA_DumpPalette_:
        PUSH    ESI EDX
        CLD
        MOV     DX,3c8h                 ; Setup palette.
        OUT     DX,AL
        INC     EDX
@@pl1:
         LODSB
         OUT    DX,AL
         LODSB
         OUT    DX,AL
         LODSB
         OUT    DX,AL
         LOOP   @@pl1
        POP     EDX ESI
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

PUBLIC VGA_ZeroPalette_
VGA_ZeroPalette_:
        PUSH    ECX EDX
        MOV     DX,3c8h                 ; Palette = all 0's.
        XOR     AL,AL
        OUT     DX,AL
        INC     DX
        MOV     ECX,256
@@pl1:
         OUT    DX,AL
         OUT    DX,AL
         OUT    DX,AL
         LOOP   @@pl1
        POP     EDX ECX
        RET

; ==========================================================

DoCol MACRO reg
LOCAL @@1, @@2
        LODSB
        CMP     AL,reg
        JZ      SHORT @@1
        JB      SHORT @@2
         SUB    AL,2
    @@2: INC    AL
    @@1:STOSB
ENDM

PUBLIC VGA_FadeOutPalette_
VGA_FadeOutPalette_:
        PUSH    EBX ESI EDI EBP
        CLD
        MOV     EBP,EAX
    @@lp:
        DoCol BL
        DoCol CL
        DoCol DL
        DEC     EBP
        JNZ     SHORT @@lp
        POP     EBP EDI ESI EBX
        RET

PUBLIC VGA_FadePalette_
VGA_FadePalette_:
        PUSH    EBX ESI EDI EBP
        CLD
        MOV     EBP,EAX
    @@lp:
        DoCol [EBX]
        DoCol [EBX+1]
        DoCol [EBX+2]
        ADD     EBX,3
        DEC     EBP
        JNZ     SHORT @@lp
        POP     EBP EDI ESI EBX
        RET

PURGE DoCol
DoCol MACRO reg
LOCAL @@1, @@2
        LODSB
        SUB     AL,reg
        JZ      SHORT @@1
        JB      SHORT @@2
         CMP    AL,AH
         JC     SHORT @@1
          MOV   AL,reg
          ADD   AL,AH
          MOV   [ESI-1],AL
        JMP     SHORT @@1
    @@2: NEG    AL
         CMP    AL,AH
         JC     SHORT @@1
          MOV   AL,reg
          SUB   AL,AH
          MOV   [ESI-1],AL
    @@1:
ENDM

PUBLIC VGA_FullFadeOutPalette_
VGA_FullFadeOutPalette_:
        PUSH    EBX ESI EDI EBP
        CLD
        MOV     EBP,EAX
        MOV     EAX,64
        SUB     EAX,EDI
        MOV     AH,AL
    @@lp:
        DoCol BL
        DoCol CL
        DoCol DL
        DEC     EBP
        JNZ     SHORT @@lp
        POP     EBP EDI ESI EBX
        RET

PUBLIC VGA_FullFadePalette_
VGA_FullFadePalette_:
        PUSH    EBX ESI EDI EBP
        CLD
        MOV     EBP,EAX
        MOV     EAX,65
        SUB     EAX,EDI
        MOV     AH,AL
    @@lp:
        DoCol [EBX]
        DoCol [EBX+1]
        DoCol [EBX+2]
        ADD     EBX,3
        DEC     EBP
        JNZ     SHORT @@lp
        POP     EBP EDI ESI EBX
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_SetDisplayPage_
VGA_SetDisplayPage_:
        PUSH    EDX
        MOV     DX,3D4h
        MOV     AL,0Ch
        MOV     AH,BH
        CLI
        OUT     DX,AX
        INC     AL
        MOV     AH,BL
        OUT     DX,AX
        STI
        POP     EDX
        RET
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_SetSplitScreen_
VGA_SetSplitScreen_:
        PUSH    EDX
        MOV     DX,3D4h
        MOV     AL,11h
        CLI
        OUT     DX,AL
        INC     EDX
        IN      AL,DX
        AND     AL,7Fh
        OUT     DX,AL
        DEC     EDX

        MOV     AH,CL
        MOV     AL,18h
        OUT     DX,AX

        MOV     AL,7
        OUT     DX,AL
        INC     EDX
        IN      AL,DX
        AND     AL,0EFh
        MOV     AH,CH
        AND     AH,1
        SHL     AH,4
        OR      AL,AH
        OUT     DX,AL
        DEC     EDX

        MOV     AL,9
        OUT     DX,AL
        INC     EDX
        IN      AL,DX
        AND     AL,0BFh
        MOV     AH,CH
        AND     AH,2
        SHL     AH,5
        OR      AL,AH
        OUT     DX,AL

        STI
        POP     EDX
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_SetCharHeight_
VGA_SetCharHeight_:
        PUSH EDX
        MOV     DX,3D4h
        MOV     AL,9
        CLI
        OUT     DX,AL
        INC     EDX
        IN      AL,DX
        AND     AL,0E0h
        OR      AL,CL
        OUT     DX,AL
        STI
        POP     EDX
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_ClearPage_
VGA_ClearPage_:
        PUSH    EDI EDX
        CLD
        ADD     EDI,0A0000h
        SetBitMask 0FFh
        SetPlanes   0Fh
        SHR     ECX,2
        XOR     EAX,EAX
        REP STOSD
        POP     EDX EDI
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_CopyPage_
VGA_CopyPage_:
        PUSH    EDI ESI EDX
        CLD
        ADD     EDI,0A0000h
        ADD     ESI,0A0000h
        SetBitMask 0h
        SetPlanes  0Fh
        REP MOVSB
        POP     EDX ESI EDI
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_FillPage_
VGA_FillPage_:
        PUSH    EDI EDX
        CLD
        ADD     EDI,0A0000h
        SetBitMask 0FFh
        SetPlanes   0Fh
        SHR     ECX,2
        IMUL    EAX,EBX,01010101h
        REP STOSD
        POP     EDX EDI
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_CopyVirtScr2ScrMX_
VGA_CopyVirtScr2ScrMX_:
        PUSH    EBX EDX EDI ESI
        CLD
        ADD     EDI,0A0000h
        SetBitMask 0FFh
        MOV     BH,1
        SHR     ECX,2

    @@l2:
         SetPlanes BH
         PUSH   ECX
         PUSH   ESI
         PUSH   EDI
         REP MOVSD
         POP    EDI
         POP    ESI
         POP    ECX
         ADD    ESI,80*200
         ADD    BH,BH
         CMP    BH,10h
         JNZ    SHORT @@l2
        POP     ESI EDI EDX EBX
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_CopyVirtScr2ScrMY_
VGA_CopyVirtScr2ScrMY_:
        PUSH    EBX EDX EDI ESI
        CLD
        ADD     EDI,0A0000h
        SetBitMask 0FFh
        MOV     BH,3
        SHR     ECX,2

    @@l2:
         SetPlanes BH
         PUSH   ECX
         PUSH   ESI
         PUSH   EDI
         REP MOVSD
         POP    EDI
         POP    ESI
         POP    ECX
         ADD    ESI,80*200
         ADD    BH,BH
         ADD    BH,BH
         CMP    BH,30h
         JNZ    SHORT @@l2
        POP     ESI EDI EDX EBX
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_CopyScrMX2VirtScr_
VGA_CopyScrMX2VirtScr_:
        PUSH    EBX EDX EDI ESI
        CLD
        ADD     ESI,0A0000h
        SetBitMask 0FFh
        XOR     BH,BH
        SHR     ECX,2

    @@l2:
         SetReadPlane BH
         PUSH   ECX
         PUSH   ESI
         PUSH   EDI
         REP MOVSD
         POP    EDI
         POP    ESI
         POP    ECX
         ADD    EDI,80*200
         INC    BH
         CMP    BH,4
         JNZ    SHORT @@l2
        POP     ESI EDI EDX EBX
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_CopyRAM2Scr_
VGA_CopyRAM2Scr_:
        PUSH    EBX EDX EDI ESI
        CLD
        ADD     EDI,0A0000h
        SetBitMask 0FFh
        MOV     BH,1

    @@l2:
         SetPlanes  BH
         PUSH   ECX
         PUSH   ESI
         PUSH   EDI
    @@l1:
          LODSB
          ADD   ESI,3
          STOSB
          LOOP  @@l1
         POP    EDI
         POP    ESI
         POP    ECX
         INC    ESI
         ADD    BH,BH
         CMP    BH,10h
         JNZ    SHORT @@l2
        POP     ESI EDI EDX EBX
        RET

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
PUBLIC VGA_CopyScr2RAM_
VGA_CopyScr2RAM_:
        PUSH    EBX EDX EDI ESI
        CLD
        ADD     ESI,0A0000h
        SetBitMask 0FFh
        XOR     BH,BH

    @@l2:
         MOV    EDX,3CEh
         MOV    AL,4
         MOV    AH,BH
         OUT    DX,AX
         PUSH   ECX
         PUSH   ESI
         PUSH   EDI
    @@l1:
          MOVSB
          ADD   EDI,3
          LOOP  @@l1
         POP    EDI
         POP    ESI
         POP    ECX
         INC    EDI
         INC    BH
         CMP    BH,4
         JNZ    SHORT @@l2
        POP     ESI EDI EDX EBX
        RET


ENDS
END

; --------------------- End of VGAA.ASM ------------------------
