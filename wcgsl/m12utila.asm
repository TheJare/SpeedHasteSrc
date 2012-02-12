; ------------------ M12UTILA.ASM ----------------
; Mode 12h routines (C) 1994 by Javier Arevalo
; ------------------------------------------------

        .386P
        .MODEL FLAT
        LOCALS @@
        JUMPS

        .CODE

XMIN=0
XMAX=640
YMIN=0
YMAX=819        ;480  Why restrict?
SCRW=80

SetBitMask MACRO p
        MOV     DX,3CEh
        MOV     AL,8
        OUT     DX,AL
        INC     EDX
        MOV     AL,p
        OUT     DX,AL
ENDM

SetWritePlanes MACRO p
        MOV     DX,3C4h         ; Write Plane.
        MOV     AL,2
        OUT     DX,AL
        INC     EDX
        MOV     AL,p
        OUT     DX,AL
ENDM

SetReadPlane MACRO p
        MOV     DX,3CEh         ; Read Plane.
        MOV     AL,4
        OUT     DX,AL
        INC     EDX
        MOV     AL,p
        OUT     DX,AL
ENDM

        UDATASEG

   ; All-round crappy texture tu use. Create with AdjustTexture.
texture DB 8*4 DUP (?)
textureEnd LABEL BYTE

x0 DD ?
x1 DD ?
y0 DD ?
y1 DD ?
w  DD ?
h  DD ?
        .CODE

; ------------------------
; ESI -> Source texture.
; EAX    -> Y destination.

AdjustTexture:
        CLD
        LEA     EDI,texture
        AND     EAX,7
        SHL     EAX,2
        MOV     ECX,8*4
        SUB     ECX,EAX
        PUSH    ESI
        ADD     ESI,EAX
        REP MOVSB
        POP     ESI
        MOV     ECX,EAX
        REP MOVSB
        RET

; ====================================================

;PUBLIC void M12_FillRectangle(int x0, int y0, int x1, int y1, const TTexture t);
;#pragma aux M12_FillRectangle parm [EAX] [EDX] [EBX] [ECX] [ESI]
PUBLIC M12_FillRectangle_
M12_FillRectangle_:
        PUSH EDI

        CMP     EAX,EBX
        JE      @@ret
        JL      SHORT @@c10
         XCHG   EAX,EBX
   @@c10:
        CMP     EAX,XMAX         ; Clip it.
        JGE     @@ret
        CMP     EAX,XMIN
        JGE     @@c11
         MOV    EAX,XMIN
    @@c11:
        CMP     EBX,XMIN
        JLE     @@ret
        CMP     EBX,XMAX
        JLE     @@c12
         MOV    EBX,XMAX
    @@c12:
        CMP     EAX,EBX
        JE      @@ret
        JL      SHORT @@c13
         XCHG   EAX,EBX
   @@c13:
        MOV     x0,EAX
        MOV     x1,EBX

        CMP     EDX,ECX
        JE      @@ret
        JL      SHORT @@c20
         XCHG   EDX,ECX
   @@c20:
        CMP     EDX,YMAX         ; Clip it.
        JGE     @@ret
        CMP     EDX,YMIN
        JGE     SHORT @@c21
         MOV    EDX,YMIN
    @@c21:
        CMP     ECX,YMIN
        JLE     @@ret
        CMP     ECX,YMAX
        JLE     SHORT @@c22
         MOV    ECX,YMAX
    @@c22:
        CMP     EDX,ECX
        JE      @@ret
        JL      SHORT @@c23
         XCHG   EDX,ECX
   @@c23:
        MOV     y0,EDX
        MOV     y1,ECX

        MOV     EAX,y0
        CALL    AdjustTexture

        MOV     EAX,y0
        MOV     EBX,SCRW
        MUL     EBX
        MOV     EDI,x0
        SHR     EDI,3
        LEA     EDI,0A0000h[EDI+EAX]
        
        MOV     EAX,x0
        SHR     EAX,3
        MOV     EBX,x1
        SHR     EBX,3
        SUB     EBX,EAX
        JNZ     @@wide
         ; One-byte rectangle.
        MOV     ECX,x0
        MOV     EAX,0FF00h
        MOV     EDX,EAX
        AND     CL,7
        JZ      SHORT @@c3
        SHR     EAX,CL
    @@c3:
        MOV     ECX,x1
        AND     CL,7
        JZ      SHORT @@c3
        SHR     EDX,CL
    @@c4:
        AND     AH,DL
        JZ      @@ret
        SetBitMask AH
        MOV     ECX,y1
        SUB     ECX,y0
        CALL    FillVerticalMaskedByte
        JMP     @@ret

   @@wide:
        PUSH    EBX              ; #bytes between x0 and x1
        MOV     ECX,x1
        MOV     EAX,0FF00h
        AND     CL,7
        JZ      SHORT @@c5
        SHR     EAX,CL
    @@c5:
        PUSH    EAX
        MOV     ECX,x0
        MOV     EAX,0FF00h
        AND     CL,7
        JZ      SHORT @@c6
        SHR     EAX,CL
    @@c6:
        MOV     ECX,y1
        SUB     ECX,y0
        SetBitMask AH
        CALL    FillVerticalMaskedByte
        POP     EBX
        POP     EAX
        PUSH    EDI EAX
        ADD     EDI,EAX
        OR      BL,BL
        JZ      @@noright
        SetBitMask BL
        CALL    FillVerticalMaskedByte
  @@noright:
        POP     EBX EDI
        DEC     EBX
        JZ      @@ret
        INC     EDI
        SetBitMask 0FFh
        XCHG    ECX,EBX

        MOV     AH,01h
        MOV     ESI,OFFSET texture
    @@l1:
        PUSH    EDI EBX
        SetWritePlanes AH
    @@l2:
        j = 0
        REPT 8
            PUSH    EDI ECX
            MOV     AL,[ESI+j]
            REP STOSB
            POP     ECX EDI
            ADD     EDI,SCRW
            DEC     EBX
            JZ      SHORT @@endit
            j = j + 4
        ENDM
        JMP     SHORT @@l2
   @@endit:
        INC     ESI                      ; Next plane.
        POP     EBX EDI
        ADD     AH,AH
        CMP     AH,10h
        JNZ     @@l1

   @@ret:
        POP     EDI
        RET

