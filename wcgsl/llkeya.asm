; -------------------------- LLKEYA.ASM -------------------------
; For use with Watcom C 9.5 and DOS4GW
; (C) Copyright 1993-4 by Jare & JCAB of Iguana.

; Based on code bye Patch/Avalanche. Most of it is not modified, except
; the strange behaviour of the _numkeypress variable. Now it does reflect
; the real number of keys pressed (as far as the keyboard can tell, which
; by no means is complete: many combinations fail). To know how many keys
; have been pressed since some time, use the timer along with this.

        LOCALS @@
        .386P

        INCLUDE VGA.INC

kSYSREQ         EQU     054h
kCAPSLOCK       EQU     03Ah
kNUMLOCK        EQU     045h
kSCROLLLOCK     EQU     046h
kLEFTCTRL       EQU     01Dh
kLEFTALT        EQU     038h
kLEFTSHIFT      EQU     02Ah
kRIGHTCTRL      EQU     09Dh
kRIGHTALT       EQU     0B8h
kRIGHTSHIFT     EQU     036h
kESC            EQU     001h
kBACKSPACE      EQU     00Eh
kENTER          EQU     01Ch
kSPACE          EQU     039h
kTAB            EQU     00Fh
kF1             EQU     03Bh
kF2             EQU     03Ch
kF3             EQU     03Dh
kF4             EQU     03Eh
kF5             EQU     03Fh
kF6             EQU     040h
kF7             EQU     041h
kF8             EQU     042h
kF9             EQU     043h
kF10            EQU     044h
kF11            EQU     057h
kF12            EQU     058h
kA              EQU     01Eh
kB              EQU     030h
kC              EQU     02Eh
kD              EQU     020h
kE              EQU     012h
kF              EQU     021h
kG              EQU     022h
kH              EQU     023h
kI              EQU     017h
kJ              EQU     024h
kK              EQU     025h
kL              EQU     026h
kM              EQU     032h
kN              EQU     031h
kO              EQU     018h
kP              EQU     019h
kQ              EQU     010h
kR              EQU     013h
kS              EQU     01Fh
kT              EQU     014h
kU              EQU     016h
kV              EQU     02Fh
kW              EQU     011h
kX              EQU     02Dh
kY              EQU     015h
kZ              EQU     02Ch
k1              EQU     002h
k2              EQU     003h
k3              EQU     004h
k4              EQU     005h
k5              EQU     006h
k6              EQU     007h
k7              EQU     008h
k8              EQU     009h
k9              EQU     00Ah
k0              EQU     00Bh
kMINUS          EQU     00Ch
kEQUAL          EQU     00Dh
kLBRACKET       EQU     01Ah
kRBRACKET       EQU     01Bh
kSEMICOLON      EQU     027h
kTICK           EQU     028h
kAPOSTROPHE     EQU     029h
kBACKSLASH      EQU     02Bh
kCOMMA          EQU     033h
kPERIOD         EQU     034h
kSLASH          EQU     035h
kINS            EQU     0D2h
kDEL            EQU     0D3h
kHOME           EQU     0C7h
kEND            EQU     0CFh
kPGUP           EQU     0C9h
kPGDN           EQU     0D1h
kLARROW         EQU     0CBh
kRARROW         EQU     0CDh
kUARROW         EQU     0C8h
kDARROW         EQU     0D0h
kKEYPAD0        EQU     052h
kKEYPAD1        EQU     04Fh
kKEYPAD2        EQU     050h
kKEYPAD3        EQU     051h
kKEYPAD4        EQU     04Bh
kKEYPAD5        EQU     04Ch
kKEYPAD6        EQU     04Dh
kKEYPAD7        EQU     047h
kKEYPAD8        EQU     048h
kKEYPAD9        EQU     049h
kKEYPADDEL      EQU     053h
kKEYPADSTAR     EQU     037h
kKEYPADMINUS    EQU     04Ah
kKEYPADPLUS     EQU     04Eh
kKEYPADENTER    EQU     09Ch
kPRTSC          EQU     0B7h
kCTRLPRTSC      EQU     0B7h
kSHIFTPRTSC     EQU     0B7h
kKEYPADSLASH    EQU     0B5h
kCTRLBREAK      EQU     0C6h
kPAUSE          EQU     0C5h

_DATA SEGMENT BYTE PUBLIC USE32 'DATA'

PUBLIC _LLK_Keys, _LLK_NumKeys, _LLK_SpacePressed, _LLK_LastScan
PUBLIC _LLK_ChainChange, _LLK_DoChain, _LLK_Autorepeat

EXTRN _LLK_OldHandler : FWORD


