; ------------------------------ FIRE.ASM ---------------------
; Credits part.
; (C) 1993 bye JCAB.

	.386
	.MODEL FLAT
	LOCALS @@

	.CODE

FIREW	= 160
FIREH	= 115
FIRES	= 4
DECV    = 160

PUBLIC FIRE_Move_
FIRE_Move_:
	PUSH	EBP
	MOV	EBP,EDX

        ADD     ESI,(FIREW+1)*FIRES

        MOV     ECX,FIREW*(FIREH-3)
        MOV     EDX,[ESI-3*FIRES]
        ADD     EDX,[ESI-2*FIRES]
        ADD     EDX,[ESI-1*FIRES]
        MOV     EBX,[ESI-3*FIRES+2*FIREW*FIRES]
        ADD     EBX,[ESI-2*FIRES+2*FIREW*FIRES]
        ADD     EBX,[ESI-1*FIRES+2*FIREW*FIRES]
    @@lp:
          ADD   EDX,[ESI]
          SUB   EDX,[ESI-3*FIRES]
          ADD   EBX,[ESI+2*FIREW*FIRES]
          SUB   EBX,[ESI+2*FIREW*FIRES-3*FIRES]
          MOV   EAX,EBX
          ADD   EAX,EDX
          ADD   EAX,[ESI+FIREW*FIRES]
          ADD   EAX,[ESI+FIREW*FIRES-2*FIRES]
          SHR   EAX,3
	  CMP	EAX,EBP
          JC    @@sal
          SUB	EAX,EBP
      @@nx:
          STOSD
          ADD   ESI,FIRES
         DEC	ECX
         JNZ    @@lp
    @@Fin:
    	POP	EBP
        RET
    @@sal:
        XOR     EAX,EAX
        STOSD
        ADD     ESI,FIRES
        DEC	ECX
        JNZ     @@lp
        JMP	@@Fin

PUBLIC FIRE_Dump_
FIRE_Dump_:
NR = 8

        INC     ESI
	MOV	DX,3C4h
	MOV	AX,0302h
	OUT	DX,AX
	PUSH	EDI ESI
	CALL	@@rut
	POP	ESI EDI
	ADD	ESI,4
	MOV	DX,3C4h
	MOV	AX,0C02h
	OUT	DX,AX
	PUSH	EDI
	CALL	@@rut
	POP	EDI
	MOV	DX,3C4h
	MOV	AX,0F02h
	OUT	DX,AX
	MOV	DX,3CEh
	MOV	AX,0008h
	OUT	DX,AX
	MOV	ESI,EDI
	ADD	EDI,FIREW/2
	MOV	EDX,100
    @@cl:
         MOV	ECX,FIREW/2
         REP	MOVSB
	ADD	EDI,FIREW/2
	ADD	ESI,FIREW/2
         DEC	EDX
         JNZ	@@cl

	MOV	DX,3CEh
	MOV	AX,0FF08h
	OUT	DX,AX

	RET

    @@rut:
    	MOV	EDX,100
    @@l1:
        MOV     ECX,FIREW/NR/2/2
    @@l2:
	 i = 0
         REPT NR
          MOV	AL,[ESI+2*FIRES*(2*i+1)]
          MOV	[EDI+2*i+1],AL
          MOV   AH,[ESI+2*FIRES*(2*i+0)]
          MOV	[EDI+2*i],AH
	  i = i + 1
         ENDM
         ADD	EDI,2*NR
         ADD	ESI,2*FIRES*2*NR
         DEC	ECX
         JNZ	@@l2
         ADD	EDI,FIREW/2
         DEC	EDX
         JNZ	@@l1
        RET

END

; ------------------------ End of FIRE.ASM ---------------------------
