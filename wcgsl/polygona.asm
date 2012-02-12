; ----------------------------- POLYGONA.ASM -------------------
; For use with WATCOM 9.5 + DOS4GW
; (C) Copyright 1993/4 by Jare & JCAB of Iguana.
; Polytracers and polyfillers.

        .386P
        .MODEL FLAT
        LOCALS @@

        ; Upper left corner is (0,0) for simplicity.
ScrXMin = 0
ScrYMin = 0
FULLTEDGESIZE  = 5*2*4
TSB = 6

        DATASEG

PUBLIC _POLY_MinX, _POLY_MinY
PUBLIC _POLY_MaxX, _POLY_MaxY, _POLY_DivTable
_POLY_MinX DD   0
_POLY_MinY DD   0
_POLY_MaxX DD 320
_POLY_MaxY DD 200
_POLY_DivTable DD 0

NScans          DD ?
Texture         DD ?
ScrWidth        DD ?
TextCounter     DD ?

ptx DD ?
ptxcz DD ?
pty DD ?
ptycz DD ?
pz  DD ?
dtx DD ?
dtxm DD ?
dty DD ?
dtym DD ?
dz  DD ?
dzm  DD ?

pl DD ?
dl_ DD ?

        CODESEG

; -------------------------------------------
; Trazado de las coordenadas de pantalla de un lado.

; PUBLIC void POLY_TraceEdge(POLY_TFullEdge *edge,
;                            const POLY_TFullVertex *v0,
;                            const POLY_TFullVertex *v1,
;                            int clipend);
; #pragma aux POLY_TraceEdge parm [EDI] [ESI] [EBX] [EDX]

PUBLIC POLY_TraceEdge_
POLY_TraceEdge_:

        PUSH    EBP EAX ECX

        PUSH    EDX             ; Save clipend

        MOV     ECX,[EBX+4]
        SUB     ECX,[ESI+4]     ; DeltaY
         ; Calc screen deltas.
        MOV     EAX,[EBX]
        SUB     EAX,[ESI]       ; DeltaX
; JMP @@dodiv
        CMP     ECX,64
        JNC     @@dodiv
COMMENT #
         TEST   EAX,EAX
         JNS    @@divpos
         NEG    EAX
         MOV    EDX,ECX
         SHL    EDX,8
         ADD    EDX,[_POLY_DivTable]  ; Already rotated 2 to the right.
         MOV    DL,AL
         MOV    EBP,[EDX*4]
         SHR    EBP,8
         MOV    DL,AH
         ADD    EBP,[EDX*4]
         NEG    EBP
         JMP    @@donediv
    @@divpos:
         MOV    EDX,ECX
         SHL    EDX,8
         ADD    EDX,[_POLY_DivTable]  ; Already rotated 2 to the right.
         MOV    DL,AL
         MOV    EBP,[EDX*4]
         SHR    EBP,8
         MOV    DL,AH
         ADD    EBP,[EDX*4]
         JMP    @@donediv
;#
        ADD     EAX,64
        CMP     EAX,128
        JNC     @@dodivc
        SHL     EAX,6+2
        ADD     EAX,[_POLY_DivTable]
        MOV     EBP,[EAX+4*ECX]
        JMP     @@donediv
  @@dodivc:
        SUB     EAX,64
  @@dodiv:
        SHL     EAX,16
        CDQ
        IDIV    ECX
        MOV     EBP,EAX
  @@donediv:

        MOV     EAX,[ESI+4]     ; y
        MOV     EBX,[ESI]       ; ptx
        SHL     EBX,16
        SUB     EAX,[_POLY_MinY]
        JGE     SHORT @@ins
          ; Clipping: EAX == Coord-Limit = -skip (Limit > Coord)
         PUSH   ECX
         MOV    ECX,EBP
         IMUL   ECX,EAX
         SUB    EBX,ECX         ; ptx += skip*dtx
         POP    ECX
         ADD    ECX,EAX         ; DeltaY -= skip
    @@ins:
         ; EDI -> Edge buffer.
         ; EBP -> DeltaX
         ; EBX -> TX
         ; ECX -> DeltaY.

         ; Now it's time to clip the DeltaY.
         POP    EAX
         SUB    ECX,EAX
         JLE    @@doneedge              ; Shouldn't happen.

EDGETRACEBITS = 5
EDGETRACEREPT = (1 SHL (EDGETRACEBITS))

          ; Add 0.5 of the delta.
         MOV    EAX,EBP
         SAR    EAX,1
         ADD    EBX,EAX

         MOV    EAX,ECX
         ADD    ECX,EDGETRACEREPT-1
         SHR    ECX,EDGETRACEBITS
         AND    EAX,EDGETRACEREPT-1
         SUB    EDI,[@@EdgeTraceAddTbl+4*EAX]
         JMP    [@@EdgeTraceJumpTbl+4*EAX]

EdgeTraceBody MACRO p
@@lde&p:
          MOV   EAX,EBX
          ADD   EBX,EBP
          SAR   EAX,16
          MOV   [EDI+p*FULLTEDGESIZE],EAX
ENDM

EdgeTraceLabel MACRO p
           DD @@lde&p
        ENDM

        ALIGN 4
@@EdgeTraceAddTbl LABEL DWORD
        DD 0
        I = EDGETRACEREPT-1
        REPT EDGETRACEREPT-1
           DD FULLTEDGESIZE*I
           I = I - 1
        ENDM
@@EdgeTraceJumpTbl LABEL DWORD
        DD @@lde0
        I = EDGETRACEREPT-1
        REPT EDGETRACEREPT-1
           EdgeTraceLabel %I
           I = I - 1
        ENDM

    @@lineloop:
          i = 0
          REPT EDGETRACEREPT
             EdgeTraceBody %i
             i = i + 1
          ENDM
          ADD   EDI,FULLTEDGESIZE*EDGETRACEREPT
          DEC   ECX
          JNZ   @@lineloop
    @@doneedge:
        POP     ECX EAX EBP
        RET

; -------------------------------------------
; Trazado de una textura.

; PUBLIC void POLY_TraceTexture(POLY_TFullEdge *edge,
;                               const POLY_TFullVertex *v0,
;                               const POLY_TFullVertex *v1,
;                               int clipend);
; #pragma aux POLY_TraceTexture parm [EDI] [ESI] [EBX] [EDX]

PUBLIC POLY_TraceTexture_
POLY_TraceTexture_:

        PUSH    EBP EAX ECX

        PUSH    EDX             ; Save clipend

        MOV     ECX,[EBX+4]
        SUB     ECX,[ESI+4]     ; DeltaY
         ; Calc texture deltas.
        MOV     EAX,[EBX+8]
        SUB     EAX,[ESI+8]     ; DeltaTX
        CDQ
        IDIV    ECX
        MOV     EBP,EAX
        MOV     EAX,[EBX+12]
        SUB     EAX,[ESI+12]    ; DeltaTY
        CDQ
        IDIV    ECX
        MOV     EDX,EAX

        MOV     EAX,[ESI+4]     ; y
        MOV     EBX,[ESI+8]     ; ptx
        MOV     ESI,[ESI+12]    ; pty
        SUB     EAX,[_POLY_MinY]
        JGE     SHORT @@ins
          ; Upper clipping: EDX == Coord-Limit = -skip (Limit > Coord)
         PUSH   ECX
         MOV    ECX,EBP
         IMUL   ECX,EAX
         SUB    EBX,ECX         ; ptx += skip*dtx
         MOV    ECX,EDX
         IMUL   ECX,EAX
         SUB    ESI,ECX         ; pty += skip*dty
         POP    ECX
         ADD    ECX,EAX         ; DeltaY -= skip
    @@ins:
         ; EDI -> Edge buffer.
         ; EBP -> DeltaTX
         ; EDX -> DeltaTY
         ; EBX -> TX
         ; ESI -> TY
         ; ECX -> DeltaY.

         ; Now it's time to clip the DeltaY.
         POP    EAX
         SUB    ECX,EAX
         JLE    @@doneedge              ; Shouldn't happen.

TEXTURETRACEBITS = 5
TEXTURETRACEREPT = (1 SHL (TEXTURETRACEBITS))

          ; Add 0.5 of the delta.
         MOV    EAX,EBP
         SAR    EAX,1
         ADD    EBX,EAX
         MOV    EAX,EDX
         SAR    EAX,1
         ADD    ESI,EAX

         MOV    EAX,ECX
         ADD    ECX,TEXTURETRACEREPT-1
         SHR    ECX,TEXTURETRACEBITS
         AND    EAX,TEXTURETRACEREPT-1
         SUB    EDI,[@@TextureTraceAddTbl+4*EAX]
         JMP    [@@TextureTraceJumpTbl+4*EAX]

TextureTraceBody MACRO p
@@lde&p:
          MOV   [EDI+p*FULLTEDGESIZE+8],EBX
          ADD   EBX,EBP
          MOV   [EDI+p*FULLTEDGESIZE+16],ESI
          ADD   ESI,EDX
ENDM

TextureTraceLabel MACRO p
           DD @@lde&p
        ENDM

        ALIGN 4