FillVerticalMaskedByte:
        ;DI==dest,CX=count. Don't get destroyed.
        MOV     AH,01h
        MOV     BL,0
        MOV     ESI,OFFSET texture
    @@l1:
        PUSH    EDI ECX

        SetWritePlanes AH
        SetReadPlane BL

    @@l2:
        i=0
        j=0
        REPT 8
         JCXZ   @@endit
         MOV    AL,[EDI+i]                      ; Load latches
         MOV    AL,[ESI+j]
         MOV    [EDI+i],AL
         DEC    ECX
         i=i+SCRW
         j=j+4
        ENDM
        ADD     EDI,8*SCRW
        JMP     @@l2
   @@endit:
        INC     ESI                      ; Next plane.
        POP     ECX EDI
        INC     BL
        ADD     AH,AH
        CMP     AH,10h
        JNZ     @@l1
        RET

; ====================================================
;PUBLIC void M12_PrepareTexture(int y, const TTexture t);
;#pragma aux M12_PrepareTexture parm [EAX] [ESI]

PUBLIC M12_PrepareTexture_
M12_PrepareTexture_:
        PUSH    ECX EDI
        CALL    AdjustTexture
        POP     EDI ECX
        RET

; ====================================================
; No clipping here!!! Too hard to do properly, someday I'll try. O:-)

MAXCHARH = 50           ; Limits to the size of fonts.
MAXCHARW = 8
        UDATASEG
letter  DB MAXCHARW*MAXCHARH DUP (?)

        .CODE

;PUBLIC void M12_DrawLetter(int x, int y, const byte *c);
;#pragma aux M12_DrawLetter parm [ECX] [EAX] [EBX]
PUBLIC M12_DrawLetter_
M12_DrawLetter_:
        PUSH    EDI ESI EDX

        MOV     EDX,SCRW
        IMUL    EDX
        MOV     EDI,ECX
        SHR     EDI,3
        LEA     EDI,0A0000h[EDI + EAX]
        AND     ECX,7
        MOV     ESI,OFFSET texture

         ; ES:DI -> screen dest.
         ; SI -> texture.
         ; BX -> letter.
         ; CL -> width.
         ; CH -> height.
        XOR     AH,AH
        MOV     CH,1
    @@l0:
        PUSH    EAX ECX
        SetReadPlane AH
        SetWritePlanes CH

        MOV     DX,3CEh                ; Bitmask.
        MOV     AL,8
        OUT     DX,AL
        INC     EDX
