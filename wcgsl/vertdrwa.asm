; ----------------------------- VERTDRAW.ASM -------------------
; Bye Jare of Iguana during 30-31/12/1994.
; Draw a vertical span of pixels, stored as:
;  #pixels to skip (transparent).
;  #pixels of data + data itself.
;   ... again, until end.
; Doesn't allow spans greater than 256, so we can use a #pixels == 0 to
; mark end of span (except as the first transparent span).


        .386P
        .MODEL FLAT
        LOCALS @@

REPTLEN =  8

DEBUG = 0

MAXY  = 768

CheckBounds MACRO
        IF DEBUG
                CMP     EDI,0A0000h
                JC      BoundsError
                CMP     EDI,0B0000h
                JNC     BoundsError
        ENDIF
ENDM


;SEGMENT _DATA DWORD PUBLIC USE32 'DATA'
        DATASEG

EXTRN   _DRW_Tile:      BYTE  ; Boolean, != 0 means tile the texture.
EXTRN   _DRW_Skip:      DWORD ; Pixels on screen to skip before starting to draw.
EXTRN   _DRW_Height:    DWORD ; Height of data to actually draw (after clipping).
EXTRN   _DRW_DDAStart:  DWORD ; DDA starting value, 16.16.
EXTRN   _DRW_DDAInc:    DWORD ; DDA increment per pixel on screen, 16.16.
EXTRN   _DRW_DDAIncInv: DWORD ; (1 / DDAInc), 16.16.

EXTRN   _DRW_TranslatePtr: DWORD ; Pointer to palette translation table.

AdditionTable LABEL DWORD
        i = 0
        REPT MAXY
                DD i
                i = i + 320
        ENDM

AdditionTableMX LABEL DWORD
        i = 0
        REPT MAXY
                DD i
                i = i + 80
        ENDM

AdditionTable640 LABEL DWORD
        i = 0
        REPT MAXY
                DD i
                i = i + 640
        ENDM



TextureStart DD 0

;ENDS

;DGROUP GROUP _DATA

;SEGMENT _TEXT DWORD PUBLIC USE32 'CODE'
        ;ASSUME CS:_TEXT,DS:DGROUP

        CODESEG


DefLabel MACRO l, i
l&i:
ENDM

MnLabel MACRO m, l, i
        m       l&i
ENDM

; extern void DRW_DoVerticalDraw(byte *dest, const byte *data);
; #pragma aux DRW_DoVerticalDraw parm [EDI] [ESI]