@@TextureTraceAddTbl LABEL DWORD
        DD 0
        I = TEXTURETRACEREPT-1
        REPT TEXTURETRACEREPT-1
           DD FULLTEDGESIZE*I
           I = I - 1
        ENDM
@@TextureTraceJumpTbl LABEL DWORD
        DD @@lde0
        I = TEXTURETRACEREPT-1
        REPT TEXTURETRACEREPT-1
           TextureTraceLabel %I
           I = I - 1
        ENDM

    @@lineloop:
          i = 0
          REPT TEXTURETRACEREPT
             TextureTraceBody %i
             i = i + 1
          ENDM
          ADD   EDI,FULLTEDGESIZE*TEXTURETRACEREPT
          DEC   ECX
          JNZ   @@lineloop
    @@doneedge:
        POP     ECX EAX EBP
        RET

; -------------------------------------------
; Trazado del shading.

; PUBLIC void POLY_TraceShade(POLY_TFullEdge *edge,
;                             const POLY_TFullVertex *v0,
;                             const POLY_TFullVertex *v1,
;                             int clipend);
; #pragma aux POLY_TraceShade parm [EDI] [ESI] [EBX] [EDX]

PUBLIC POLY_TraceShade_
POLY_TraceShade_:

        PUSH    EBP EAX ECX

        PUSH    EDX             ; Save clipend

        MOV     ECX,[EBX+4]
        SUB     ECX,[ESI+4]     ; DeltaY
         ; Calc texture deltas.
        MOV     EAX,[EBX+16]
        SUB     EAX,[ESI+16]    ; DeltaTX
        CDQ
        IDIV    ECX
        MOV     EBP,EAX

        MOV     EAX,[ESI+4]     ; y
        MOV     EBX,[ESI+16]    ; ptx
        SUB     EAX,[_POLY_MinY]
        JGE     SHORT @@ins
          ; Upper clipping: EDX == Coord-Limit = -skip (Limit > Coord)
         MOV    ESI,EBP
         IMUL   ESI,EAX
         SUB    EBX,ESI         ; ptx += skip*dtx
         ADD    ECX,EAX         ; DeltaY -= skip
    @@ins:
         ; EDI -> Edge buffer.
         ; EBP -> DeltaShade
         ; EBX -> Shade
         ; ECX -> DeltaY.

         ; Now it's time to clip the DeltaY.
         POP    EAX
         SUB    ECX,EAX
         JLE    @@doneedge              ; Shouldn't happen.

SHADETRACEBITS = 6
SHADETRACEREPT = (1 SHL (SHADETRACEBITS))

          ; Add 0.5 of the delta.
         MOV    EAX,EBP
         SAR    EAX,1
         ADD    EBX,EAX

         MOV    EAX,ECX
         ADD    ECX,SHADETRACEREPT-1
         SHR    ECX,SHADETRACEBITS
         AND    EAX,SHADETRACEREPT-1
         SUB    EDI,[@@ShadeTraceAddTbl+4*EAX]
         JMP    [@@ShadeTraceJumpTbl+4*EAX]

ShadeTraceBody MACRO p
@@lde&p:
          MOV   [EDI+p*FULLTEDGESIZE+24],EBX
          ADD   EBX,EBP
ENDM

ShadeTraceLabel MACRO p
           DD @@lde&p
        ENDM

        ALIGN 4
@@ShadeTraceAddTbl LABEL DWORD
        DD 0
        I = SHADETRACEREPT-1
        REPT SHADETRACEREPT-1
           DD FULLTEDGESIZE*I
           I = I - 1
        ENDM
@@ShadeTraceJumpTbl LABEL DWORD
        DD @@lde0
        I = SHADETRACEREPT-1
        REPT SHADETRACEREPT-1
           ShadeTraceLabel %I
           I = I - 1
        ENDM

    @@lineloop:
          i = 0
          REPT SHADETRACEREPT
             ShadeTraceBody %i
             i = i + 1
          ENDM
          ADD   EDI,FULLTEDGESIZE*SHADETRACEREPT
          DEC   ECX
          JNZ   @@lineloop
    @@doneedge:
        POP     ECX EAX EBP
        RET

; -------------------------------------------
; Trazado de la Z.

; PUBLIC void POLY_TraceZ(POLY_TFullEdge *edge,
;                         const POLY_TFullVertex *v0,
;                         const POLY_TFullVertex *v1,
;                         int clipend);
; #pragma aux POLY_TraceZ parm [EDI] [ESI] [EBX] [EDX]

PUBLIC POLY_TraceZ_
POLY_TraceZ_:

        PUSH    EBP EAX ECX

        PUSH    EDX             ; Save clipend

        MOV     ECX,[EBX+4]
        SUB     ECX,[ESI+4]     ; DeltaY
         ; Calc texture deltas.
        MOV     EAX,[EBX+28]
        SUB     EAX,[ESI+28]    ; DeltaZ
        CDQ
        IDIV    ECX
        MOV     EBP,EAX

        MOV     EAX,[ESI+4]     ; y
        MOV     EBX,[ESI+28]    ; ptz
        SUB     EAX,[_POLY_MinY]
        JGE     SHORT @@ins
          ; Upper clipping: EDX == Coord-Limit = -skip (Limit > Coord)
         MOV    ESI,EBP
         IMUL   ESI,EAX
         SUB    EBX,ESI         ; ptx += skip*dtx
         ADD    ECX,EAX         ; DeltaY -= skip
    @@ins:
         ; EDI -> Edge buffer.
         ; EBP -> DeltaZ
         ; EBX -> Z
         ; ECX -> DeltaY.

         ; Now it's time to clip the DeltaY.
         POP    EAX
         SUB    ECX,EAX
         JLE    @@doneedge              ; Shouldn't happen.

          ; Add 0.5 of the delta.
         MOV    EAX,EBP
         SAR    EAX,1
         ADD    EBX,EAX

ZTraceBody MACRO p
          MOV   [EDI+p*FULLTEDGESIZE+32],EBX
          ADD   EBX,EBP
ENDM

MINZREPT = 32
	CMP	ECX,MINZREPT
	JC	@@rc1
    @@rl0:
    	i = 0;
    	REPT MINZREPT
		ZTraceBody %i
		i = i + 1
	ENDM
	ADD	EDI,MINZREPT*FULLTEDGESIZE
	SUB	ECX,MINZREPT
	JZ	@@doneedge
	CMP	ECX,MINZREPT
	JNC	@@rl0
    @@rc1:
MINZREPT = 8
	CMP	ECX,MINZREPT
	JC	@@rc2
    @@rl1:
    	i = 0;
    	REPT MINZREPT
		ZTraceBody %i
		i = i + 1
	ENDM
	ADD	EDI,MINZREPT*FULLTEDGESIZE
	SUB	ECX,MINZREPT
	JZ	@@doneedge
	CMP	ECX,MINZREPT
	JNC	@@rl1
    @@rc2:
	ZTraceBody 0
	ADD	EDI,FULLTEDGESIZE
	DEC	ECX
	JNZ	@@rc2
    @@doneedge:
        POP     ECX EAX EBP
        RET

; ==========================================================
; Polyfillers.
; ==========================================================

; PUBLIC void POLY_SolidDump(byte *dest, const POLY_TFullEdge *edge, int nscans, int width, int color);
; #pragma aux POLY_SolidDump parm [EDI] [ESI] [ECX] [EDX] [EAX]

PUBLIC POLY_SolidDump_
POLY_SolidDump_:
        CLD
        PUSH    EBP EBX
        MOV     AH,AL
        MOV     EBP,EAX
        SHL     EAX,16
        MOV     AX,BP
        MOV     [NScans],ECX

         ; Two loops here: One when there's horizontal clipping (left or right)
         ; and the other that is faster when there isn't need to clip.

   @@dlclip:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MinX]
        JGE     @@nlc
    @@cl1:                      ; There's left clipping, so we will stay.
         MOV    EBX,[_POLY_MinX]
         MOV    ECX,[ESI+4]
         CMP    ECX,[_POLY_MaxX]
         JL     @@nrc
         JMP    @@cl2
    @@nlc:                      ; There wasn't left clipping.
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JLE     @@ncl1          ; No right clipping: change loop.
    @@cl2:
         MOV    ECX,[_POLY_MaxX]
    @@nrc:
        LEA     EBP,[EDI+EDX]
        SUB     ECX,EBX
        JLE     @@nxcl
        ADD     EDI,EBX
        MOV     EBX,ECX

        TEST    EDI,3
        JZ      @@clgo
        MOV     ECX,4
        SUB     ECX,EDI
        AND     ECX,3
        CMP     EBX,ECX
        JL      @@clgo            ; Less than 3 pixels.. don't matter how.
        SUB     EBX,ECX
        REP     STOSB
        MOV     ECX,EBX
    @@clgo:
        SHR     ECX,2
        REP STOSD
        MOV     ECX,EBX
        AND     ECX,3
        REP STOSB

    @@nxcl:
        MOV     EDI,EBP
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dlclip
        POP     EBX EBP
        RET

   @@dlnoclip:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MinX]
        JS      @@cl1
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JG      @@cl2
    @@ncl1:
        LEA     EBP,[EDI+EDX]
        SUB     ECX,EBX
        JLE     @@nxnc
        ADD     EDI,EBX
        MOV     EBX,ECX
        TEST    EDI,3
        JZ      @@nclgo
        MOV     ECX,4
        SUB     ECX,EDI
        AND     ECX,3
        CMP     EBX,ECX
        JL      @@nclgo         ; Less than 3 pixels.. don't matter how.
        SUB     EBX,ECX
        REP     STOSB
        MOV     ECX,EBX
    @@nclgo:
        SHR     ECX,2
        REP STOSD
        MOV     ECX,EBX
        AND     ECX,3
        REP STOSB

    @@nxnc:
        MOV     EDI,EBP
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dlnoclip
        POP     EBX EBP
        RET

