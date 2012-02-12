
        .386
        .MODEL FLAT
	LOCALS @@

GRIDB    = 7
PIXELB   = 6
DECIMALB = 19
GRIDM    = ((1 SHL (GRIDB))    - 1) SHL (DECIMALB+PIXELB)
PIXELM   = ((1 SHL (PIXELB))   - 1) SHL (DECIMALB)
DECIMALM = ((1 SHL (DECIMALB)) - 1)

        DATASEG

counter DD 0
ct1	DD 0
ct2	DD 0

        CODESEG

        ; Register setup for floor coords in the inner loop:
        ;   6.6.20             (grid,pixel,decimals)
        ; That makes 64x64 blocks of 64x64 pixels == 4096x4096 map.
        ; 4096 bytes per map, and up to 1Mb for 256 tiles graphics.

;PUBLIC void FL_DrawRaster(byte *dest, int width,
;                          int x, int y, int dx, int dy);
;#pragma aux FL_DrawRaster parm [EDI] [EAX]
;                               [EBX] [ECX] [EDX] [ESI]
PUBLIC FL_DrawRaster_
FL_DrawRaster_:
        PUSH    EBP
        MOV     EBP,ESI
	ADD	EAX,1Fh
        SHR     EAX,5
        JZ	bye1
        MOV	[ct2],EAX
   fll1:
   	MOV	[counter],32/4
   	MOV	AL,[EDI]	; Read to bring the mem into the WB cache.
   flloop:

HiBody MACRO i
        MOV     EAX,ECX
        SHR     EAX,DECIMALB+PIXELB
        SHLD    EAX,EBX,GRIDB
        MOV     ESI,12345678h+[EAX*4]
mapaddrh&i LABEL DWORD
        MOV     EAX,ECX
        SHR     EAX,DECIMALB-PIXELB
        AND     EAX,PIXELM SHR (DECIMALB-PIXELB)
        ADD     ESI,EAX
        MOV     EAX,EBX
        ADD     ECX,EBP
        SHR     EAX,DECIMALB
        AND     EAX,PIXELM SHR DECIMALB         ; This makes EAX < 64.
        MOV     AL,BYTE PTR [ESI+EAX]
        ADD     EBX,EDX
        MOV     AL,12345678h+[EAX]
transaddrh&i LABEL DWORD
        MOV     [EDI+i],AL
ENDM

        HiBody 0
        HiBody 1
        HiBody 2
        HiBody 3
        ADD     EDI,4
        DEC     [counter]
        JNZ     flloop
        DEC     [ct2]
        JNZ     fll1
   bye1:
        POP     EBP
        RET

;PUBLIC void FL_DrawRasterLo(byte *dest, int width,
;                            int x, int y, int dx, int dy);
;#pragma aux FL_DrawRasterLo parm [EDI] [EAX]
;                                 [EBX] [ECX] [EDX] [ESI]
PUBLIC FL_DrawRasterLo_
FL_DrawRasterLo_:
        PUSH    EBP
        MOV     EBP,ESI
        ADD     EBX,EDX         ; Advance one pixel.
        ADD     ECX,EBP
        ADD     EDX,EDX         ; Double the deltas.
        ADD     EBP,EBP
	ADD	EAX,1Fh
        SHR     EAX,5
        JZ	bye2
        MOV	[ct2],EAX
   fl2l1:
   	MOV	[counter],32/4/2
   	MOV	AL,[EDI]	; Read to bring the mem into the WB cache.
   fl2loop:

LoBody MACRO i
        MOV     EAX,ECX
        SHR     EAX,DECIMALB+PIXELB
        SHLD    EAX,EBX,GRIDB
        MOV     ESI,12345678h+[EAX*4]
mapaddrl&i LABEL DWORD
        MOV     EAX,ECX
        SHR     EAX,DECIMALB-PIXELB
        AND     EAX,PIXELM SHR (DECIMALB-PIXELB)
        ADD     ESI,EAX
        MOV     EAX,EBX
        ADD     ECX,EBP
        SHR     EAX,DECIMALB
        AND     EAX,PIXELM SHR DECIMALB         ; This makes EAX < 64.
        MOV     AL,BYTE PTR [ESI+EAX]
        ADD     EBX,EDX
        MOV     AL,12345678h+[EAX]
transaddrl&i LABEL DWORD
        MOV     AH,AL
        MOV     [EDI+2*i],AX
ENDM

        LoBody 0
        LoBody 1
        LoBody 2
        LoBody 3
        ADD     EDI,2*4
        DEC     [counter]
        JNZ     fl2loop
        DEC     [ct2]
        JNZ     fl2l1
   bye2:
        POP     EBP
        RET

;PUBLIC void FL_DrawRasterTrans(byte *dest, int width,
;                          int x, int y, int dx, int dy);
;#pragma aux FL_DrawRasterTrans parm [EDI] [EAX]
;                               [EBX] [ECX] [EDX] [ESI]
PUBLIC FL_DrawRasterTrans_
FL_DrawRasterTrans_:
        PUSH    EBP
        MOV     EBP,ESI
        MOV     [counter],EAX
   fltloop:

        MOV     EAX,ECX
        SHR     EAX,DECIMALB+PIXELB
        SHLD    EAX,EBX,GRIDB
        MOV     ESI,12345678h+[EAX*4]
