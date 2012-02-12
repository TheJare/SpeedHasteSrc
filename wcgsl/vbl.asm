ð; -------------------------- VBL.H -------------------------
; For use with Watcom C 10.0 and DOS4GW
; Vertical blanking interrupt.
; (C) Copyright 1993-95 by Jare & JCAB of Iguana.

	.386
	.MODEL FLAT
	JUMPS
	LOCALS @@

SetBorder MACRO r,g,b
    LOCAL @@j
	PUSH	EDX EAX
	MOV	DX,3C8h
	XOR	AL,AL
	OUT	DX,AL
	INC	DX
      IFDIF <&r>,<0>
	MOV	AL,&r
      ENDIF
	OUT	DX,AL
      IFDIF <&g>,<&r>
	MOV	AL,&g
      ENDIF
	OUT	DX,AL
      IFDIF <&b>,<&g>
	MOV	AL,&b
      ENDIF
	OUT	DX,AL
    @@j:POP	EAX EDX
ENDM

VSync MACRO
     LOCAL @@vs1, @@vs2
	PUSH	EAX EDX
	MOV	DX,3DAh 		; Retrace sync.
@@vs1:
	IN	AL,DX
	TEST	AL,8
	JNZ	@@vs1			; While not in display do.
@@vs2:
	IN	AL,DX
	TEST	AL,8
	JZ	@@vs2			; While in display do.
	POP	EDX EAX
ENDM


	DATASEG

ALIGN 4

    ; Set this prior to using the module. By default it is set to
    ; FALSE, meaning the VBL will have unrestricted access to the
    ; retrace signal. If TRUE, the VBL will have to asume that it
    ; can not sync correctly, and therefor revert to a flick-prone
    ; pure 70Hz mode and not wait for the retrace.
EXTRN _VBL_CompatibleMode: DWORD

PUBLIC _VBL_OldHandler, _VBL_Active, _VBL_Empty

PUBLIC _Frames
_Frames LABEL WORD

FullFrame   DW 0		; Total timerticks for a frame.
UsualFrame  DW 0		; Ticks to program the timer for a frame.
HalfFrame   DW 0		; ... to reach the midframe point.
SecFrame    DW 0		; ... from midframe point to retrace.
IntDelay    DW 0
VBLTest     DD 0

_VBL_OldHandlerOfs LABEL DWORD
_VBL_OldHandler 	DD 0		   ; Old handler.
_VBL_OldHandlerSeg 	DW 0		   ; Selector. :)

ALIGN 4

_EAX DD 0
_EBX DD 0
_ECX DD 0
_EDX DD 0
_ESI DD 0
_EDI DD 0
_EBP DD 0

VBLCounter   DD 0		; Frame counter.
VBLVSyncOldV DD 0		; Alternate frame ctr. for VBLVSync.

SecTimerAct DB 0		; Indicates the presence of a sec. timer.
WasFullTick DB 0		; Set to 0 by half frame handler.
_VBL_Active DB 0		; = 0: Ignore the timer INT.
_VBL_Empty  DB 0		; Idem, for temporary use.

ALIGN 4

PUBLIC _VBL_HalfHandler, _VBL_FullHandler, _VBL_Selector

_VBL_HalfHandler DD OFFSET RetValue ; Routine for the half frame.
_VBL_FullHandler DD OFFSET RetValue ; Routine for the full frame.
_VBL_Selector	 DW 0		    ; Data segment selector.

ALIGN 4

ChangePageVal  DW 0
ChangePageFlag DB 0

ALIGN 4

ChangePanVal   DB 0
ChangePanFlag  DB 0

ALIGN 4

PUBLIC _VBL_Palette, _VBL_OldPal, _VBL_FirstColor, _VBL_LastColor

_VBL_Palette	DD 0
_VBL_OldPal	DD 0

_VBL_FirstColor DW 256
_VBL_LastColor	DW 0

VBL_TempPal	DD 0
VBL_TempOldPal	DD 0
VBL_TempNum	DD 0


	CODESEG

; --------------------------------------------------
; Timer handler for delay test.

PUBLIC VBL_TestHandler_
VBL_TestHandler_:
	CLI
	PUSH	DS
	MOV	DS,CS:[_VBL_Selector]
	INC	[VBLTest]
	POP	DS
	PUSH	EAX
	MOV	AL,20h			; Signal EOI.
	OUT	20h,AL
	POP	EAX
	IRETD