; PUBLIC void POLY_GouraudDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width);
; #pragma aux POLY_GouraudDump parm [EDI] [ESI] [ECX] [EDX]

PUBLIC POLY_GouraudDump_
POLY_GouraudDump_:
        CLD
        PUSH    EBP
        MOV     [NScans],ECX
        MOV     [ScrWidth],EDX

   @@dl:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MaxX]
        JGE     @@nx
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MinX]
        JLE     @@nx
        SUB     ECX,EBX         ; Length of run.
        JLE     @@nx
        MOV     EAX,[ESI+28]    ; l1
        SUB     EAX,[ESI+24]    ; l1-l0
        CDQ
        IDIV    ECX             ; l delta per pixel.
        MOV     [dl_],EAX

        CMP     EBX,[_POLY_MinX]
        JGE     @@nlc
         ; Left clipping. Advance texture values by appropiate amnt.
         SUB    EBX,[_POLY_MinX]
         NEG    EBX             ; EBX < 0  so make it positive.
         MOV    EAX,[ESI+28]    ; l1
         SUB    EAX,[ESI+24]    ; l1-l0
         CDQ
         IMUL   EBX
         IDIV   ECX             ; l delta per pixel.
         ADD    [ESI+24],EAX
         MOV    EBX,[_POLY_MinX]
    @@nlc:
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JLE     @@nrc
          ; Right clipping: simply cut the scan right edge.
         MOV    ECX,[_POLY_MaxX]
    @@nrc:
        SUB     ECX,EBX
        JLE     @@nx
        PUSH    EDI ESI
        ADD     EDI,EBX

        ; Draw the scan. Register setup by now:
        ; ECX = #pixels to draw.
        ; EDI = screen ptr.
        ; ESI = edge info ptr.

GOURDUMPBITS = 5
GOURDUMPREPT = (1 SHL (GOURDUMPBITS))

         MOV    EAX,ECX
         ADD    ECX,GOURDUMPREPT-1
         SHR    ECX,GOURDUMPBITS
         MOV    EDX,ECX
         MOV    EBP,[ESI+24]          ; pl?
         MOV    EBX,EBP
         SHR    EBX,8
         MOV    ESI,[dl_]
         SHLD   ECX,ESI,16
         SHL    ESI,16
         SHL    EBP,16
         AND    EAX,GOURDUMPREPT-1
         SUB    EDI,[@@GoDrawAddTbl+4*EAX]
         JMP    [@@GoDrawJumpTbl+4*EAX]

GoDrawBody MACRO p
@@jtdb&p:
        ADD     EBP,ESI
        ADC     BH,CL
        MOV     [EDI+p],BH
ENDM

GoDrawLabel MACRO p
           DD @@jtdb&p
        ENDM

        ALIGN 4
@@GoDrawAddTbl LABEL DWORD
        DD 0
        I = GOURDUMPREPT-1
        REPT GOURDUMPREPT-1
           DD I
           I = I - 1
        ENDM
@@GoDrawJumpTbl LABEL DWORD
        DD @@jtdb0
        I = GOURDUMPREPT-1
        REPT GOURDUMPREPT-1
           GoDrawLabel %I
           I = I - 1
        ENDM

    @@textdrawl:
          i = 0
          REPT GOURDUMPREPT
             GoDrawBody %i
             i = i + 1
          ENDM
          ADD   EDI,GOURDUMPREPT
          DEC   EDX
          JNZ   @@textdrawl

        POP     ESI EDI

    @@nx:
        ADD     EDI,[ScrWidth]
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dl
        POP     EBP
        RET

; -------------------------------------------

; PUBLIC void POLY_ShadeDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, int color, const byte *ltable);
; #pragma aux POLY_ShadeDump parm [EDI] [ESI] [ECX] [EDX] [EBX] [EAX]

PUBLIC POLY_ShadeDump_
POLY_ShadeDump_:
        CLD
        PUSH    EBP
        MOV     [NScans],ECX
        MOV     [ScrWidth],EDX
        MOVZX   EBX,BL
        ADD     EAX,EBX
        MOV     [Texture],EAX

   @@dl:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MaxX]
        JGE     @@nx
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MinX]
        JLE     @@nx
        SUB     ECX,EBX         ; Length of run.
        JLE     @@nx
        MOV     EAX,[ESI+28]    ; l1
        SUB     EAX,[ESI+24]    ; l1-l0
        CDQ
        IDIV    ECX             ; l delta per pixel.
        MOV     [dl_],EAX

        CMP     EBX,[_POLY_MinX]
        JGE     @@nlc
         ; Left clipping. Advance texture values by appropiate amnt.
         SUB    EBX,[_POLY_MinX]
         NEG    EBX             ; EBX < 0  so make it positive.
         MOV    EAX,[ESI+28]    ; l1
         SUB    EAX,[ESI+24]    ; l1-l0
         CDQ
         IMUL   EBX
         IDIV   ECX             ; l delta per pixel.
         ADD    [ESI+24],EAX
         MOV    EBX,[_POLY_MinX]
    @@nlc:
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JLE     @@nrc
          ; Right clipping: simply cut the scan right edge.
         MOV    ECX,[_POLY_MaxX]
    @@nrc:
        SUB     ECX,EBX
        JLE     @@nx
        PUSH    EDI ESI
        ADD     EDI,EBX

        ; Draw the scan. Register setup by now:
        ; ECX = #pixels to draw.
        ; EDI = screen ptr.
        ; ESI = edge info ptr.

         MOV    EDX,[Texture]

SHADEDUMPBITS = 5
SHADEDUMPREPT = (1 SHL (SHADEDUMPBITS))

         MOV    EAX,ECX
         ADD    ECX,SHADEDUMPREPT-1
         SHR    ECX,SHADEDUMPBITS
         MOV    [TextCounter],ECX
         MOV    EBP,[ESI+24]          ; pl?
         MOV    EBX,EBP
         SHR    EBX,8
         XOR    BL,BL
         MOV    ESI,[dl_]
         SHLD   ECX,ESI,16
         SHL    ESI,16
         SHL    EBP,16
         AND    EAX,SHADEDUMPREPT-1
         SUB    EDI,[@@ShDrawAddTbl+4*EAX]
         JMP    [@@ShDrawJumpTbl+4*EAX]

ShDrawBody MACRO p
@@jtdb&p:
        ADD     EBP,ESI
        ADC     BH,CL
        MOV     AL,[EDX+EBX]
        MOV     [EDI+p],AL
ENDM

ShDrawLabel MACRO p
           DD @@jtdb&p
        ENDM

        ALIGN 4
@@ShDrawAddTbl LABEL DWORD
        DD 0
        I = SHADEDUMPREPT-1
        REPT SHADEDUMPREPT-1
           DD I
           I = I - 1
        ENDM
@@ShDrawJumpTbl LABEL DWORD
        DD @@jtdb0
        I = SHADEDUMPREPT-1
        REPT SHADEDUMPREPT-1
           ShDrawLabel %I
           I = I - 1
        ENDM

    @@textdrawl:
          i = 0
          REPT SHADEDUMPREPT
             ShDrawBody %i
             i = i + 1
          ENDM
          ADD   EDI,SHADEDUMPREPT
          DEC   [TextCounter]
          JNZ   @@textdrawl

        POP     ESI EDI

    @@nx:
        ADD     EDI,[ScrWidth]
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dl
        POP     EBP
        RET


; PUBLIC void POLY_TextureDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture);
; #pragma aux POLY_TextureDump parm [EDI] [ESI] [ECX] [EDX] [EBX]

