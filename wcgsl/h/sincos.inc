; -------------------------- SINCOS.INC --------------------------
; Trig routines for Watcom C & Tran's PMode.
; 16 bit angles, and the result is given in 2.30 fixed point.

;IFNDEF  _SINCOS_H_
;        _SINCOS_H_ EQU 1

EXTRN __CosTbl: DWORD

EXTRN InitSinCos_ : NEAR                ; Initialize ONCE AND ONLY ONCE.

ANGLEDIVISOR = 16-12
ANGLEINC = 1 SHL (ANGLEDIVISOR - 3)     ; ????? que e etto??? <JC>

Cos MACRO dest
        SHR     EBX,ANGLEDIVISOR-2
        AND     EBX,3FFCh
        MOV     &dest,CS:[__CosTbl+EBX]
ENDM

Sin MACRO dest
        ADD     EBX,4000h
        Cos     &dest
ENDM

SinCos MACRO sin, cos
        Cos     &cos
        ADD     EBX,1000h
        AND     EBX,3FFCh
        MOV     &sin,CS:[__CosTbl+EBX]
ENDM

; Fixed point multiplication 2.30*XX.YY = XX.YY

FixMul MACRO parm
        IMUL    &parm
        SHRD    EAX,EDX,30
ENDM


;ENDIF

; --------------------- End of SINCOS.INC ------------------------