; --------------------------------------------------
; VBL Timer handler. Calls subhandlers and sets
; timers. First, a do-nothing IRQ handler for
; the times when you don't want to relaunch the
; timer (when setting it, for example).

TimerVBLEmpty:
	PUSH	EAX
	MOV	AL,20h			; Signal EOI.
	OUT	20h,AL
	POP	EAX
	IRETD
; --------------------------------------------------
PUBLIC VBL_Handler_
VBL_Handler_:
	CLI
	CLD
	ASSUME CS:nothing
	TEST	CS:[_VBL_Active],0FFh
	ASSUME CS:_TEXT
	JZ	SHORT TimerVBLEmpty
	ASSUME CS:nothing
	TEST	CS:[_VBL_Empty],0FFh
	ASSUME CS:_TEXT
	JNZ	SHORT TimerVBLEmpty
;JMP   SHORT TimerVBLEmpty
	PUSH	DS
	ASSUME CS:nothing
	MOV	DS,CS:[_VBL_Selector]
	ASSUME CS:_TEXT
	MOV	[_VBL_Empty],1
	PUSH	ES
	PUSH	GS
	PUSH	FS
	MOV	ES,[_VBL_Selector]
	MOV	GS,[_VBL_Selector]
	MOV	FS,[_VBL_Selector]
	MOV	[_EAX],EAX
	MOV	[_EBX],EBX
	MOV	[_ECX],ECX
	MOV	[_EDX],EDX
	MOV	[_ESI],ESI
	MOV	[_EDI],EDI
	MOV	[_EBP],EBP
;	 SetBorder 63, 0, 0
	MOV	AL,20h			; Signal EOI.
	OUT	20h,AL
	XOR	AL,AL
	CMP	AL,[SecTimerAct]
	JZ	SHORT @@EndFrame	; JMP if no sec. timer.
	CMP	AL,[WasFullTick]
	JZ	SHORT @@EndFrame	; Or if it's the full timer.
	MOV	[WasFullTick],AL

	MOV	AL,36h
	OUT	43h,AL
	MOVZX	EAX,[SecFrame]
	OUT	40h,AL
	MOV	AL,AH
	OUT	40h,AL

	STI
	; Way to go!!
	CALL	[_VBL_HalfHandler]
	JMP	@@end

   @@EndFrame:
	CMP	[ChangePageFlag],0
	JZ	SHORT @@nopage
	 MOV	EDX,3D4h
	 MOV	AL,0Ch
	 MOV	AH,BYTE PTR [ChangePageVal+1]
	 OUT	DX,AX
	 INC	AL
	 MOV	AH,BYTE PTR [ChangePageVal]
	 OUT	DX,AX
	 MOV	[ChangePageFlag],0
   @@nopage:

	; Do a VSync while dumping the palette.

	XOR	ECX,ECX

	MOVSX	EAX,[_VBL_FirstColor]
	MOVSX	EDX,[_VBL_LastColor]
	CMP	EAX,EDX
	JG	LARGE @@vscont
	MOV	CL,AL
	MOV	CH,DL
	LEA	EAX,[2*EAX+EAX]
	MOV	ESI,[_VBL_Palette]
	LEA	ESI,[ESI+EAX]
	MOV	EDI,[_VBL_OldPal]
	LEA	EDI,[EDI+EAX]
	CMP	[_VBL_CompatibleMode],0
	JNZ	@@compat1
	DEC	CH
	JMP	SHORT @@vsentry

  @@vsnc1:
	 CMP	CL,CH
	 JA	SHORT @@vssc2
	 INC	CL
  @@vsentry:
	 MOV	EAX,[ESI]
	 MOV	EBX,[EDI]
	 AND	EAX,003F3F3Fh
	 AND	EBX,003F3F3Fh
	 CMP	EAX,EBX
	 JNZ	SHORT @@vssc3_prep
  @@vsnc3:ADD	ESI,3
	  ADD	EDI,3
  @@vsnc2:
	 MOV	EDX,3DAh		; Retrace sync.
	 IN	AL,DX
	 TEST	AL,8
	JZ	SHORT @@vsnc1		; While in display do.
	JMP	SHORT @@vscont

  @@vssc3_prep:
	MOV	EDX,3C8h
	MOV	BL,AL
	MOV	AL,CL
	OUT	DX,AL
	MOV	AL,BL
	INC	EDX
	JMP	SHORT @@vssc3

  @@vssc1:
	 CMP	CL,CH
	 JA	SHORT @@vssc2
	 INC	CL
	 MOV	EAX,[ESI]
	 MOV	EBX,[EDI]
	 AND	EAX,003F3F3Fh
	 AND	EBX,003F3F3Fh
	 CMP	EAX,EBX
	 JZ	SHORT @@vsnc3
	  MOV	EDX,3C9h
  @@vssc3:
  	  STOSB
	  SHR	EAX,8
	  STOSW
	  OUTSB
	  OUTSB
	  OUTSB
  @@vssc2:
	 MOV	EDX,3DAh		; Retrace sync.
	 IN	AL,DX
	 TEST	AL,8
	JZ	SHORT @@vssc1		; While in display do.

  @@vscont:
	CMP	[_VBL_CompatibleMode],0
	JNZ	@@compat1
	 MOV	EDX,3DAh		; Retrace sync.
	 IN	AL,DX
	 TEST	AL,8
	 JZ	SHORT @@vscont		; While in display do.

   @@compat1:
	MOV	[VBL_TempPal],ESI
	MOV	[VBL_TempOldPal],EDI
	MOV	[VBL_TempNum],ECX

