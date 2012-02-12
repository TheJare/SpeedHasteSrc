; ----------------------------- HORDRAWA.ASM -------------------
; Bye Jare of Iguana during 30-31/12/1994.
; Draw an horizontal span of pixels, stored as:
;  #pixels to skip (transparent).
;  #pixels of data + data itself.
;   ... again, until end.
; Doesn't allow spans greater than 256, so we can use a #pixels == 0 to
; mark end of span (except as the first transparent span).

        .386P
        .MODEL FLAT
        LOCALS @@

	DATASEG

EXTRN   _DRW_Tile:      BYTE  ; Boolean, != 0 means tile the texture.

        CODESEG

; extern void DRW_DoHorizontalDraw(byte *dest, const byte *data, int skip, int width);
; #pragma aux DRW_DoHorizontalDraw parm [EDI] [ESI] [EDX] [EBX]

PUBLIC DRW_DoHorizontalDraw_
DRW_DoHorizontalDraw_:
        CLD
        PUSH    ECX
@@aga:
        PUSH	ESI
        MOV	AL,[ESI]
        INC	ESI
        TEST    AL,AL
        JZ      @@draw
@@skip:
         MOVZX  EAX,AL
	 SUB	EBX,EAX
	 JLE	@@bye		; Width exhausted, goodbye.
         SUB	EDX,EAX
         JNC	@@draw
         SUB    EDI,EDX		; EDX is < 0, so it's ADD Abs(EDX)
         XOR	EDX,EDX
@@draw:
        MOV	AL,[ESI]
        INC	ESI
        TEST    AL,AL
        JZ      @@enddata
        MOVZX   ECX,AL
        ADD	ESI,ECX
        SUB	EBX,ECX		; Subtract run width from total width
        SUB	EDX,ECX		; And from 'skip'.
        JNC	@@ltr		; If skip is still > 0 skip this run.
    @@dtr:
	ADD	ESI,EDX
	XOR	ECX,ECX
	SUB	ECX,EDX		; ECX = Abs(EDX) = -EDX
	TEST	EBX,EBX
	JNS	@@drawit
	 ADD	ECX,EBX		; ECX = ECX - Abs(EBX) ECX - (-EBX)
    @@drawit:
	XOR	EDX,EDX
	TEST	ECX,ECX		; Reposition for byte copying.
	JLE	@@bye
    	MOV	EAX,ECX
        SHR     ECX,2
        REP MOVSD
        MOV     CL,AL
        AND     CL,3
        REP MOVSB
  @@ltr:
	TEST	EBX,EBX
	JLE	@@bye
        MOV	AL,[ESI]
        INC	ESI
        TEST    AL,AL
        JNZ     @@skip
  @@enddata:
	CMP	[_DRW_Tile],0
	JZ	@@bye
        POP	ESI		; Tile!
        JMP	@@aga
   @@bye:
	POP	ESI
        POP     ECX
        RET

END

; ----------------------------- HORDRAWA.ASM -------------------