DoVerticalDraw MACRO

        PUSHAD

        MOV     [TextureStart],ESI      ; Store for texture tiling.
        MOV     EBP,[_DRW_DDAStart]
        MOV     EBX,[_DRW_DDAInc]
        MOV     EDX,[_DRW_Height]
        MOV     ECX,[_DRW_Skip]
        OR      ECX,ECX
        JZ      @@dodraw

         ; Must skip some of the initial data. We know how many pixels
         ; on screen (#pix) will that be, so calc how many pixels of data
         ; (#data) it means. Keep all decimal parts updated losing as little
         ; precision as possible. The skip zone may (will) end in the middle
         ; of a run of data or a run of transparency, so care must be taken
         ; for this.
         ; Possible exit points: end of data, end of skipping while processing
         ; a transparent run, or while processing a data run.

         ; ECX == #pixels to skip.
;        XOR     EDX,EDX
;        MOV     EAX,EBX         ; EDX:EAX = DDAInc.
;        MUL     ECX             ; EDX:EAX == #data to skip << 16
;        ADD     EBP,EAX         ; Keep DDAPos up to date.
        MOV     ECX,EBP         ; Integer part of DDAPos is #data to skip.
        SHR     ECX,16          ; ECX = #data to skip.
        MOVZX   EBP,BP          ; Clean the integer part.

        MOV     EDX,[_DRW_Height]
        XOR     EAX,EAX
  @@doskiptexture:
        CMP     BYTE PTR [ESI],0        ; First run may be empty.
        JZ      @@skipdata
  @@skiptrans:
        MOVZX   EAX,BYTE PTR [ESI]
        TEST    EAX,EAX
        JZ      @@sktreoftex    ; End of texture while skipping transparency.
        SUB     ECX,EAX
        JC      @@sktreofsk     ; End of skipping while skipping transparency.
  @@skipdata:
        MOVZX   EAX,BYTE PTR [ESI+1]
        TEST    EAX,EAX
        JZ      @@skdaeoftex    ; End of texture while skipping data.
        LEA     ESI,[ESI + EAX + 2]
        SUB     ECX,EAX
        JC      @@skdaeofsk     ; End of skipping while skipping data.
;        INC     ESI
        JMP     @@skiptrans

   @@sktreoftex:
   @@skdaeoftex:
         ; Might want to tile the texture vertically? Uncomment.
        CMP     [_DRW_Tile],0
        JZ      @@notile1
         MOV    ESI,[TextureStart]
         JMP    @@doskiptexture
   @@notile1:
        POPAD
        NOP     ; ?
        RET

         ; End of skipping while skipping transparency.
  @@sktreofsk:
        NEG     ECX     ; # of transparent data to "draw".
        JMP     SHORT @@drawtrans

         ; End of skipping while skipping data.
  @@skdaeofsk:
;        ADD     ESI,ECX ; Restore ESI to the first data pixel to draw.
        NEG     ECX     ; #data remaining to "draw".
        JMP     @@getdata

@@dodraw:
        MOVZX   ECX,BYTE PTR [ESI]
        TEST    ECX,ECX                 ; First run may be empty.
        JZ      @@drawdata
  @@drawtrans:
        ; ESI -> ptr to start of data run.
        ; EDI -> ptr to screen.
        ; EDX -> remaining pixels on screen.
        ; EBP,EBX -> DDA stuff.

        SHL     ECX,16
        SUB     ECX,EBP
        JC      @@outoftrans
        LEA     EAX,[ECX+EBX-1]
        MOV     EBP,EDX
        XOR     EDX,EDX
        IDIV    EBX
        SUB     EBP,EAX
        JB      @@totalend
        ADD     EDI,ADDTABLE[EAX*4]
        NEG     EDX
        ADD     EDX,EBX
        XCHG    EDX,EBP

        ; ESI -> ptr to start of data run.
        ; EDI -> ptr to screen.
        ; EDX -> remaining pixels on screen.
        ; EBP,EBX -> DDA stuff.
        ; EAX -> ptr to palette translation table (page aligned, low byte = 0).
        ; The order of the following code is:
        ;   <<Get Texture data, translate it, decide which loop>>
        ;   <<Fast texturing loop>>     -> Doesn't check EBX every time.
        ;   <<Jump table for fast loop>>
        ;   <<Slow texturing loop>>     -> Checks EBX every time.
        ;   <<Jump table for slow loop>>

  @@drawdata:
        INC     ESI
        MOVZX   ECX,BYTE PTR [ESI]      ; Get length of run.
        JCXZ    @@endoftex

        LEA     ESI,[ESI+ECX+1]
  @@getdata:
        XOR     EAX,EAX
        SHLD    EAX,EBP,16
        SUB     EAX,ECX
        JNS     @@outofdata
        SHLD    EDX,EBX,16
        SHLD    EBX,EDX,16
        MOV     ECX,EAX
        MOVZX   EDX,DX
        SHL     EBP,16
        MOV     EAX,[_DRW_TranslatePtr]
        SUB     BX,REPTLEN
        JC      @@slowloop
;        SUB     BL,REPTLEN
 @@fastloop:

GenericFast MACRO i
        MOV     AL,[ESI+ECX]
        MOV     AL,[EAX]

        CheckBounds

        c = 0
        REPT NumCols
         MOV    [EDI+SCRW*i+c],AL
         c = c + 1
        ENDM

        ADD     EBP,EBX
        ADC     ECX,EDX
        MnLabel JC, @@drdafaeofrun, %i  ; Eof run while draw data fast.
ENDM
        i = 0
        REPT REPTLEN
                GenericFast %i
                i = i + 1
        ENDM
        ADD     EDI,[ADDTABLE+4*(REPTLEN)]
        SUB     BX,REPTLEN
        JNC     @@fastloop
 @@slowloop:
        ADD     BX,REPTLEN
        JZ      @@totalend

GenericSlow MACRO i
        MOV     AL,[ESI+ECX]
        MOV     AL,[EAX]

        CheckBounds

        c = 0
        REPT NumCols
         MOV    [EDI+c],AL
         c = c + 1
        ENDM

        ADD     EDI,[ADDTABLE+4]
        DEC     BX
        JZ      @@totalend
        ADD     EBP,EBX
        ADC     ECX,EDX
        JC      @@fixdraw  ; Eof run while draw data slowly.
ENDM
        i = 0
        REPT REPTLEN
                GenericSlow %i
                i = i + 1
        ENDM
         ; Can't reach here. Anyway....
        JMP     @@totalend
@@outoftrans:
        MOV     EBP,ECX
        NEG     EBP
        JMP     @@drawdata
@@outofdata:
        SHL     ECX,16
        SUB     EBP,ECX
        MOVZX   ECX,BYTE PTR [ESI]      ; Get length of run.
        TEST    ECX,ECX
        JNZ     @@drawtrans
        JMP     @@endoftex  

@@fixdraw:
        SHRD    EBP,ECX,16
        MOV     EAX,EBX
        SHRD    EBX,EDX,16
        MOVZX   EDX,AX
        MOVZX   ECX,BYTE PTR [ESI]      ; Get length of run.
        TEST    ECX,ECX
        JNZ     @@drawtrans

   @@endoftex:
         ; Might want to tile the texture vertically? Uncomment.
        CMP     [_DRW_Tile],0
        JZ      @@totalend
         MOV    ESI,[TextureStart]
         JMP    @@dodraw
   @@totalend:
        POPAD
        NOP     ; ?
        RET

GenericFastJump MACRO i
        DefLabel @@drdafaeofrun %i
        ADD     EDI,[ADDTABLE+4*(i+1)]
        ADD     BX,REPTLEN - (i+1)
        JNZ     @@fixdraw
        JMP     @@totalend  
ENDM
        i = 0
        REPT REPTLEN
                GenericFastJump %i
                i = i + 1
        ENDM

        IF DEBUG
        BoundsError:
                INT 3
                POPAD
                NOP     ; ?
                RET
        ENDIF

ENDM



SCRW    = 320
ADDTABLE = AdditionTable

PUBLIC DRW_DoVerticalDraw_
DRW_DoVerticalDraw_:
PUBLIC DRW_DoVerticalDraw1_
DRW_DoVerticalDraw1_:

        NumCols = 1
        DoVerticalDraw

PUBLIC DRW_DoVerticalDraw2_
DRW_DoVerticalDraw2_:

        NumCols = 2
        DoVerticalDraw

PUBLIC DRW_DoVerticalDraw3_
DRW_DoVerticalDraw3_:

        NumCols = 3
        DoVerticalDraw

PUBLIC DRW_DoVerticalDraw4_
DRW_DoVerticalDraw4_:

        NumCols = 4
        DoVerticalDraw

SCRW    = 80
ADDTABLE = AdditionTableMX

PUBLIC DRW_DoVerticalDrawMX_
DRW_DoVerticalDrawMX_:

        NumCols = 1
        DoVerticalDraw

SCRW    = 640
ADDTABLE = AdditionTable640

PUBLIC DRW_DoVerticalDraw640_
DRW_DoVerticalDraw640_:

        NumCols = 1
        DoVerticalDraw

; ------------------------------

DATASEG

BigInc	DD ?

CODESEG


DoTransparentDraw MACRO
        PUSHAD

        MOV     [TextureStart],ESI      ; Store for texture tiling.
        MOV     EBP,[_DRW_DDAStart]
        MOV     EBX,[_DRW_DDAInc]
        MOV     EDX,[_DRW_Height]
        MOV     ECX,[_DRW_Skip]
        OR      ECX,ECX
        JZ      @@dodraw

         ; Must skip some of the initial data. We know how many pixels
         ; on screen (#pix) will that be, so calc how many pixels of data
         ; (#data) it means. Keep all decimal parts updated losing as little
         ; precision as possible. The skip zone may (will) end in the middle
         ; of a run of data or a run of transparency, so care must be taken
         ; for this.
         ; Possible exit points: end of data, end of skipping while processing
         ; a transparent run, or while processing a data run.

         ; ECX == #pixels to skip.
;        XOR     EDX,EDX
;        MOV     EAX,EBX         ; EDX:EAX = DDAInc.
;        MUL     ECX             ; EDX:EAX == #data to skip << 16
;        ADD     EBP,EAX         ; Keep DDAPos up to date.
        MOV     ECX,EBP         ; Integer part of DDAPos is #data to skip.
        SHR     ECX,16          ; ECX = #data to skip.
        MOVZX   EBP,BP          ; Clean the integer part.

        MOV     EDX,[_DRW_Height]
        XOR     EAX,EAX
  @@doskiptexture:
        CMP     BYTE PTR [ESI],0        ; First run may be empty.
        JZ      @@skipdata
  @@skiptrans:
        MOVZX   EAX,BYTE PTR [ESI]
        TEST    EAX,EAX
        JZ      @@sktreoftex    ; End of texture while skipping transparency.
        SUB     ECX,EAX
        JC      @@sktreofsk     ; End of skipping while skipping transparency.
  @@skipdata:
        MOVZX   EAX,BYTE PTR [ESI+1]
        TEST    EAX,EAX
        JZ      @@skdaeoftex    ; End of texture while skipping data.
        LEA     ESI,[ESI + EAX + 2]
        SUB     ECX,EAX
        JC      @@skdaeofsk     ; End of skipping while skipping data.
;        INC     ESI
        JMP     @@skiptrans

   @@sktreoftex:
   @@skdaeoftex:
         ; Might want to tile the texture vertically? Uncomment.
        CMP     [_DRW_Tile],0
        JZ      @@notile1
         MOV    ESI,[TextureStart]
         JMP    @@doskiptexture
   @@notile1:
        POPAD
        NOP     ; ?
        RET

         ; End of skipping while skipping transparency.
  @@sktreofsk:
        NEG     ECX     ; # of transparent data to "draw".
        JMP     SHORT @@drawtrans

         ; End of skipping while skipping data.
  @@skdaeofsk:
;        ADD     ESI,ECX ; Restore ESI to the first data pixel to draw.
        NEG     ECX     ; #data remaining to "draw".
        JMP     @@getdata

@@dodraw:
        MOVZX   ECX,BYTE PTR [ESI]
        TEST    ECX,ECX                 ; First run may be empty.
        JZ      @@drawdata
  @@drawtrans:
        ; ESI -> ptr to start of data run.
        ; EDI -> ptr to screen.
        ; EDX -> remaining pixels on screen.
        ; EBP,EBX -> DDA stuff.

        SHL     ECX,16
        SUB     ECX,EBP
        JC      @@outoftrans
        LEA     EAX,[ECX+EBX-1]
        MOV     EBP,EDX
        XOR     EDX,EDX
        IDIV    EBX
        SUB     EBP,EAX
        JB      @@totalend
        ADD     EDI,ADDTABLE[EAX*4]
        NEG     EDX
        ADD     EDX,EBX
        XCHG    EDX,EBP

        ; ESI -> ptr to start of data run.
        ; EDI -> ptr to screen.
        ; EDX -> remaining pixels on screen.
        ; EBP,EBX -> DDA stuff.
        ; EAX -> ptr to palette translation table (page aligned, low byte = 0).
        ; The order of the following code is:
        ;   <<Get Texture data, translate it, decide which loop>>
        ;   <<Fast texturing loop>>     -> Doesn't check EBX every time.
        ;   <<Jump table for fast loop>>
        ;   <<Slow texturing loop>>     -> Checks EBX every time.
        ;   <<Jump table for slow loop>>

  @@drawdata:
        INC     ESI
        MOVZX   ECX,BYTE PTR [ESI]      ; Get length of run.
        JCXZ    @@endoftex

        LEA     ESI,[ESI+ECX+1]
  @@getdata:
        XOR     EAX,EAX
        SHLD    EAX,EBP,16
        SUB     EAX,ECX
        JNS     @@outofdata
        SHLD    EDX,EBX,16
        SHLD    EBX,EDX,16
        MOV     ECX,EAX
        MOVZX   EDX,DX
        SHL     EBP,16
        MOV     EAX,[_DRW_TranslatePtr]
        SUB     BX,REPTLEN
        MOV	[BigInc],EDX
        MOV	EDX,0
        JC      @@slowloop
;        SUB     BL,REPTLEN
 @@fastloop:

GenericFast MACRO i
        MOV	AL,[EDI+SCRW*i]
        MOV     DH,[ESI+ECX]
        MOV     AL,[EAX+EDX]
        MOV     [EDI+SCRW*i],AL

        ADD     EBP,EBX
        ADC     ECX,[BigInc]
        MnLabel JC, @@drdafaeofrun, %i  ; Eof run while draw data fast.
ENDM
        i = 0
        REPT REPTLEN
                GenericFast %i
                i = i + 1
        ENDM
        ADD     EDI,[ADDTABLE+4*(REPTLEN)]
        SUB     BX,REPTLEN
        JNC     @@fastloop
 @@slowloop:
        ADD     BX,REPTLEN
        JZ      @@totalend

GenericSlow MACRO i
        MOV     DH,[ESI+ECX]
        MOV	AL,[EDI]
        MOV     AL,[EAX+EDX]
        MOV     [EDI],AL
        ADD     EDI,[ADDTABLE+4]
        DEC     BX
        JZ      @@totalend
        ADD     EBP,EBX
        ADC     ECX,[BigInc]
        JC      @@fixdraw  ; Eof run while draw data slowly.
ENDM
        i = 0
        REPT REPTLEN
                GenericSlow %i
                i = i + 1
        ENDM
         ; Can't reach here. Anyway....
        JMP     @@totalend
@@outoftrans:
        MOV     EBP,ECX
        NEG     EBP
        JMP     @@drawdata
@@outofdata:
        SHL     ECX,16
        SUB     EBP,ECX
        MOVZX   ECX,BYTE PTR [ESI]      ; Get length of run.
        TEST    ECX,ECX
        JNZ     @@drawtrans
        JMP     @@endoftex

@@fixdraw:
	MOV	EDX,[BigInc]
        SHRD    EBP,ECX,16
        MOV     EAX,EBX
        SHRD    EBX,EDX,16
        MOVZX   EDX,AX
        MOVZX   ECX,BYTE PTR [ESI]      ; Get length of run.
        TEST    ECX,ECX
        JNZ     @@drawtrans

   @@endoftex:
         ; Might want to tile the texture vertically? Uncomment.
        CMP     [_DRW_Tile],0
        JZ      @@totalend
         MOV    ESI,[TextureStart]
         JMP    @@dodraw
   @@totalend:
        POPAD
        NOP     ; ?
        RET

GenericFastJump MACRO i
        DefLabel @@drdafaeofrun %i
        ADD     EDI,[ADDTABLE+4*(i+1)]
        ADD     BX,REPTLEN - (i+1)
        JNZ     @@fixdraw
        JMP     @@totalend
ENDM
        i = 0
        REPT REPTLEN
                GenericFastJump %i
                i = i + 1
        ENDM

        IF DEBUG
        BoundsError:
                INT 3
                POPAD
                NOP     ; ?
                RET
        ENDIF
ENDM


PUBLIC DRW_DoTransparentDraw_
DRW_DoTransparentDraw_:
SCRW    = 320
ADDTABLE = AdditionTable

        DoTransparentDraw

PUBLIC DRW_DoTransparentDraw640_
DRW_DoTransparentDraw640_:
SCRW    = 640
ADDTABLE = AdditionTable640

        DoTransparentDraw


ENDS

END


; ----------------------------- VERTDRAW.ASM -------------------
