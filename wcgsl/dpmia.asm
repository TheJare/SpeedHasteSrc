; -------------------------- DPMIA.ASM -------------------------
; Assembler routines that interface the DPMI server.
; For use with Watcom C 9.5 and DOS4GW
; (C) Copyright 1993-4 by Jare & JCAB of Iguana.

        LOCALS @@
        .386P

_DATA SEGMENT BYTE PUBLIC USE32 'DATA'
_DATA ENDS
DGROUP GROUP _DATA

_TEXT SEGMENT PARA PUBLIC USE32 'CODE'
ASSUME CS:_TEXT, DS:DGROUP




PUBLIC DPMI_InterruptFix_
DPMI_InterruptFix_:
		PUSH	EBX ECX

		MOV	ECX,ESP
		MOV	BX,SS
		PUSH	EBX
                PUSH    ECX
		MOV	EAX,6
		INT	31h
		SHL	ECX,16
		MOV	CX,DX
		POP	EAX
                POP     EDX
		ADD	EBP,ECX
		ADD	ECX,EAX
		MOV	BX,DS
		MOV	SS,BX
		MOV	ESP,ECX
                MOV     ES,BX

		POP	ECX EBX
		RET




PUBLIC DPMI_InterruptRestore_
DPMI_InterruptRestore_:
		PUSH	EBX ECX

		MOV	EBX,ESP
		SUB	EBX,EAX
		SUB	EBP,EBX
		MOV	SS,DX
		MOV	ESP,EAX

		POP	ECX EBX
		RET




ENDS
END
