
        .386
        .MODEL FLAT
	LOCALS @@

	DATASEG

	CODESEG


; PUBLIC void HZ_DrawBackground320(byte *dest, const byte *b1, int h1, const byte *b2, int h2, int h);
; pragma aux  HZ_DrawBackground320 parm [EDI] [ESI] [EDX] [EBX] [ECX] [EAX]

PUBLIC HZ_DrawBackground320_
HZ_DrawBackground320_:
	PUSH	EBP

   @@l1:
   	CMP	EAX,0	; h
   	JLE	@@bye
   	CMP

   @@bye:
   	POP	EBP
	RET


END