PUBLIC POLY_TextureDump_
POLY_TextureDump_:
        CLD
        PUSH    EBP
        MOV     [NScans],ECX
        MOV     [Texture],EBX
        MOV     [ScrWidth],EDX

   @@dl:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MaxX]
        JGE     @@nx
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MinX]
        JLE     @@nx
        SUB     ECX,EBX         ; Length of run.
        JLE     @@nx
        MOV     EAX,[ESI+12]    ; tx1
        SUB     EAX,[ESI+8]     ; tx1-tx0
        CDQ
        IDIV    ECX             ; tx delta per pixel.
        MOV     [dtx],EAX
        MOV     EAX,[ESI+20]    ; ty1
        SUB     EAX,[ESI+16]    ; ty1-ty0
        CDQ
        IDIV    ECX             ; ty delta per pixel.
        MOV     [dty],EAX

        CMP     EBX,[_POLY_MinX]
        JGE     @@nlc
         ; Left clipping. Advance texture values by appropiate amnt.
         SUB    EBX,[_POLY_MinX]
         NEG    EBX             ; EBX < 0  so make it positive.
         MOV    EAX,[ESI+12]    ; tx1
         SUB    EAX,[ESI+8]     ; tx1-tx0
         IMUL   EBX
         IDIV   ECX             ; tx delta per pixel.
         ADD    [ESI+8],EAX
         MOV    EAX,[ESI+20]    ; ty1
         SUB    EAX,[ESI+16]    ; ty1-ty0
         IMUL   EBX
         IDIV   ECX             ; ty delta per pixel.
         ADD    [ESI+16],EAX
         MOV    EBX,[_POLY_MinX]
    @@nlc:
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JLE     @@nrc
          ; Right clipping: simply cut the scan right edge.
         MOV    ECX,[_POLY_MaxX]
    @@nrc:
        SUB     ECX,EBX
        JLE     @@nx
        PUSH    EDI ESI
        ADD     EDI,EBX

        ; Draw the scan. Register setup by now:
        ; ECX = #pixels to draw.
        ; EDI = screen ptr.
        ; ESI = edge info ptr.

          ; 0.5 advance
         MOV    EAX,[dtx]
         SAR    EAX,1
         ADD    EAX,[ESI+8]
         MOV    [ptx],EAX
         MOV    EAX,[dty]
         SAR    EAX,1
         ADD    EAX,[ESI+16]
         MOV    [pty],EAX

         MOV    EBX,[dty]
         SHL    EBX,8
         MOV    BX,WORD PTR [dtx+1]
         MOV    EBP,[ESI+16]
         SHL    EBP,8
         MOV    BP,WORD PTR [ESI+8+1]

         MOV    EDX,[Texture]

TEXTUREDUMPEBITS = 5
TEXTUREDUMPREPT  = (1 SHL (TEXTUREDUMPEBITS))

TextDrawBody MACRO p
        ; Texture loop: This uses EAX,EBP,EBX,EDX and EDI. ESI and ECX are free.
        ; TSB is the number of bits for each texture coordinate.
        MOV   EAX,EBP	   ;1
        SHR   EAX,32-TSB   ;2/1	; This clears high word of EAX. Keep it clean.
        SHLD  AX,BP,TSB	   ;4/6
        ADD   EBP,EBX	   ;1/
        MOV   AL,[EDX+EAX] ;1/
        MOV   [EDI+p],AL   ;2/2
        	           ;11/12

ENDM

    @@biglp:
         CMP    ECX,TEXTUREDUMPREPT
         JC     @@litlp
         MOV    EAX,[dtx]                       ; Calc adjusted values.
         SHL    EAX,TEXTUREDUMPEBITS
         ADD    [ptx],EAX
         MOV    EAX,[dty]
         SHL    EAX,TEXTUREDUMPEBITS
         ADD    [pty],EAX
         I = 0
         REPT TEXTUREDUMPREPT
                TextDrawBody %I
                I = I + 1
         ENDM
         SUB    ECX,TEXTUREDUMPREPT
         JZ     @@eofdraw
         ADD    EDI,TEXTUREDUMPREPT
         MOV    EBP,[pty]                       ; Adjust from calced values.
         SHL    EBP,8
         MOV    BP,WORD PTR [ptx+1]
         JMP    @@biglp
  @@litlp:
         CMP    ECX,8
         JC     @@minilp
         I = 0
         REPT 8
                TextDrawBody %I
                I = I + 1
         ENDM
         SUB    ECX,8
         JZ     @@eofdraw
         ADD    EDI,8
         JMP    @@litlp
  @@minilp:
         I = 0
         REPT 8-1-1
                TextDrawBody %I
                DEC     ECX
                JZ      @@eofdraw
                I = I + 1
         ENDM
         TextDrawBody %I
    @@eofdraw:
        POP     ESI EDI

    @@nx:
        ADD     EDI,[ScrWidth]
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dl
        POP     EBP
        RET



; PUBLIC void POLY_TextureDump256(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture);
; #pragma aux POLY_TextureDump256 parm [EDI] [ESI] [ECX] [EDX] [EBX]

PUBLIC POLY_TextureDump256_
POLY_TextureDump256_:
        CLD
        PUSH    EBP
        MOV     [NScans],ECX
        MOV     [Texture],EBX
        MOV     [ScrWidth],EDX

   @@dl:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MaxX]
        JGE     @@nx
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MinX]
        JLE     @@nx
        SUB     ECX,EBX         ; Length of run.
        JLE     @@nx
        MOV     EAX,[ESI+12]    ; tx1
        SUB     EAX,[ESI+8]     ; tx1-tx0
        CDQ
        IDIV    ECX             ; tx delta per pixel.
        MOV     [dtx],EAX
        MOV     EAX,[ESI+20]    ; ty1
        SUB     EAX,[ESI+16]    ; ty1-ty0
        CDQ
        IDIV    ECX             ; ty delta per pixel.
        MOV     [dty],EAX

        CMP     EBX,[_POLY_MinX]
        JGE     @@nlc
         ; Left clipping. Advance texture values by appropiate amnt.
         SUB    EBX,[_POLY_MinX]
         NEG    EBX             ; EBX < 0  so make it positive.
         MOV    EAX,[ESI+12]    ; tx1
         SUB    EAX,[ESI+8]     ; tx1-tx0
         IMUL   EBX
         IDIV   ECX             ; tx delta per pixel.
         ADD    [ESI+8],EAX
         MOV    EAX,[ESI+20]    ; ty1
         SUB    EAX,[ESI+16]    ; ty1-ty0
         IMUL   EBX
         IDIV   ECX             ; ty delta per pixel.
         ADD    [ESI+16],EAX
         MOV    EBX,[_POLY_MinX]
    @@nlc:
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JLE     @@nrc
          ; Right clipping: simply cut the scan right edge.
         MOV    ECX,[_POLY_MaxX]
    @@nrc:
        SUB     ECX,EBX
        JLE     @@nx
        PUSH    EDI ESI
        ADD     EDI,EBX

        ; Draw the scan. Register setup by now:
        ; ECX = #pixels to draw.
        ; EDI = screen ptr.
        ; ESI = edge info ptr.

          ; 0.5 advance
         MOV    EAX,[dtx]
         SAR    EAX,1
         ADD    EAX,[ESI+8]
         MOV    [ptx],EAX
         MOV    EAX,[dty]
         SAR    EAX,1
         ADD    EAX,[ESI+16]
         MOV    [pty],EAX

         MOV    EBX,[dty]
         SHL    EBX,8
         MOV    BX,WORD PTR [dtx+1]
         MOV    EBP,[ESI+16]
         SHL    EBP,8
         MOV    BP,WORD PTR [ESI+8+1]

         MOV    EDX,[Texture]

TEXTUREDUMPEBITS = 5
TEXTUREDUMPREPT  = (1 SHL (TEXTUREDUMPEBITS))

TextDrawBody MACRO p
        ; Texture loop: This uses EAX,EBP,EBX,EDX and EDI. ESI and ECX are free.
        ; TSB is the number of bits for each texture coordinate.
        MOV   EAX,EBP	   ;1
        SHR   EAX,32-TSB   ;2/1	; This clears high word of EAX. Keep it clean.
        SHLD  AX,BP,8 	   ;4/6
        ADD   EBP,EBX	   ;1/
        MOV   AL,[EDX+EAX] ;1/
        MOV   [EDI+p],AL   ;2/2
        	           ;11/12

ENDM

    @@biglp:
         CMP    ECX,TEXTUREDUMPREPT
         JC     @@litlp
         MOV    EAX,[dtx]                       ; Calc adjusted values.
         SHL    EAX,TEXTUREDUMPEBITS
         ADD    [ptx],EAX
         MOV    EAX,[dty]
         SHL    EAX,TEXTUREDUMPEBITS
         ADD    [pty],EAX
         I = 0
         REPT TEXTUREDUMPREPT
                TextDrawBody %I
                I = I + 1
         ENDM
         SUB    ECX,TEXTUREDUMPREPT
         JZ     @@eofdraw
         ADD    EDI,TEXTUREDUMPREPT
         MOV    EBP,[pty]                       ; Adjust from calced values.
         SHL    EBP,8
         MOV    BP,WORD PTR [ptx+1]
         JMP    @@biglp
  @@litlp:
         CMP    ECX,8
         JC     @@minilp
         I = 0
         REPT 8
                TextDrawBody %I
                I = I + 1
         ENDM
         SUB    ECX,8
         JZ     @@eofdraw
         ADD    EDI,8
         JMP    @@litlp
  @@minilp:
         I = 0
         REPT 8-1-1
                TextDrawBody %I
                DEC     ECX
                JZ      @@eofdraw
                I = I + 1
         ENDM
         TextDrawBody %I
    @@eofdraw:
        POP     ESI EDI

    @@nx:
        ADD     EDI,[ScrWidth]
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dl
        POP     EBP
        RET