_LLK_Keys         DB 256 DUP(0)
_LLK_NumKeys      DB 0
_LLK_SpacePressed DB 0
_LLK_LastScan     DB 0
_LLK_ChainChange  DB 1          ; Allow user to select chaining behaviour.
_LLK_DoChain      DB 0          ; Don't chain by default.
_LLK_Autorepeat   DB 0          ; No autorepeat by default.
e0flag            DB 0
e1flag            DB 0
_DATA ENDS
DGROUP GROUP _DATA

_TEXT SEGMENT PARA PUBLIC USE32 'CODE'
ASSUME CS:_TEXT, DS:DGROUP

PUBLIC LLK_NewInt9_
LLK_NewInt9_:
                CLI
                PUSH    EAX EBX DS

                MOV     AX,DGROUP
                MOV     DS,AX

                XOR     EBX,EBX
                IN      AL,60h
                MOV     BL,AL

                CMP     [e1flag],0      ; Was e1h?
                JZ      @@checke1       
                MOV     [e1flag],0      ; Clear it
                MOV     [e0flag],128    ; Treat second e1h code as an e0h code.
                JMP     @@already       ; Skip first of the two e1h codes.
@@checke1:
                CMP     BL,0E1h         ; was it an E1 key?
                JNE     SHORT @@checke0
                MOV     [e1flag],1
                JMP     @@already
@@checke0:
                CMP     BL,0E0h         ; was it an E0 key?
                JNE     @@setscancode

; E0 key routine
                MOV     [e0flag],128
                JMP     @@already

@@setscancode:	CMP	[e0flag],0
		JZ	@@noe0bynow
                  ; Ignore codes for 104- arrow keys.
		 CMP	BL,02Ah
		 JZ	@@skipe0
		 CMP	BL,0AAh
		 JZ	@@skipe0
		 CMP	BL,036h
		 JZ	@@skipe0
		 CMP	BL,0B6h
		 JZ	@@skipe0
		 JMP	@@noe0bynow
	@@skipe0:
		MOV 	[e0flag],0	; Clear it.
		JMP	@@c3
	@@noe0bynow:
		MOV     AL,BL                   ; save scan code
                AND     BL,01111111b
                ADD     BL,[e0flag]
                AND     AL,10000000b            ; keep break bit, if set
                XOR     AL,10000000b            ; flip bit - 1 means pressed
                                                ;          - 0 means released
                ROL     AL,1                    ; put it in bit 0
                MOV     AH,AL
                MOV     [e0flag],0              ; set E0 to 0
                CMP     AL,_LLK_Keys[EBX]       ; If already pressed,
                JZ      SHORT @@already         ; don't increment keynumpress.
                 MOV    _LLK_Keys[EBX],AL       ; set index for key
                 TEST   AL,AL                   ; Key released?
                 JZ     SHORT @@c1
                 CMP    [_LLK_ChainChange],0    ; Allow changes to DoChain?
                 JZ     @@nochg                 ; Not, so skip.
                 CMP    BL,kSCROLLLOCK          ; User wants DoChain change?
                 JNZ    @@nochg
                 XOR    [_LLK_DoChain],1        ; Your wishes are my orders.
        @@nochg:
                 MOV    [_LLK_LastScan],BL      ; Store scan of last keydown.
                 CMP    BL,kSPACE               ; Should render this obsolete.
                 JZ     SHORT @@c2
                 CMP    BL,kESC
                 JNZ    SHORT @@c1
             @@c2:
                  MOV   [_LLK_SpacePressed],BL
             @@c1:
                 SHL    AL,1                    ; set to 2 or 0
                 DEC    AL                      ; 1 = press, -1 = release
                 ADD    [_LLK_NumKeys],AL       ; inc or dec keypress
       @@already:
                TEST    AH,AH                   ; Released?
                JZ      @@c3
                CMP     [_LLK_Autorepeat],0     ; Allow autorepeat?
                JZ      @@c3
                MOV     [_LLK_LastScan],BL      ; Store scan of last keydown.
          @@c3:
                CMP     [_LLK_DoChain],0        ; Shall I chain to BIOS?
                JZ      @@eoi                   ; Not, so jump.
                PUSHF
                CALL    [_LLK_OldHandler]
                JMP     @@bye
        @@eoi:
                MOV     AL,20H          ; Send generic EOI to PIC
                OUT     20H,AL          ; 001 00 000
                                        ;  |   |  |
                                        ;  |   |  +--- INT request level
                                        ;  |   +------ OCW2
                                        ;  +---------- non-specific EOI command
        @@bye:
                POP     DS EBX EAX
                IRETD
ENDS
                END

; -------------------------- LLKEYA.ASM -------------------------