mapaddrt0 LABEL DWORD
        MOV     EAX,ECX
        SHR     EAX,DECIMALB-PIXELB
        AND     EAX,PIXELM SHR (DECIMALB-PIXELB)
        ADD     ESI,EAX
        MOV     EAX,EBX
        ADD     ECX,EBP
        SHR     EAX,DECIMALB
        AND     EAX,PIXELM SHR DECIMALB         ; This makes EAX < 64.
        MOV     AL,BYTE PTR [ESI+EAX]
        ADD     EBX,EDX
        CMP     AL,160
        JC      @@nx
        CMP     AL,192
        JNC     @@nx
        MOV     BYTE PTR [EDI],185
        INC     EDI
        DEC     [counter]
        JNZ     fltloop
        POP     EBP
        RET
    @@nx:
	MOV	EAX,1800h
        MOV     AL,[EDI]
        INC     EDI
        MOV     AL,12345678h+[EAX]
transaddrt0 LABEL DWORD
	MOV	[EDI-1],AL
        DEC     [counter]
        JNZ     fltloop

        POP     EBP
        RET

;PUBLIC void FL_DrawRasterNoTile(byte *dest, int width,
;                                uint32 x, uint32 y, uint32 dx, uint32 dy);
;#pragma aux FL_DrawRasterNoTile parm [EDI] [EAX]
;                                     [EBX] [ECX] [EDX] [ESI]
PUBLIC FL_DrawRasterNoTile_
FL_DrawRasterNoTile_:
        PUSH    EBP
        MOV     EBP,ESI
        SHR     EAX,2
        MOV     [counter],EAX
   flntloop:

HiBodyNT MACRO i
        LOCAL @@c1, @@c2
        MOV     EAX,ECX
        OR      EAX,EBX
        TEST    EAX,0C0000000h
        JZ      @@c1
         ADD    EBX,EDX
         ADD    ECX,EBP
;         MOV    BYTE PTR [EDI+i],0
         JMP    @@c2
   @@c1:
        MOV     EAX,ECX
        AND     EAX,NOT 0C0000000h
        SHR     EAX,DECIMALB+PIXELB-2
        ROL     EBX,2
        SHLD    EAX,EBX,GRIDB
        ROR     EBX,2
        MOV     ESI,12345678h+[EAX*4]
mapaddrhnt&i LABEL DWORD
        MOV     EAX,ECX
;        AND     EAX,NOT 0C0000000h
        SHR     EAX,DECIMALB-PIXELB-2
        AND     EAX,PIXELM SHR (DECIMALB-PIXELB)
        ADD     ESI,EAX
        MOV     EAX,EBX
        ADD     ECX,EBP
        SHR     EAX,DECIMALB-2
        AND     EAX,PIXELM SHR DECIMALB         ; This makes EAX < 64.
        MOV     AL,BYTE PTR [ESI+EAX]
        ADD     EBX,EDX
;        MOV     AL,12345678h+[EAX]
;transaddrhnt&i LABEL DWORD
        MOV     [EDI+i],AL
     @@c2:
ENDM

        HiBodyNT 0
        HiBodyNT 1
        HiBodyNT 2
        HiBodyNT 3
        ADD     EDI,4
        DEC     [counter]
        JNZ     flntloop

        POP     EBP
        RET




;PUBLIC void FL_SetMap(const byte *map[], const byte *trans);
;#pragma aux FL_SetMap parm [EAX] [EBX]

PUBLIC FL_SetMap_
FL_SetMap_:
        TEST    EAX,EAX
        JZ      @@c1
        CMP     EAX,[mapaddrh0-4]
        JZ      @@c1
        MOV     [mapaddrh0-4],EAX
        MOV     [mapaddrh1-4],EAX
        MOV     [mapaddrh2-4],EAX
        MOV     [mapaddrh3-4],EAX
        MOV     [mapaddrl0-4],EAX
        MOV     [mapaddrl1-4],EAX
        MOV     [mapaddrl2-4],EAX
        MOV     [mapaddrl3-4],EAX
        MOV     [mapaddrt0-4],EAX
        MOV     [mapaddrhnt0-4],EAX
        MOV     [mapaddrhnt1-4],EAX
        MOV     [mapaddrhnt2-4],EAX
        MOV     [mapaddrhnt3-4],EAX
   @@c1:
        TEST    EBX,EBX
        JZ      @@c2
        CMP     EBX,[transaddrh0-4]
        JZ      @@c2
        MOV     [transaddrh0-4],EBX
        MOV     [transaddrh1-4],EBX
        MOV     [transaddrh2-4],EBX
        MOV     [transaddrh3-4],EBX
        MOV     [transaddrl0-4],EBX
        MOV     [transaddrl1-4],EBX
        MOV     [transaddrl2-4],EBX
        MOV     [transaddrl3-4],EBX
        MOV     [transaddrt0-4],EBX
   @@c2:
        RET

END