; -------------------------------------------

; PUBLIC void POLY_HoleTexDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture);
; #pragma aux POLY_HoleTexDump parm [EDI] [ESI] [ECX] [EDX] [EBX]

PUBLIC POLY_HoleTexDump_
POLY_HoleTexDump_:
        CLD
        PUSH    EBP
        MOV     [NScans],ECX
        MOV     [Texture],EBX
        MOV     [ScrWidth],EDX

   @@dl:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MaxX]
        JGE     @@nx
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MinX]
        JLE     @@nx
        SUB     ECX,EBX         ; Length of run.
        JLE     @@nx
        MOV     EAX,[ESI+12]    ; tx1
        SUB     EAX,[ESI+8]     ; tx1-tx0
        CDQ
        IDIV    ECX             ; tx delta per pixel.
        MOV     [dtx],EAX
        MOV     EAX,[ESI+20]    ; ty1
        SUB     EAX,[ESI+16]    ; ty1-ty0
        CDQ
        IDIV    ECX             ; ty delta per pixel.
        MOV     [dty],EAX

        CMP     EBX,[_POLY_MinX]
        JGE     @@nlc
         ; Left clipping. Advance texture values by appropiate amnt.
         SUB    EBX,[_POLY_MinX]
         NEG    EBX             ; EBX < 0  so make it positive.
         MOV    EAX,[ESI+12]    ; tx1
         SUB    EAX,[ESI+8]     ; tx1-tx0
         IMUL   EBX
         IDIV   ECX             ; tx delta per pixel.
         ADD    [ESI+8],EAX
         MOV    EAX,[ESI+20]    ; ty1
         SUB    EAX,[ESI+16]    ; ty1-ty0
         IMUL   EBX
         IDIV   ECX             ; ty delta per pixel.
         ADD    [ESI+16],EAX
         MOV    EBX,[_POLY_MinX]
    @@nlc:
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JLE     @@nrc
          ; Right clipping: simply cut the scan right edge.
         MOV    ECX,[_POLY_MaxX]
    @@nrc:
        SUB     ECX,EBX
        JLE     @@nx
        PUSH    EDI ESI
        ADD     EDI,EBX

        ; Draw the scan. Register setup by now:
        ; ECX = #pixels to draw.
        ; EDI = screen ptr.
        ; ESI = edge info ptr.

          ; 0.5 advance
         MOV    EAX,[dtx]
         SAR    EAX,1
         ADD    EAX,[ESI+8]
         MOV    [ptx],EAX
         MOV    EAX,[dty]
         SAR    EAX,1
         ADD    EAX,[ESI+16]
         MOV    [pty],EAX

         MOV    EBX,[dty]
         SHL    EBX,8
         MOV    BX,WORD PTR [dtx+1]
         MOV    EBP,[ESI+16]
         SHL    EBP,8
         MOV    BP,WORD PTR [ESI+8+1]

         MOV    EDX,[Texture]

HOLETEXDUMPEBITS = 5
HOLETEXDUMPREPT  = (1 SHL (HOLETEXDUMPEBITS))

HoleTexDrawBody MACRO p
    LOCAL @@c1
        ; TSB is the number of bits for each texture coordinate.
        MOV	EAX,EBP
        SHR	EAX,32-TSB ; This clears high word of EAX. Keep it clean.
        SHLD	AX,BP,TSB
        ADD	EBP,EBX
        MOV	AL,[EDX+EAX]
        TEST	AL,AL
        JZ	@@c1
         MOV	[EDI+p],AL
    @@c1:
ENDM

    @@biglp:
         CMP    ECX,HOLETEXDUMPREPT
         JC     @@litlp
         MOV    EAX,[dtx]                       ; Calc adjusted values.
         SHL    EAX,HOLETEXDUMPEBITS
         ADD    [ptx],EAX
         MOV    EAX,[dty]
         SHL    EAX,HOLETEXDUMPEBITS
         ADD    [pty],EAX
         I = 0
         REPT HOLETEXDUMPREPT
                HoleTexDrawBody %I
                I = I + 1
         ENDM
         SUB    ECX,HOLETEXDUMPREPT
         JZ     @@eofdraw
         ADD    EDI,HOLETEXDUMPREPT
         MOV    EBP,[pty]                       ; Adjust from calced values.
         SHL    EBP,8
         MOV    BP,WORD PTR [ptx+1]
         JMP    @@biglp
  @@litlp:
         CMP    ECX,8
         JC     @@minilp
         I = 0
         REPT 8
                HoleTexDrawBody %I
                I = I + 1
         ENDM
         SUB    ECX,8
         JZ     @@eofdraw
         ADD    EDI,8
         JMP    @@litlp
  @@minilp:
         I = 0
         REPT 8-1-1
                HoleTexDrawBody %I
                DEC     ECX
                JZ      @@eofdraw
                I = I + 1
         ENDM
         HoleTexDrawBody %I
    @@eofdraw:
        POP     ESI EDI

    @@nx:
        ADD     EDI,[ScrWidth]
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dl
        POP     EBP
        RET

; -------------------------------------------

; PUBLIC void POLY_ShadeTexDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture, word ShadeTableSel);
; #pragma aux POLY_ShadeTexDump parm [EDI] [ESI] [ECX] [EDX] [EBX] [EAX]

        DATASEG

RecalcDeltas DD 0

        CODESEG

PUBLIC POLY_ShadeTexDump_
POLY_ShadeTexDump_:
        CLD
        PUSH    EBP FS
        MOV     [NScans],ECX
        MOV     [Texture],EBX
        MOV     [ScrWidth],EDX
        MOV     FS,AX
COMMENT #
        MOV     [RecalcDeltas],1
        CMP     ECX,6
        JC      @@dl
; JMP @@dl

        MOV     EBX,[ESI+4*FULLTEDGESIZE]
        MOV     ECX,[ESI+4*FULLTEDGESIZE+4]
        SUB     ECX,EBX         ; Length of run.
        JLE     @@dl
        MOV     EAX,[ESI+4*FULLTEDGESIZE+12]    ; tx1
        SUB     EAX,[ESI+4*FULLTEDGESIZE+8]     ; tx1-tx0
        CDQ
        IDIV    ECX             ; tx delta per pixel.
        MOV     [dtx],EAX
        MOV     EAX,[ESI+4*FULLTEDGESIZE+20]    ; ty1
        SUB     EAX,[ESI+4*FULLTEDGESIZE+16]    ; ty1-ty0
        CDQ
        IDIV    ECX             ; ty delta per pixel.
        MOV     [dty],EAX
        MOV     [RecalcDeltas],0
;#
   @@dl:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MaxX]
        JGE     @@nx
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MinX]
        JLE     @@nx
        SUB     ECX,EBX         ; Length of run.
        JLE     @@nx
;        CMP     [RecalcDeltas],0
;        JZ      @@okdeltas
        MOV     EAX,[ESI+12]    ; tx1
        SUB     EAX,[ESI+8]     ; tx1-tx0
        CDQ
        IDIV    ECX             ; tx delta per pixel.
        MOV     [dtx],EAX
        MOV     EAX,[ESI+20]    ; ty1
        SUB     EAX,[ESI+16]    ; ty1-ty0
        CDQ
        IDIV    ECX             ; ty delta per pixel.
        MOV     [dty],EAX
   @@okdeltas:
        MOV     EAX,[ESI+28]    ; l1
        SUB     EAX,[ESI+24]    ; l1-l0
        CDQ
        IDIV    ECX             ; l delta per pixel.
        MOV     [dl_],EAX

        CMP     EBX,[_POLY_MinX]
        JGE     @@nlc
         ; Left clipping. Advance texture values by appropiate amnt.
         SUB    EBX,[_POLY_MinX]
         NEG    EBX             ; EBX < 0  so make it positive.
         MOV    EAX,[ESI+12]    ; tx1
         SUB    EAX,[ESI+8]     ; tx1-tx0
         CDQ
         IMUL   EBX
         IDIV   ECX             ; tx delta per pixel.
         ADD    [ESI+8],EAX
         MOV    EAX,[ESI+20]    ; ty1
         SUB    EAX,[ESI+16]    ; ty1-ty0
         CDQ
         IMUL   EBX
         IDIV   ECX             ; ty delta per pixel.
         ADD    [ESI+16],EAX
         MOV    EAX,[ESI+28]    ; l1
         SUB    EAX,[ESI+24]    ; l1-l0
         CDQ
         IMUL   EBX
         IDIV   ECX             ; l delta per pixel.
         ADD    [ESI+24],EAX
         MOV    EBX,[_POLY_MinX]
    @@nlc:
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JLE     @@nrc
          ; Right clipping: simply cut the scan right edge.
         MOV    ECX,[_POLY_MaxX]
    @@nrc:
        SUB     ECX,EBX
        JLE     @@nx
        PUSH    EDI ESI
        ADD     EDI,EBX

        ; Draw the scan. Register setup by now:
        ; ECX = #pixels to draw.
        ; EDI = screen ptr.
        ; ESI = edge info ptr.

          ; 0.5 advance
         MOV    EAX,[dtx]
         SAR    EAX,1
         ADD    EAX,[ESI+8]
         MOV    [ptx],EAX
         MOV    EAX,[dty]
         SAR    EAX,1
         ADD    EAX,[ESI+16]
         MOV    [pty],EAX
         MOV    EAX,[dl_]
         SAR    EAX,1
         ADD    EAX,[ESI+24]
         MOV    [pl],EAX

         MOV    EBX,[dty]
         SHL    EBX,8
         MOV    BX,WORD PTR [dtx+1]
         MOV    EBP,[pty]              ;[ESI+16]
         SHL    EBP,8
         MOV    BP,WORD PTR [ptx+1]    ; [ESI+8+1]

         MOV    EDX,[Texture]

