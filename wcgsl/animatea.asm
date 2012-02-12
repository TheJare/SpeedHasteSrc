; -------------------------- ANIMATEA.ASM -------------------------
; For use with Watcom C 9.5 and DOS4GW
; (C) Copyright 1993-4 by Jare & JCAB of Iguana.

        LOCALS @@
        .386P

_TEXT SEGMENT PARA PUBLIC USE32 'CODE'
ASSUME CS:_TEXT, DS:DGROUP

; PUBLIC void ANIM_doFrame(byte *dest, const byte *from, int nbyt);
; #pragma aux ANIM_doFrame parm [EDI] [ESI] [ECX]

PUBLIC ANIM_doFrame_
ANIM_doFrame_:
        PUSH    EDX EBX EBP
        LEA     EBP,[EDI + ECX]

     @@l:
        MOVZX   EAX,BYTE PTR [ESI]
        MOV     DL,[ESI+1]
        ADD     ESI,2
        OR      EAX,EAX
        JZ      SHORT @@data          ; Shouldn't happen.
        CMP     AL,255
        JNZ     SHORT @@rle
         ADD    AX,[ESI]
         ADD    ESI,3
    @@rle:
        LEA     EBX,[EDI+EAX]
        OR      DL,DL
        JZ      SHORT @@data

     @@rlel:
         i = 0
         REPT 8
                XOR    [EDI+i],DL
                DEC    EAX
                JZ     SHORT @@data
                i = i + 1
         ENDM
         XOR    [EDI+i],DL
         ADD    EDI,9
         DEC    EAX
         JNZ    SHORT @@rlel

    @@data:
        MOV     EDI,EBX
        CMP     EDI,EBP
        JAE     SHORT @@bye

        MOVZX   EAX,BYTE PTR [ESI]
        INC     ESI
        OR      EAX,EAX
        JZ      SHORT @@aga          ; Shouldn't happen.
        LEA     EDX,[ESI+EAX]
        LEA     EBX,[EDI+EAX]
        MOV     ECX,EAX
     @@datal1:
         i = 0
         REPT 8
                MOV    AL,[ESI+i]
                XOR    [EDI+i],AL
                DEC    EAX
                JZ     SHORT @@data
                i = i + 1
         ENDM



        CMP     AL,255
        JNZ     SHORT @@raw
         ADD    AX,[ESI+2]
         ADD    ESI,3
    @@raw:
        ADD     ESI,2
        OR      DL,DL
        JZ      SHORT @@data
     @@rlel:
         i = 0
         REPT 8
                XOR    [EDI+i],DL
                DEC    EAX
                JZ     SHORT @@data
                i = i + 1
         ENDM
         XOR    [EDI+i],DL
         DEC    EAX
         JNZ    SHORT @@rlel



    @@aga:
        CMP     EDI,EBP
        JAE     SHORT @@l

    @@bye:
        POP     EBP EBX EDX
        RET

ENDS
                END

; -------------------------- ANIMATEA.ASM -------------------------