;        MOV     AL,7
;        SUB     AL,CL
        MOV     CH,CL
        MOV     EAX,ECX
        SHL     ECX,16
        MOV     CX,AX
        i = 0
        REPT 8
         MOV    AL,[EBX+i]                ; Read letter data.
         SHR    AL,CL
         ROL    ECX,8
         OUT    DX,AL                     ; Use as a mask.
         MOV    AL,[EDI+SCRW*i]                ; Load latches.
         MOV    AL,[ESI+4*i]                     ; Load texture.
         MOV    [EDI+SCRW*i],AL                ; Put color data.
         XOR    EAX,EAX
         MOV    AH,[EBX+i]                ; Read letter data.
         SHR    EAX,CL
         ROL    ECX,8
         OUT    DX,AL                     ; Use as a mask.
         MOV    AL,[EDI+SCRW*i+1]                ; Load latches.
         MOV    AL,[ESI+4*i]                     ; Load texture.
         MOV    [EDI+SCRW*i+1],AL                ; Put color data.
         i = i + 1
        ENDM

        POP     ECX EAX
        INC     ESI
        SHL     CH,1
        INC     AH
        CMP     AH,4
        JNZ     @@l0

        POP     EDX ESI EDI
        RET

; ====================================================

;PUBLIC void M12_SaveArea(int x0, int y0, int x1, int y1, byte *dest);
;#pragma aux M12_SaveArea parm [ESI] [EAX] [ECX] [EBX] [EDI]
PUBLIC M12_SaveArea_
M12_SaveArea_:
        PUSH    EBP EDX

        CLD
        SUB     EBX,EAX         ; h = y1-y0
        MOV     EBP,SCRW
        MUL     EBP
        SHR     ESI,3
        ADD     ECX,7
        SHR     ECX,3
        SUB     ECX,ESI         ; w = x1 - x0  BYTE ALIGNED
        LEA     ESI,0A0000h[ESI+EAX]

        MOV     EDX,ECX
        MOV     EBP,EBX

        XOR     AH,AH
    @@l1:
        PUSH    ESI EDX
        SetReadPlane AH
        POP     EDX
        MOV     EBX,EBP
    @@l2:
        MOV     ECX,EDX
        REP MOVSB
        SUB     ESI,EDX
        ADD     ESI,SCRW
        DEC     EBX
        JNZ     @@l2
        POP     ESI
        INC     AH
        CMP     AH,4
        JNZ     @@l1

        POP     EDX EBP
        RET

; ====================================================

;PUBLIC void M12_DumpArea(int x0, int y0, int x1, int y1, const byte *from);
;#pragma aux M12_DumpArea parm [EDI] [EAX] [ECX] [EBX] [ESI]
PUBLIC M12_DumpArea_
M12_DumpArea_:
        PUSH    EBP EDX

        CLD
        SUB     EBX,EAX         ; h = y1-y0
        MOV     EBP,SCRW
        MUL     EBP
        SHR     EDI,3
        ADD     ECX,7
        SHR     ECX,3
        SUB     ECX,EDI         ; w = x1 - x0  BYTE ALIGNED
        LEA     EDI,0A0000h[EDI+EAX]

        SetBitMask 0FFh
        MOV     EDX,ECX
        MOV     EBP,EBX

        MOV     AH,1
    @@l1:
        PUSH    EDI EDX
        SetWritePlanes AH
        POP     EDX
        MOV     EBX,EBP
    @@l2:
        MOV     ECX,EDX
        REP MOVSB
        SUB     EDI,EDX
        ADD     EDI,SCRW
        DEC     EBX
        JNZ     @@l2
        POP     EDI
        ADD     AH,AH
        CMP     AH,10h
        JNZ     @@l1

        POP     EDX EBP
        RET

COMMENT #

; ====================================================
; OJO: s¢lo bitmaps de m s de 8 pixels!!!!!!!!!