SHADETEXDUMPBITS = 5
SHADETEXDUMPREPT = (1 SHL (SHADETEXDUMPBITS))
ShTextDrawBody MACRO p
        ; Texture loop: This uses EAX,EBP,EBX,EDX and EDI. ESI and ECX are free.
        ; TSB is the number of bits for each texture coordinate.
        MOV   EAX,EBP
        SHR   EAX,32-TSB ; This clears high word of EAX. Keep it clean.
        SHLD  AX,BP,TSB
        ADD   EBP,EBX
        MOV   AL,[EDX+EAX]
        MOV   AH,CH
        ADD   ECX,ESI
        MOV   AL,FS:[EAX]
        MOV   [EDI+p],AL
ENDM

         MOV    [TextCounter],ECX
         MOVSX  ECX,WORD PTR [pl+1]          ; pl?
         MOVSX  ESI,WORD PTR [dl_+1]
         TEST   ESI,ESI
         JNS    @@dlp           ; delta < 0?
          INC   ESI             ; Round to zero, not to bottom
  @@dlp:
    @@biglp:
         CMP    [TextCounter],SHADETEXDUMPREPT
         JC     @@litlp
         MOV    EAX,[dtx]                       ; Calc adjusted values.
         SHL    EAX,SHADETEXDUMPBITS
         ADD    [ptx],EAX
         MOV    EAX,[dty]
         SHL    EAX,SHADETEXDUMPBITS
         ADD    [pty],EAX
         MOV    EAX,[dl_]
         SHL    EAX,SHADETEXDUMPBITS
         ADD    [pl],EAX
         I = 0
         REPT SHADETEXDUMPREPT
                ShTextDrawBody %I
                I = I + 1
         ENDM
         SUB    [TextCounter],SHADETEXDUMPREPT
         JZ     @@eofdraw
         ADD    EDI,SHADETEXDUMPREPT
         MOV    EBP,[pty]                       ; Adjust from calced values.
         SHL    EBP,8
         MOV    BP,WORD PTR [ptx+1]
         MOV    ECX,DWORD PTR [pl+1]
         JMP    @@biglp
  @@litlp:
         CMP    [TextCounter],8
         JC     @@minilp
         I = 0
         REPT 8
                ShTextDrawBody %I
                I = I + 1
         ENDM
         SUB    [TextCounter],8
         JZ     @@eofdraw
         ADD    EDI,8
         JMP    @@litlp
  @@minilp:
         I = 0
         REPT 8-1-1
                ShTextDrawBody %I
                DEC     [TextCounter]
                JZ      @@eofdraw
                I = I + 1
         ENDM
         ShTextDrawBody %I
    @@eofdraw:
        POP     ESI EDI

    @@nx:
        ADD     EDI,[ScrWidth]
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dl
        POP     FS EBP
        RET

; PUBLIC void POLY_ShadeTexDump256(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture, word ShadeTableSel);
; #pragma aux POLY_ShadeTexDump256 parm [EDI] [ESI] [ECX] [EDX] [EBX] [EAX]

        CODESEG

PUBLIC POLY_ShadeTexDump256_
POLY_ShadeTexDump256_:
        CLD
        PUSH    EBP FS
        MOV     [NScans],ECX
        MOV     [Texture],EBX
        MOV     [ScrWidth],EDX
        MOV     FS,AX
COMMENT #
        MOV     [RecalcDeltas],1
        CMP     ECX,6
        JC      @@dl
; JMP @@dl

        MOV     EBX,[ESI+4*FULLTEDGESIZE]
        MOV     ECX,[ESI+4*FULLTEDGESIZE+4]
        SUB     ECX,EBX         ; Length of run.
        JLE     @@dl
        MOV     EAX,[ESI+4*FULLTEDGESIZE+12]    ; tx1
        SUB     EAX,[ESI+4*FULLTEDGESIZE+8]     ; tx1-tx0
        CDQ
        IDIV    ECX             ; tx delta per pixel.
        MOV     [dtx],EAX
        MOV     EAX,[ESI+4*FULLTEDGESIZE+20]    ; ty1
        SUB     EAX,[ESI+4*FULLTEDGESIZE+16]    ; ty1-ty0
        CDQ
        IDIV    ECX             ; ty delta per pixel.
        MOV     [dty],EAX
        MOV     [RecalcDeltas],0
;#
   @@dl:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MaxX]
        JGE     @@nx
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MinX]
        JLE     @@nx
        SUB     ECX,EBX         ; Length of run.
        JLE     @@nx
;        CMP     [RecalcDeltas],0
;        JZ      @@okdeltas
        MOV     EAX,[ESI+12]    ; tx1
        SUB     EAX,[ESI+8]     ; tx1-tx0
        CDQ
        IDIV    ECX             ; tx delta per pixel.
        MOV     [dtx],EAX
        MOV     EAX,[ESI+20]    ; ty1
        SUB     EAX,[ESI+16]    ; ty1-ty0
        CDQ
        IDIV    ECX             ; ty delta per pixel.
        MOV     [dty],EAX
   @@okdeltas:
        MOV     EAX,[ESI+28]    ; l1
        SUB     EAX,[ESI+24]    ; l1-l0
        CDQ
        IDIV    ECX             ; l delta per pixel.
        MOV     [dl_],EAX

        CMP     EBX,[_POLY_MinX]
        JGE     @@nlc
         ; Left clipping. Advance texture values by appropiate amnt.
         SUB    EBX,[_POLY_MinX]
         NEG    EBX             ; EBX < 0  so make it positive.
         MOV    EAX,[ESI+12]    ; tx1
         SUB    EAX,[ESI+8]     ; tx1-tx0
         CDQ
         IMUL   EBX
         IDIV   ECX             ; tx delta per pixel.
         ADD    [ESI+8],EAX
         MOV    EAX,[ESI+20]    ; ty1
         SUB    EAX,[ESI+16]    ; ty1-ty0
         CDQ
         IMUL   EBX
         IDIV   ECX             ; ty delta per pixel.
         ADD    [ESI+16],EAX
         MOV    EAX,[ESI+28]    ; l1
         SUB    EAX,[ESI+24]    ; l1-l0
         CDQ
         IMUL   EBX
         IDIV   ECX             ; l delta per pixel.
         ADD    [ESI+24],EAX
         MOV    EBX,[_POLY_MinX]
    @@nlc:
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JLE     @@nrc
          ; Right clipping: simply cut the scan right edge.
         MOV    ECX,[_POLY_MaxX]
    @@nrc:
        SUB     ECX,EBX
        JLE     @@nx
        PUSH    EDI ESI
        ADD     EDI,EBX

        ; Draw the scan. Register setup by now:
        ; ECX = #pixels to draw.
        ; EDI = screen ptr.
        ; ESI = edge info ptr.

          ; 0.5 advance
         MOV    EAX,[dtx]
         SAR    EAX,1
         ADD    EAX,[ESI+8]
         MOV    [ptx],EAX
         MOV    EAX,[dty]
         SAR    EAX,1
         ADD    EAX,[ESI+16]
         MOV    [pty],EAX
         MOV    EAX,[dl_]
         SAR    EAX,1
         ADD    EAX,[ESI+24]
         MOV    [pl],EAX

         MOV    EBX,[dty]
         SHL    EBX,8
         MOV    BX,WORD PTR [dtx+1]
         MOV    EBP,[pty]              ;[ESI+16]
         SHL    EBP,8
         MOV    BP,WORD PTR [ptx+1]    ; [ESI+8+1]

         MOV    EDX,[Texture]

SHADETEXDUMPBITS = 5
SHADETEXDUMPREPT = (1 SHL (SHADETEXDUMPBITS))
ShTextDrawBody MACRO p
        ; Texture loop: This uses EAX,EBP,EBX,EDX and EDI. ESI and ECX are free.
        ; TSB is the number of bits for each texture coordinate.
        MOV   EAX,EBP
        SHR   EAX,32-TSB ; This clears high word of EAX. Keep it clean.
        SHLD  AX,BP,8
        ADD   EBP,EBX
        MOV   AL,[EDX+EAX]
        MOV   AH,CH
        ADD   ECX,ESI
        MOV   AL,FS:[EAX]
        MOV   [EDI+p],AL