;	 SetBorder 0, 63,63

	INC	[VBLCounter]
	MOV	[WasFullTick],1
	MOVZX	EDX,[HalfFrame]
	CMP	[SecTimerAct],0
	JNZ	SHORT @@justfull
	 MOVZX	EDX,[UsualFrame]
   @@justfull:
	MOV	AL,36h
	OUT	43h,AL
	MOV	EAX,EDX
	OUT	40h,AL
	MOV	AL,AH
	OUT	40h,AL

	; Finish dumping the palette.

	MOV	ECX,[VBL_TempNum]
	JCXZ	SHORT @@rvscont
	CMP	CL,CH
	JA	SHORT @@rvscont
	CMP	[_VBL_CompatibleMode],0
	JZ	@@compat2
	 DEC	CL
	 DEC	CH
    @@compat2:
	MOV	ESI,[VBL_TempPal]
	MOV	EDI,[VBL_TempOldPal]

  @@rvsnc1:
	 MOV	EAX,[ESI]
	 MOV	EBX,[EDI]
	 AND	EAX,003F3F3Fh
	 AND	EBX,003F3F3Fh
	 CMP	EAX,EBX
	 JNZ	SHORT @@rvssc3_prep
  @@rvsnc3:
	  ADD	ESI,3
	  ADD	EDI,3
	INC	CL
	CMP	CL,CH
	JNA	SHORT @@rvsnc1
	JMP	SHORT @@rvscont

  @@rvssc3_prep:
	MOV	EDX,3C8h
	MOV	BL,AL
	MOV	AL,CL
	INC	AL
	OUT	DX,AL
	MOV	AL,BL
	INC	EDX
	JMP	SHORT @@rvssc3

  @@rvssc1:
	 MOV	EAX,[ESI]
	 MOV	EBX,[EDI]
	 AND	EAX,003F3F3Fh
	 AND	EBX,003F3F3Fh
	 CMP	EAX,EBX
	 JZ	SHORT @@rvsnc3
  @@rvssc3:
	  STOSB
	  SHR	EAX,8
	  STOSW
	  OUTSB
	  OUTSB
	  OUTSB
	INC	CL
	CMP	CL,CH
	JNA	SHORT @@rvssc1

  @@rvscont:

	XOR	EAX,EAX
	MOV	[_VBL_LastColor],AX
	INC	AH
	MOV	[_VBL_FirstColor],AX

	STI
	; Way to go!!
	CALL	[_VBL_FullHandler]

    @@end:
;	 SetBorder 0, 0, 0
	CLI
	MOV	EAX,[_EAX]
	MOV	EBX,[_EBX]
	MOV	ECX,[_ECX]
	MOV	EDX,[_EDX]
	MOV	ESI,[_ESI]
	MOV	EDI,[_EDI]
	MOV	EBP,[_EBP]
	POP	FS
	POP	GS
	POP	ES
	MOV	[_VBL_Empty],0
	POP	DS
	IRETD


; --------------------------------------------------
; Calculate & initialize the timer.
; SI = Scan lines till secondary timer.
;      0 means no secondary timer.
;  (still have to test repeated calls to Calculate)