PUBLIC M12DrawBitmap

scrWidth  DW 0
rightMask DB 0

M12DrawBitmap PROC USES SI DI ES DS, x:WORD,y:WORD,w:WORD,h:WORD,from:DWORD
        MOV     AX,y
        MOV     DI,SCRW
        MUL     DI
        MOV     DI,x
        SHR     DI,3
        ADD     DI,AX
        MOV     AX,0A000h
        MOV     ES,AX
        LDS     SI,from

        MOV     DX,x
        MOV     CX,DX
        ADD     CX,w
        SHR     CX,3
        SHR     DX,3
        SUB     CX,DX
        MOV     CS:[scrWidth],CX

        MOV     CX,x
        MOV     DX,w
        ADD     CX,DX
        AND     CL,7
        MOV     AH,0FFh
        AND     CL,7
        JZ      @@rc1
         SHR     AH,CL
     @@rc1:
        MOV     CS:[rightMask],AH
        MOV     CX,x
        AND     CL,7

        ADD     DX,7
        SHR     DX,3
        MOV     w,DX

        SetBitMask 0FFh
        MOV     AX,100h
    @@l1:
        PUSH    AX DI
        PUSH    AX
        SetWritePlanes AH
        MOV     DX,3CEh         ; Read Plane.
        MOV     AL,4
        OUT     DX,AL
        INC     DX
        POP     AX
        OUT     DX,AL
        MOV     BX,h
    @@l2:

        MOV     CH,BYTE PTR [scrWidth]
        PUSH    SI

        MOV     AL,[SI]
        SHR     AL,CL
        MOV     AH,0FFh
        SHR     AH,CL
        NOT     AH
        AND     AH,ES:[DI]
        OR      AL,AH
        MOV     ES:[DI],AL
        INC     DI

        DEC     CH
        JE      @@c1
    @@l3:
         MOV    AH,[SI]
         MOV    AL,[SI+1]
         SHR    AX,CL
         MOV    ES:[DI],AL
         INC    SI
         INC    DI
         DEC    CH
         JNZ    @@l3
    @@c1:

        MOV     AH,[SI]
        MOV     AL,[SI+1]
        SHR     AX,CL
        MOV     AH,[rightMask]
        NOT     AH
        AND     AL,AH
        NOT     AH
        AND     AH,ES:[DI]
        OR      AL,AH
        MOV     ES:[DI],AL
        POP     SI
        ADD     SI,w

        SUB     DI,[scrWidth]
        ADD     DI,SCRW
        DEC     BX
        JNZ     @@l2

        POP     DI AX
        INC     AL
        ADD     AH,AH
        CMP     AH,10h
        JNZ     @@l1
        RET
ENDP M12DrawBitmap

;#

; ====================================================

drawSpritePlane:
    @@l1:
        PUSH    EAX ECX
        MOV     AH,0FFh         ; Left byte.
        MOV     AL,[EBX]
        SHR     EAX,CL
        MOV     AH,[EDI]
        AND     AH,AL
        MOV     DL,[ESI]
        SHR     DL,CL
        OR      AH,DL
        MOV     [EDI],AH
        INC     EDI

        DEC     CH
        JZ      @@rightb
  @@middbl:
        MOV     AH,[EBX]         ; Middle bytes.
        MOV     AL,[EBX+1]
        SHR     EAX,CL
        MOV     AH,[EDI]
        AND     AH,AL
        MOV     DH,[ESI]
        MOV     DL,[ESI+1]
        SHR     EDX,CL
        OR      AH,DL
        MOV     [EDI],AH
        INC     EDI
        INC     ESI
        INC     EBX
        DEC     CH
        JNZ     SHORT @@middbl

  @@rightb:
        MOV     AL,0FFh         ; Left byte.
        MOV     AH,[EBX]
        SHR     EAX,CL
        MOV     AH,[EDI]
        AND     AH,AL
        MOV     DH,[ESI]
        XOR     DL,DL
        SHR     EDX,CL
        OR      AH,DL
        MOV     [EDI],AH
        INC     EDI
        INC     ESI
        INC     EBX

        POP     ECX EAX
        SUB     EDI,[w]
        ADD     EDI,SCRW-1
        DEC     EAX
        JNZ     @@l1
        RET