ENDM

         MOV    [TextCounter],ECX
         MOVSX  ECX,WORD PTR [pl+1]          ; pl?
         MOVSX  ESI,WORD PTR [dl_+1]
         TEST   ESI,ESI
         JNS    @@dlp           ; delta < 0?
          INC   ESI             ; Round to zero, not to bottom
  @@dlp:
    @@biglp:
         CMP    [TextCounter],SHADETEXDUMPREPT
         JC     @@litlp
         MOV    EAX,[dtx]                       ; Calc adjusted values.
         SHL    EAX,SHADETEXDUMPBITS
         ADD    [ptx],EAX
         MOV    EAX,[dty]
         SHL    EAX,SHADETEXDUMPBITS
         ADD    [pty],EAX
         MOV    EAX,[dl_]
         SHL    EAX,SHADETEXDUMPBITS
         ADD    [pl],EAX
         I = 0
         REPT SHADETEXDUMPREPT
                ShTextDrawBody %I
                I = I + 1
         ENDM
         SUB    [TextCounter],SHADETEXDUMPREPT
         JZ     @@eofdraw
         ADD    EDI,SHADETEXDUMPREPT
         MOV    EBP,[pty]                       ; Adjust from calced values.
         SHL    EBP,8
         MOV    BP,WORD PTR [ptx+1]
         MOV    ECX,DWORD PTR [pl+1]
         JMP    @@biglp
  @@litlp:
         CMP    [TextCounter],8
         JC     @@minilp
         I = 0
         REPT 8
                ShTextDrawBody %I
                I = I + 1
         ENDM
         SUB    [TextCounter],8
         JZ     @@eofdraw
         ADD    EDI,8
         JMP    @@litlp
  @@minilp:
         I = 0
         REPT 8-1-1
                ShTextDrawBody %I
                DEC     [TextCounter]
                JZ      @@eofdraw
                I = I + 1
         ENDM
         ShTextDrawBody %I
    @@eofdraw:
        POP     ESI EDI

    @@nx:
        ADD     EDI,[ScrWidth]
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dl
        POP     FS EBP
        RET

; -------------------------------------------

; PUBLIC void POLY_LightTexDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture, const byte *ltable);
; #pragma aux POLY_LightTexDump parm [EDI] [ESI] [ECX] [EDX] [EBX] [EAX]

PUBLIC POLY_LightTexDump_
POLY_LightTexDump_:
        CLD
        PUSH    EBP
        MOV     [NScans],ECX
        MOV     [Texture],EBX
        MOV     [ScrWidth],EDX
        MOV     [dl_],EAX

   @@dl:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MaxX]
        JGE     @@nx
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MinX]
        JLE     @@nx
        SUB     ECX,EBX         ; Length of run.
        JLE     @@nx
        MOV     EAX,[ESI+12]    ; tx1
        SUB     EAX,[ESI+8]     ; tx1-tx0
        CDQ
        IDIV    ECX             ; tx delta per pixel.
        MOV     [dtx],EAX
        MOV     EAX,[ESI+20]    ; ty1
        SUB     EAX,[ESI+16]    ; ty1-ty0
        CDQ
        IDIV    ECX             ; ty delta per pixel.
        MOV     [dty],EAX

        CMP     EBX,[_POLY_MinX]
        JGE     @@nlc
         ; Left clipping. Advance texture values by appropiate amnt.
         SUB    EBX,[_POLY_MinX]
         NEG    EBX             ; EBX < 0  so make it positive.
         MOV    EAX,[ESI+12]    ; tx1
         SUB    EAX,[ESI+8]     ; tx1-tx0
         CDQ
         IMUL   EBX
         IDIV   ECX             ; tx delta per pixel.
         ADD    [ESI+8],EAX
         MOV    EAX,[ESI+20]    ; ty1
         SUB    EAX,[ESI+16]    ; ty1-ty0
         CDQ
         IMUL   EBX
         IDIV   ECX             ; ty delta per pixel.
         ADD    [ESI+16],EAX
         MOV    EBX,[_POLY_MinX]
    @@nlc:
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JLE     @@nrc
          ; Right clipping: simply cut the scan right edge.
         MOV    ECX,[_POLY_MaxX]
    @@nrc:
        SUB     ECX,EBX
        JLE     @@nx
        PUSH    EDI ESI
        ADD     EDI,EBX

        ; Draw the scan. Register setup by now:
        ; ECX = #pixels to draw.
        ; EDI = screen ptr.
        ; ESI = edge info ptr.

          ; 0.5 advance
         MOV    EAX,[dtx]
         SAR    EAX,1
         ADD    EAX,[ESI+8]
         MOV    [ptx],EAX
         MOV    EAX,[dty]
         SAR    EAX,1
         ADD    EAX,[ESI+16]
         MOV    [pty],EAX

         MOV    EBX,[dty]
         SHL    EBX,8
         MOV    BX,WORD PTR [dtx+1]
         MOV    EBP,[pty]              ;[ESI+16]
         SHL    EBP,8
         MOV    BP,WORD PTR [ptx+1]    ; [ESI+8+1]

         MOV    EDX,[Texture]

LIGHTTEXDUMPBITS = 5
LIGHTTEXDUMPREPT = (1 SHL (LIGHTTEXDUMPBITS))
LtTextDrawBody MACRO p
        ; Texture loop: This uses EAX,EBP,EBX,EDX and EDI.
        ; TSB is the number of bits for each texture coordinate.
        MOV   EAX,EBP
        SHR   EAX,32-TSB ; This clears high word of EAX. Keep it clean.
        SHLD  AX,BP,TSB
        ADD   EBP,EBX
        MOV   CL,[EDX+EAX]
        MOV   AL,[ESI+ECX]
        MOV   [EDI+p],AL
ENDM

         MOV    [TextCounter],ECX
         XOR    ECX,ECX
         MOV    ESI,[dl_]
    @@biglp:
         CMP    [TextCounter],LIGHTTEXDUMPREPT
         JC     @@litlp
         MOV    EAX,[dtx]                       ; Calc adjusted values.
         SHL    EAX,LIGHTTEXDUMPBITS
         ADD    [ptx],EAX
         MOV    EAX,[dty]
         SHL    EAX,LIGHTTEXDUMPBITS
         ADD    [pty],EAX
         I = 0
         REPT LIGHTTEXDUMPREPT
                LtTextDrawBody %I
                I = I + 1
         ENDM
         SUB    [TextCounter],LIGHTTEXDUMPREPT
         JZ     @@eofdraw
         ADD    EDI,LIGHTTEXDUMPREPT
         MOV    EBP,[pty]                       ; Adjust from calced values.
         SHL    EBP,8
         MOV    BP,WORD PTR [ptx+1]
         JMP    @@biglp
  @@litlp:
         CMP    [TextCounter],8
         JC     @@minilp
         I = 0
         REPT 8
                LtTextDrawBody %I
                I = I + 1
         ENDM
         SUB    [TextCounter],8
         JZ     @@eofdraw
         ADD    EDI,8
         JMP    @@litlp
   @@minilp:
         I = 0
         REPT 8-1-1
                LtTextDrawBody %I
                DEC     [TextCounter]
                JZ      @@eofdraw
                I = I + 1
         ENDM
         LtTextDrawBody %I
    @@eofdraw:
        POP     ESI EDI

    @@nx:
        ADD     EDI,[ScrWidth]
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dl
        POP     EBP
        RET

;PUBLIC void POLY_TransDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *ltable);
;#pragma aux POLY_TransDump parm [EDI] [ESI] [ECX] [EDX] [EBX]

PUBLIC POLY_TransDump_
POLY_TransDump_:
        CLD
        PUSH    EBP
        MOV     [NScans],ECX
        MOV     [ScrWidth],EDX

   @@dl:
        MOV     EAX,[ESI]
        CMP     EAX,[_POLY_MaxX]
        JGE     @@nx
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MinX]
        JLE     @@nx
        CMP     EAX,[_POLY_MinX]
        JGE     @@c1
         MOV    EAX,[_POLY_MinX]
   @@c1:
        CMP     ECX,[_POLY_MaxX]
        JL      @@c2
         MOV    ECX,[_POLY_MaxX]
   @@c2:
        SUB     ECX,EAX         ; Length of run.
        JLE     @@nx
        PUSH    EDI
        ADD     EDI,EAX

        ; Draw the scan. Register setup by now:
        ; ECX = #pixels to draw.
        ; EDI = screen ptr.

TRANSDUMPBITS = 5
TRANSDUMPREPT = (1 SHL (TRANSDUMPBITS))

         MOV    EAX,ECX
         ADD    ECX,TRANSDUMPREPT-1
         SHR    ECX,TRANSDUMPBITS
         AND    EAX,TRANSDUMPREPT-1             ; < 256.
;         SUB    EDI,[@@TrDrawAddTbl+4*EAX]
         JMP    [@@TrDrawJumpTbl+4*EAX]

TrDrawBody MACRO p
@@jtdb&p:
        MOV     AL,[EDI]
        INC     EDI
        MOV     AL,[EBX+EAX]
        MOV     [EDI-1],AL
ENDM

TrDrawLabel MACRO p
           DD @@jtdb&p
        ENDM

        ALIGN 4
@@TrDrawAddTbl LABEL DWORD
        DD 0
        I = TRANSDUMPREPT-1
        REPT TRANSDUMPREPT-1
           DD I
           I = I - 1
        ENDM