PUBLIC VBL_InitializeA_
VBL_InitializeA_:
	PUSH	ECX EDX EBX ESI EDI
	XOR	EAX,EAX
	MOV	[_VBL_Selector],DS
	MOV	[VBLCounter],EAX
	MOV	[VBLVSyncOldV],EAX
	MOV	[SecTimerAct],AL
	MOV	[_VBL_Active],AL
	MOV	[WasFullTick],AL

	CMP	[_VBL_CompatibleMode],0
	JZ 	@@nocompat
	MOV	[IntDelay],0
	MOV	EAX,[_VBL_CompatibleMode]
	IMUL 	EAX,10
	MOV	EBX,EAX
	MOV	EAX,65536*182
	XOR	EDX,EDX
	IDIV	EBX
	MOV	[FullFrame],AX
	JMP	@@storetime

   @@nocompat:
	IN	AL,21h
	PUSH	EAX
	AND	AL,0FEh
	OUT	21h,AL
	STI

	MOV	AL,38h		; 36h
	OUT	43h,AL
	MOV	AL,10
	OUT	40h,AL
	XOR	EAX,EAX
	MOV	[VBLTest],EAX
	OUT	40h,AL
  @@dly: TEST	[VBLTest],0FFFFFFFFh
	 JZ	@@dly
;	 INT	 8
	CLI
	POP	EAX
	OUT	21h,AL

	MOV	DX,3DAh
	IN	AL,DX
	MOV	DX,3C8h
	XOR	AL,AL
	OUT	DX,AL
	INC	DX
	OUT	DX,AL
	OUT	DX,AL
	OUT	DX,AL
	MOV	AX,DS
	MOV	DS,AX
	MOV	DS,AX
	MOV	DS,AX
	XOR	EAX,EAX
	OUT	43h,AL
	IN	AL,40h
	MOV	AH,AL
	IN	AL,40h
	XCHG	AL,AH
	NEG	EAX
	SHR	AX,1
	MOV	[IntDelay],AX

	VSync
	MOV	AL,36h
	OUT	43h,AL
	XOR	AL,AL
	OUT	40h,AL
	XOR	AL,AL
	OUT	40h,AL
	VSync
	XOR	EAX,EAX
	OUT	43h,AL
	IN	AL,40h
	MOV	AH,AL
	IN	AL,40h
	XCHG	AL,AH
	NEG	EAX
	SHR	AX,1
	MOV	[FullFrame],AX
	SUB	AX,[IntDelay]
	SUB	AH,2

   @@storetime:
	MOV	[UsualFrame],AX
	MOV	EBX,EAX
	OR	ESI,ESI
	JZ	SHORT @@nohalf

	MOV	AX,[FullFrame]
	MOV	ECX,ESI
	ADD	ECX,35			; >>>> Value Warning <<<<
	MUL	ECX			; Might change with the videomode.
	MOV	ECX,449 		; >>>> Value Warning <<<<
	DIV	ECX
	MOV	EDX,EBX
	SUB	EDX,EAX
	MOV	[HalfFrame],AX
	MOV	[SecFrame],DX
	MOV	[SecTimerAct],1
   @@nohalf:
	MOV	AL,36h
	OUT	43h,AL
	MOV	AL,BL
	OUT	40h,AL
	MOV	AL,BH
	OUT	40h,AL
	POP	EDI ESI EBX EDX	ECX
RetValue:
	RET


; ------------------------
; Special VSync that waits until EAX frames (at least) have
; occured since you last called it.
; I.e. if you call it every EAX frames, it won't wait. :-)
; If EAX=0, it never waits (useful for flushing the internal
; count, and asking for the number of frames past).
; Returns in EAX the number of frames that have actually passed.

PUBLIC VBL_VSync_
VBL_VSync_:
	PUSH	EBX ECX EDX ESI

	CMP 	[_VBL_OldHandlerOfs],0
	JNZ	@@noret
	CMP 	[_VBL_OldHandlerSeg],0
	JZ	@@ret
   @@noret:
	STI

	MOV	ECX,EAX

	CMP	[_VBL_CompatibleMode],0
	JNZ	@@compat1
	MOV	EDX,3DAh		; Retrace sync.
	IN	AL,DX
	AND	AL,8
	MOV	AH,AL

    @@compat1:
	MOV	EBX,ECX
	ADD	EBX,EBX
	ADD	EBX,4

    @@w:
	MOV	EDX,[VBLCounter]
	SUB	EDX,[VBLVSyncOldV]
	CMP	EDX,ECX
	JNC	SHORT @@c
	CMP	[_VBL_CompatibleMode],0
	JNZ	@@w
	MOV	EDX,3DAh		; Retrace sync.
	IN	AL,DX
	AND	AL,8
	CMP	AH,AL
	JZ	SHORT @@w
	MOV	AH,AL
	DEC	EBX
	JNZ	SHORT @@w

	VSync
	MOVZX	EBX,[FullFrame]
	CLI
	MOV	AL,36h
	OUT	43h,AL
	MOV	AL,BL
	OUT	40h,AL
	MOV	AL,BH
	OUT	40h,AL
	STI
	MOV	EDX,ECX
	ADD	EDX,2

    @@c:
	MOV	EAX,EDX
	MOV	EDX,[VBLCounter]
	MOV	[VBLVSyncOldV],EDX
    @@ret:
	POP	ESI EDX ECX EBX
	RET