zeroSpritePlane:
    @@l1:
        PUSH    EAX ECX
        MOV     AH,0FFh         ; Left byte.
        MOV     AL,[EBX]
        SHR     EAX,CL
        AND     [EDI],AL
        INC     EDI

        DEC     CH
        JZ      SHORT @@rightb
  @@middbl:
        MOV     AH,[EBX]         ; Middle bytes.
        MOV     AL,[EBX+1]
        SHR     EAX,CL
        AND     [EDI],AL
        INC     EDI
        INC     ESI
        INC     EBX
        DEC     CH
        JNZ     @@middbl

  @@rightb:
        MOV     AL,0FFh         ; Left byte.
        MOV     AH,[EBX]
        SHR     EAX,CL
        AND     [EDI],AL
        INC     EDI
        INC     ESI
        INC     EBX

        POP     ECX EAX
        SUB     EDI,[w]
        ADD     EDI,SCRW-1
        DEC     EAX
        JNZ     @@l1
        RET

; ====================================================
COMMENT #
PUBLIC M12DrawSprite
M12DrawSprite PROC USES DS ES DI SI, x:WORD,y:WORD,w:WORD,h:WORD,spr:DWORD
        SetBitMask 0FFh
        MOV     DX,w
        ADD     DX,7
        SHR     DX,3
        MOV     w,DX
        LDS     BX,spr
        MOV     AX,h
        MUL     DX
        MOV     SI,BX
        ADD     SI,AX
        MOV     AX,y
        MOV     DX,SCRW
        MUL     DX
        MOV     CX,x
        MOV     DI,CX
        SHR     DI,3
        ADD     DI,AX
        AND     CL,7
        MOV     CH,BYTE PTR w

        MOV     DX,3C4h
        MOV     AL,2
        OUT     DX,AL

        MOV     AX,0A000h
        MOV     ES,AX

        MOV     AX,100h
   @@l0:
        PUSH    DI BX AX

        PUSH    AX
        SetWritePlanes AH
        MOV     DX,3CEh         ; Read Plane.
        MOV     AL,4
        OUT     DX,AL
        INC     DX
        POP     AX
        OUT     DX,AL

        MOV     AX,[h]
        CALL    drawSpritePlane

        POP     AX BX DI
        INC     AL
        ADD     AH,AH
        CMP     AH,10h
        JNZ     @@l0

        RET
ENDP M12DrawSprite
;#

; ====================================================
;PUBLIC void M12_DrawCursor(int x, int y, int w, int h, const byte *cur);
;#pragma aux M12_DrawCursor parm [ECX] [EDI] [EDX] [EAX] [EBX]
PUBLIC M12_DrawCursor_
M12_DrawCursor_:
        PUSH    ESI

        MOV     [h],EAX
        ADD     EDX,7
        SHR     EDX,3
        MOV     [w],EDX
        MUL     EDX
        MOV     ESI,EBX
        ADD     ESI,EAX
        MOV     EAX,EDI
        MOV     EDX,SCRW
        MUL     EDX
        MOV     EDI,ECX
        SHR     EDI,3
        LEA     EDI,0A0000h[EDI+EAX]
        AND     CL,7

        SetBitMask 0FFh
        MOV     DX,3C4h
        MOV     AL,2
        OUT     DX,AL

        MOV     EAX,0803h
   @@l0:
        PUSH    EDI EBX EAX

        PUSH    EAX
        SetWritePlanes AH
        MOV     DX,3CEh         ; Read Plane.
        MOV     AL,4
        OUT     DX,AL
        INC     EDX
        POP     EAX
        OUT     DX,AL

        MOV     CH,BYTE PTR [w]
        CMP     AL,3
        MOV     EAX,[h]

        JNZ     SHORT @@white
;         CALL   zeroSpritePlane
;         JMP    @@c1
     @@white:

        CALL    drawSpritePlane
     @@c1:

        POP     EAX EBX EDI
        DEC     AL
        SHR     AH,1
        JNZ     @@l0

        POP     ESI
        RET

; ====================================================
; ====================================================
; ====================================================
; ====================================================
; ====================================================
; ====================================================
; ====================================================


END

; ------------------ M12UTILA.ASM ----------------