@@TrDrawJumpTbl LABEL DWORD
        DD @@jtdb0
        I = TRANSDUMPREPT-1
        REPT TRANSDUMPREPT-1
           TrDrawLabel %I
           I = I - 1
        ENDM

    @@textdrawl:
          i = 0
          REPT TRANSDUMPREPT
             TrDrawBody %i
             i = i + 1
          ENDM
          DEC   ECX
          JNZ   @@textdrawl

        POP     EDI

    @@nx:
        ADD     EDI,[ScrWidth]
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dl
        POP     EBP
        RET




; PUBLIC void POLY_ZTextureDump(byte *dest, POLY_TFullEdge *edge, int nscans, int width, const byte *texture);
; #pragma aux POLY_ZTextureDump parm [EDI] [ESI] [ECX] [EDX] [EBX]

ZTEXDUMPREPT = 16
ZTEXDUMPBITS = 4

PUBLIC POLY_ZTextureDump_
POLY_ZTextureDump_:
        CLD
        PUSH    EBP
        MOV     [NScans],ECX
        MOV     [Texture],EBX
        MOV     [ScrWidth],EDX

   @@dl:
        MOV     EBX,[ESI]
        CMP     EBX,[_POLY_MaxX]
        JGE     @@nx
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MinX]
        JLE     @@nx
        SUB     ECX,EBX         ; Length of run.
        JLE     @@nx
        MOV     EAX,[ESI+12]    ; tx1
        SUB     EAX,[ESI+8]     ; tx1-tx0
        CDQ
        IDIV    ECX             ; tx delta per pixel.
        MOV     [dtx],EAX
	SHL	EAX,ZTEXDUMPBITS
	MOV	[dtxm],EAX
        MOV     EAX,[ESI+20]    ; ty1
        SUB     EAX,[ESI+16]    ; ty1-ty0
        CDQ
        IDIV    ECX             ; ty delta per pixel.
        MOV     [dty],EAX
	SHL	EAX,ZTEXDUMPBITS
	MOV	[dtym],EAX
        MOV     EAX,[ESI+36]    ; z1
        SUB     EAX,[ESI+32]    ; z1-z0
        CDQ
        IDIV    ECX             ; z delta per pixel.
        MOV     [dz],EAX
	SHL	EAX,ZTEXDUMPBITS
	MOV	[dzm],EAX

        CMP     EBX,[_POLY_MinX]
        JGE     @@nlc
         ; Left clipping. Advance texture values by appropiate amnt.
         SUB    EBX,[_POLY_MinX]
         NEG    EBX             ; EBX < 0  so make it positive.
         MOV    EAX,[ESI+12]    ; tx1
         SUB    EAX,[ESI+8]     ; tx1-tx0
         IMUL   EBX
         IDIV   ECX             ; tx delta per pixel.
         ADD    [ESI+8],EAX
         MOV    EAX,[ESI+20]    ; ty1
         SUB    EAX,[ESI+16]    ; ty1-ty0
         IMUL   EBX
         IDIV   ECX             ; ty delta per pixel.
         ADD    [ESI+16],EAX
         MOV    EAX,[ESI+36]    ; z1
         SUB    EAX,[ESI+32]    ; z1-z0
         IMUL   EBX
         IDIV   ECX             ; z delta per pixel.
         ADD    [ESI+32],EAX
         MOV    EBX,[_POLY_MinX]
    @@nlc:
        MOV     ECX,[ESI+4]
        CMP     ECX,[_POLY_MaxX]
        JLE     @@nrc
          ; Right clipping: simply cut the scan right edge.
         MOV    ECX,[_POLY_MaxX]
    @@nrc:
        SUB     ECX,EBX
        JLE     @@nx
        PUSH    EDI ESI
        ADD     EDI,EBX

        MOV	EAX,[ESI+8]
        MOV	[ptx],EAX
        MOV	EAX,[ESI+16]
        MOV	[pty],EAX
        MOV	EAX,[ESI+32]
        MOV	[pz],EAX

        ; Draw the scan. Register setup by now:
        ; ECX = #pixels to draw.
        ; EDI = screen ptr.
        ; ESI = edge info ptr.

ZTextDrawBody MACRO p
        MOV   EAX,EBP	   ;1
        SHR   EAX,32-TSB   ;2/1	; This clears high word of EAX. Keep it clean.
        SHLD  AX,BP,TSB	   ;4/6
        ADD   EBP,EBX	   ;1/
        MOV   AL,[EDX+EAX] ;1/
        MOV   [EDI+p],AL   ;2/2
        	           ;11/12
ENDM

	MOV	EDX,[ptx]
	XOR	EAX,EAX
	IDIV	[pz]
	MOV	[ptxcz],EAX	; Store p-correct tx
	MOV	EDX,[pty]
	XOR	EAX,EAX
	IDIV	[pz]
	MOV	[ptycz],EAX	; Store p-correct ty

	CMP	ECX,ZTEXDUMPREPT
	JC	@@endlp

    @@biglp:
	MOV	EDX,[dtxm]	; Increment tx, ty, z
	ADD	[ptx],EDX
	MOV	EDX,[dtym]
	ADD	[pty],EDX
	MOV	EDX,[dzm]
	ADD	[pz],EDX

	MOV	EDX,[pty]
	XOR	EAX,EAX
	IDIV	[pz]
	MOV	EDX,[ptycz]
	MOV	EBP,EDX
	SHL	EBP,32-6-23
	MOV	[ptycz],EAX	; Store p-correct ty
	SUB	EAX,EDX		; delta of p-correct ty
;	SHL	EAX,32-6-23
;	SAR	EAX,ZTEXDUMPBITS
	SAR	EAX,ZTEXDUMPBITS-(32-6-23)
	MOV	EBX,EAX
	MOV	EDX,[ptx]
	XOR	EAX,EAX
	IDIV	[pz]
	MOV	EDX,[ptxcz]
	SAR	EDX,16-(32-6-23)
	MOV	BP,DX
	MOV	EDX,[ptxcz]
	MOV	[ptxcz],EAX	; Store p-correct tx
	SUB	EAX,EDX		; delta of p-correct ty
	SAR	EAX,16-(32-6-23-ZTEXDUMPBITS)
	MOV	BX,AX

	; Now EBP contains tx, ty and EBX is dtx, dty

	MOV	EDX,[Texture]
        I = 0
        REPT ZTEXDUMPREPT
                ZTextDrawBody %I
                I = I + 1
        ENDM
        SUB     ECX,ZTEXDUMPREPT
        JZ      @@eofdraw
        ADD     EDI,ZTEXDUMPREPT
	CMP	ECX,ZTEXDUMPREPT
	JC	@@endlp
        JMP     @@biglp
  @@endlp:
	MOV	EAX,[dtx]	; Increment tx, ty, z
	IMUL	ECX
	ADD	[ptx],EAX
	MOV	EAX,[dty]
	IMUL	ECX
	ADD	[pty],EAX
	MOV	EAX,[dz]
	IMUL	ECX
        ADD	[pz],EAX

	MOV	EDX,[pty]
	XOR	EAX,EAX
	IDIV	[pz]
	MOV	EDX,[ptycz]
	MOV	EBP,EDX
	SHL	EBP,32-6-23
	SUB	EAX,EDX		; delta of p-correct ty
	MOV	EDX,EAX
	SAR	EDX,31
	IDIV	ECX
	SHL	EAX,32-6-23
	MOV	EBX,EAX
	MOV	EDX,[ptx]
	XOR	EAX,EAX
	IDIV	[pz]
	MOV	EDX,[ptxcz]
	SHR	EDX,16-(32-6-23)
	MOV	BP,DX
	MOV	EDX,[ptxcz]
	SUB	EAX,EDX		; delta of p-correct ty
	MOV	EDX,EAX
	SAR	EDX,31
	IDIV	ECX
	SHR	EAX,16-(32-6-23)
	MOV	BX,AX
	MOV	EDX,[Texture]
    @@litlp:
         ZTextDrawBody 0
         INC	EDI
         DEC	ECX
         JNZ	@@litlp

    @@eofdraw:
        POP     ESI EDI

    @@nx:
        ADD     EDI,[ScrWidth]
        ADD     ESI,FULLTEDGESIZE
        DEC     [NScans]
        JNZ     @@dl
        POP     EBP
        RET



END





ZShTextDrawBody MACRO p
@@jtdb&p:
        ; Texture loop: This uses EAX,EBP,EBX,EDX and EDI. ESI and ECX are free.
        ; TSB is the number of bits for each texture coordinate.
        MOV   EAX,EBP
        SHR   EAX,32-TSB ; This clears high word of EAX. Keep it clean.
        SHLD  AX,BP,TSB
        ADD   EBP,EBX
        MOV   AL,[EDX+EAX]
        MOV   AH,CH
        ADD   ECX,ESI
        MOV   AL,FS:[EAX]
        MOV   ES:[EDI+p],AL
        MOV   ES:[EDI*4+p*4+64000],ECX

ENDM



; ----------------------------- POLYGONA.ASM -------------------

