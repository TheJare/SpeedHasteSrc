; -------------------------- LLSCRA.ASM -------------------------
; For use with Watcom C 9.5 and DOS4GW
; (C) Copyright 1993-4 by Jare & JCAB of Iguana.

        LOCALS @@
        .386P

        INCLUDE VGA.INC

LLSM_DIRECT   = 0
LLSM_VIRTUAL  = 1
LLSM_ADDITIVE = 2

LLSVM_MODE13 = 0
LLSVM_MODEY  = 1
LLSVM_MODEX  = 2
LLSVM_MODEZ  = 3

_DATA SEGMENT BYTE PUBLIC USE32 'DATA'

EXTRN _LLS_Screen_  : DWORD
EXTRN _LLS_Virtual1 : DWORD
EXTRN _LLS_Virtual2 : DWORD

EXTRN _LLS_SizeX   : DWORD
EXTRN _LLS_SizeY   : DWORD
EXTRN _LLS_SizeY   : DWORD
EXTRN _LLS_Size    : DWORD
EXTRN _LLS_VMode   : DWORD
EXTRN _LLS_Mode    : DWORD
EXTRN _LLS_UpdateMinY   : DWORD
EXTRN _LLS_UpdateMaxY   : DWORD

_DATA ENDS

DGROUP GROUP _DATA

_TEXT SEGMENT PARA PUBLIC USE32 'CODE'
ASSUME CS:_TEXT, DS:DGROUP

PUBLIC LLS_UpdateVGA_
LLS_UpdateVGA_:
        PUSH    EDI ESI EBX ECX EDX ES
        PUSH    DS
        POP     ES
        CLD
        SetBitMask 0FFh                 ; ET4000 needs this even in Mode13h.
        MOV     EDI,0A0000h             ; Set registers for the next routines.
        MOV     ESI,[_LLS_Screen_]
        MOV     ECX,[_LLS_Size]
        SHR     ECX,2

        MOV     EAX,[_LLS_Mode]
        CMP     [_LLS_VMode],LLSVM_MODE13
        JNZ     LARGE no13h

          ; Mode 13h special routines.
         CMP    EAX,LLSM_DIRECT
         JZ     LARGE bye               ; Do nothing if DIRECT.
         CMP    EAX,LLSM_VIRTUAL
         JNZ    SHORT @@novirt
@@virt:
         MOV    EAX,[_LLS_UpdateMaxY]
         SUB    EAX,[_LLS_UpdateMinY]
         JBE    bye
         MUL    [_LLS_SizeX]
         MOV    ECX,EAX
         SHR    ECX,2
         MOV    EAX,[_LLS_SizeX]
         MUL    [_LLS_UpdateMinY]
         ADD    ESI,EAX
         ADD    EDI,EAX
         REP MOVSD                      ; Plain MOVSD if VIRTUAL.
         JMP    LARGE bye
      @@novirt:                         ; Intelligent updating of changed bits.
;         PUSH   ECX
         MOV    EDX,4
         LEA    EBX,[EDI-4]
         MOV    EDI,[_LLS_Virtual2]
         SUB    EDI,EDX
         INC ECX
      @@eq:
        DEC ECX
        SUB     EBX,ESI                 ; Prepare EBX for aligning with ESI.
        ADD     EDI,EDX
        REPZ CMPSD                      ; Find a different dword.
        JZ      LARGE @@done            ; JMP if did NOT finish due to CMP.
        INC     ECX
        MOV     EAX,[ESI-4]
        MOV     [EDI-4],EAX
        ADD     EBX,ESI                 ; EBX is -4 since the beginning.
      @@al2:
        i = 0
        REPT 8
                MOV     [EBX],EAX
                ADD     EBX,EDX
                DEC     ECX
                JZ      SHORT @@done
                LODSD
                CMP     [EDI],EAX
                JZ      @@eq
                STOSD
        ENDM
        JMP     @@al2
COMMENT #
          ; Matching loop - find first non-matching dword.
         REPZ CMPSD
         JZ     SHORT @@done    ; Count exhausted?
          ; ECX,EDI and ESI have advanced 1 more than needed, i.e. if there
          ; were no equal words, they would have advanced 1 anyway. Take
          ; this into account later.

          ; Non-matching loop - find first matching dword.
         INC    ECX             ; Undo the extra dec described above.
         MOV    EDX,ECX         ; Save previous counter.
         REPNZ CMPSD            ; Find any MORE differences.
          ; The same applies now, ESI and EDI point to the dword FOLLOWING
          ; the one that matched. And ECX is off by one.
         SUB    ECX,EDX         ; ECX = (-1) * (dwords in this run + 1)
         SUB    EDX,ECX         ; EDX = dwords still remaining - 1.
         LEA    ESI,[ESI+4*ECX] ; Restore the start addresses of the run.
         NEG    ECX
         DEC    ECX
         ADD    EBX,ESI         ; Align EBX (screen) pointer with ESI.
         XCHG   EBX,EDI
         REP MOVSD              ; And update the run.
         XCHG   EBX,EDI
         MOV    ECX,EDX         ; We now skip the (matching) value that caused
                                ; the second CMPS to fail, because we don't
                                ; need it. So, no ECX readjusting, but ESI.
         ADD    ESI,4

         SUB    EBX,ESI         ; Prepare EBX for aligning with ESI.
         JMP    SHORT @@al1
     @@done:
          ; Force update the last dword. Just in case.
         MOV    EAX,[ESI-4]
         MOV    [EBX-4],EAX

          ; Time to update the second buffer.
         MOV    EDI,0A0000h
         MOV    ESI,[_LLS_Virtual2]
         POP    ECX
         REP MOVSD
         JMP    SHORT bye
;#
    @@done:
         JMP SHORT bye

     no13h:
        CMP     EAX,LLSM_DIRECT
        JZ      SHORT bye
        CMP     EAX,LLSM_VIRTUAL
        JNZ     SHORT @@novirt
         MOV    AH,1
      @@l1:
          SetPlanes AH
          MOV   ECX,[_LLS_Size]
          SHR   ECX,4
          REP MOVSD
          SHL   AH,1
          CMP   AH,10h
          JNZ   SHORT @@l1
         JMP    SHORT bye
   @@novirt:

   bye:
        POP     ES EDX ECX EBX ESI EDI
        RET

ENDS
                END

; -------------------------- LLSCRA.ASM -------------------------