; ------------------------
; Change a portion of the VGA palette.

PUBLIC VBL_DumpPalette_
VBL_DumpPalette_:
	PUSH	EDX
	PUSH	ECX EDI
	JCXZ	SHORT @@ret
	LEA	EDI,[2*EDI+EDI]
	ADD	EDI,[_VBL_Palette]
	LEA	ECX,[2*ECX+ECX]
	REP MOVSB
	POP	EDI ECX
	MOVSX	EAX,[_VBL_FirstColor]
	CMP	EDI,EAX
	JNL	SHORT @@1
	  MOV	[_VBL_FirstColor],DI
    @@1:MOVSX	EAX,[_VBL_LastColor]
	MOV	EDX,EDI
	ADD	EDX,ECX
	DEC	EDX
	CMP	EDX,EAX
	JNG	SHORT @@2
	  MOV	[_VBL_LastColor],DX
    @@2:
  @@ret:POP	EDX
	RET

; ------------------------
; Zero the VGA palette.

PUBLIC VBL_ZeroPalette_
VBL_ZeroPalette_:
	PUSH	ECX EDI	EAX
	MOV	EDI,[_VBL_Palette]
	MOV	ECX,768
	XOR	EAX,EAX
	REP STOSB
	MOV	[_VBL_FirstColor],0
	MOV	[_VBL_LastColor],255
	POP	EAX EDI ECX
	RET

; ------------------------
; Set the VGA page address (EAX) on the next VBL.

PUBLIC VBL_ChangePage_
VBL_ChangePage_:
	MOV	[ChangePageVal],AX
	MOV	[ChangePageFlag],1
	RET


; -----------------------
; Restore system time from CMOS registers.
; Just to be nice.

PUBLIC VBL_RestoreSystemTime_
VBL_RestoreSystemTime_:
	PUSH	EBX ECX EDX ESI EDI

	XOR	AL,AL
	OUT	70h,AL
	IN	AL,71h
	MOV	DH,AL
	AND	DH,15
	SHR	AL,4
	MOV	DL,10
	MUL	DL
	ADD	DH,AL

	MOV	AL,2
	OUT	70h,AL
	IN	AL,71h
	MOV	CL,AL
	AND	CL,15
	SHR	AL,4
	MOV	DL,10
	MUL	DL
	ADD	CL,AL

	MOV	AL,4
	OUT	70h,AL
	IN	AL,71h
	MOV	CH,AL
	AND	CH,15
	SHR	AL,4
	MOV	DL,10
	MUL	DL
	ADD	CH,AL

	XOR	DL,DL
;	 MOV	 [v86r_cx],CX
;	 MOV	 [v86r_dx],DX
;	 MOV	 [v86r_ah],2Dh
;	 MOV	 AL,21h
;	 INT	 33h
	MOV	AH,2Dh
	INT	21h

	MOV	AL,7
	OUT	70h,AL
	IN	AL,71h
	MOV	DL,AL
	AND	DL,15
	SHR	AL,4
	MOV	CH,10
	MUL	CH
	ADD	DL,AL

	MOV	AL,8
	OUT	70h,AL
	IN	AL,71h
	MOV	DH,AL
	AND	DH,15
	SHR	AL,4
	MOV	CH,10
	MUL	CH
	ADD	DH,AL

	MOV	AL,9
	OUT	70h,AL
	IN	AL,71h
	MOV	CL,AL
	AND	CL,15
	SHR	AL,4
	MOV	CH,10
	MUL	CH
	ADD	CL,AL

	XOR	CH,CH
	ADD	CX,1900
;	 MOV	 [v86r_cx],CX
;	 MOV	 [v86r_dx],DX
;	 MOV	 [v86r_ah],2Bh
;	 MOV	 AL,21h
;	 INT	 33h
	MOV	AH,2Bh
	INT	21h

	POP	EDI ESI EDX ECX EBX
	RET


END

