; ----------------------- ROT3DA.ASM -------------------------
; For use with WATCOM 9.5 + DOS4GW
; (C) Copyright 1993/4 by Jare & JCAB of Iguana.

; 3D basic routines.

        .386P
        .MODEL FLAT
        LOCALS @@


        INCLUDE SINCOS.INC

; ========================================================
; a = Roll(Z), b = Pitch(X), c = Yaw(Y)
; /                                                                \
; |  COSc*COSa+SINc*SINb*SINa -COSc*SINa+SINc*SINb*COSa  SINc*COSb |
; |  COSb*SINa                 COSb*COSa                -SINb      | ð RM
; | -SINc*COSa+COSc*SINb*SINa  SINc*SINa+COSc*SINb*COSa  COSc*COSb |
; \                                                                /

        DATASEG

cosA DD 0
cosB DD 0
cosC DD 0
sinA DD 0
sinB DD 0
sinC DD 0
sinBsinA DD 0
sinBcosA DD 0

add1 DD 0
add2 DD 0

;PUBLIC R3D_TRotMatrix R3D_MIdentity;
;PUBLIC R3D_TPosVector R3D_PZero;

PUBLIC _R3D_MIdentity, _R3D_PZero
PUBLIC _R3D_FocusX, _R3D_FocusY, _R3D_CenterX, _R3D_CenterY

_R3D_MIdentity  DD 40000000h,0,0, 0,40000000h,0, 0,0,40000000h
_R3D_PZero      DD 0,0,0

_R3D_FocusX  DD 256
_R3D_FocusY  DD 256*5/6
_R3D_CenterX DD 160
_R3D_CenterY DD 100


        CODESEG

; extern void R3D_CrossProduct(R3D_PPosVector dest, R3D_PPosVector u,
;                              R3D_PPosVector v, int prec);
; #pragma aux R3D_CrossProduct parm caller [EDI] [ESI] [EBX] [ECX]
PUBLIC R3D_CrossProduct_
R3D_CrossProduct_:
        PUSH    EDX EBP

        MOV     EAX,[ESI+4]             ; Uy
        IMUL    DWORD PTR [EBX+8]       ; UyúVz
        SHRD    EAX,EDX,CL
        MOV     EBP,EAX
        MOV     EAX,[ESI+8]             ; Uz
        IMUL    DWORD PTR [EBX+4]       ; UzúVy
        SHRD    EAX,EDX,CL
        SUB     EBP,EAX                 ; UyúVz - UzúVy
        MOV     [EDI+0],EBP

        MOV     EAX,[ESI+8]             ; Uz
        IMUL     DWORD PTR [EBX+0]       ; UzúVx
        SHRD    EAX,EDX,CL
        MOV     EBP,EAX
        MOV     EAX,[ESI+0]             ; Ux
        IMUL     DWORD PTR [EBX+8]       ; UxúVz
        SHRD    EAX,EDX,CL
        SUB     EBP,EAX                 ; - (UxúVz - UzúVx)
        MOV     [EDI+4],EBP

        MOV     EAX,[ESI+0]             ; Ux
        IMUL     DWORD PTR [EBX+4]       ; UxúVy
        SHRD    EAX,EDX,CL
        MOV     EBP,EAX
        MOV     EAX,[ESI+4]             ; Uy
        IMUL     DWORD PTR [EBX+0]       ; UyúVx
        SHRD    EAX,EDX,CL
        SUB     EBP,EAX                 ; UxúVy - UyúVx
        MOV     [EDI+8],EBP

        POP     EBP EDX
        RET


; extern void R3D_Gen3DMatrix(R3D_PRotMatrix dest, R3D_PAngleValue angs);
; #pragma aux R3D_Gen3DMatrix parm caller [EDI] [ESI]
PUBLIC R3D_Gen3DMatrix_
R3D_Gen3DMatrix_:
        PUSH    EBX ECX EDX EAX
        MOV     BX,[ESI+4]
        SinCos  EAX, EDX
        MOV     [cosC],EDX
        MOV     [sinC],EAX

        MOV     BX,[ESI]
        SinCos  EAX, EDX
        MOV     [cosA],EDX
        MOV     [sinA],EAX

        MOV     BX,[ESI+2]
        SinCos  EAX, EDX
        MOV     [cosB],EDX
        MOV     [sinB],EAX

        MOV     ECX,EAX
        FixMul  <DWORD PTR [sinA]>
        MOV     [sinBsinA],EAX
        MOV     EAX,[cosA]
        FixMul  ECX
        MOV     [sinBcosA],EAX

        MOV     EAX,[cosC]
        FixMul  <DWORD PTR [cosA]>
        MOV     ECX,EAX
        MOV     EAX,[sinC]
        FixMul  <DWORD PTR [sinBsinA]>
        ADD     EAX,ECX
        STOSD          

        MOV     EAX,[cosC]
        FixMul  <DWORD PTR [sinA]>
        MOV     ECX,EAX
        MOV     EAX,[sinC]
        FixMul  <DWORD PTR [sinBcosA]>
        SUB     EAX,ECX
        STOSD          

        MOV     EAX,[sinC]
        FixMul  <DWORD PTR [cosB]>
        STOSD          

        MOV     EAX,[cosB]
        FixMul  <DWORD PTR [sinA]>
        STOSD          

        MOV     EAX,[cosB]
        FixMul  <DWORD PTR [cosA]>
        STOSD          

        MOV     EAX,[sinB]
        NEG     EAX
        STOSD

        MOV     EAX,[sinC]
        FixMul  <DWORD PTR [cosA]>
        MOV     ECX,EAX
        MOV     EAX,[cosC]
        FixMul  <DWORD PTR [sinBsinA]>
        SUB     EAX,ECX
        STOSD          

        MOV     EAX,[sinC]
        FixMul  <DWORD PTR [sinA]>
        MOV     ECX,EAX
        MOV     EAX,[cosC]
        FixMul  <DWORD PTR [sinBcosA]>
        ADD     EAX,ECX
        STOSD          

        MOV     EAX,[cosC]
        FixMul  <DWORD PTR [cosB]>
        STOSD          
                        
        POP     EAX EDX ECX EBX
        RET

; ========================================================
; extern void R3D_Gen3DInverseMatrix(R3D_PRotMatrix dest, R3D_PAngleValue angs);
; #pragma aux R3D_Gen3DInverseMatrix parm caller [EDI] [ESI]
PUBLIC R3D_Gen3DInverseMatrix_
R3D_Gen3DInverseMatrix_:
        PUSH    EBX ECX EDX EAX
        MOV     BX,[ESI+4]
        SinCos  EAX, EDX
        MOV     [cosC],EDX
        MOV     [sinC],EAX

        MOV     BX,[ESI]
        SinCos  EAX, EDX
        MOV     [cosA],EDX
        MOV     [sinA],EAX

        MOV     BX,[ESI+2]
        SinCos  EAX, EDX
        MOV     [cosB],EDX
        MOV     [sinB],EAX

        MOV     ECX,EAX
        FixMul  <DWORD PTR [sinA]>
        MOV     [sinBsinA],EAX
        MOV     EAX,[cosA]
        FixMul  ECX
        MOV     [sinBcosA],EAX

        MOV     EAX,[cosC]
        FixMul  <DWORD PTR [cosA]>
        MOV     ECX,EAX
        MOV     EAX,[sinC]
        FixMul  <DWORD PTR [sinBsinA]>
        ADD     EAX,ECX
        MOV     [EDI+4*(3*0+0)],EAX

        MOV     EAX,[cosC]
        FixMul  <DWORD PTR [sinA]>
        MOV     ECX,EAX
        MOV     EAX,[sinC]
        FixMul  <DWORD PTR [sinBcosA]>
        SUB     EAX,ECX
        MOV     [EDI+4*(3*1+0)],EAX

        MOV     EAX,[sinC]
        FixMul  <DWORD PTR [cosB]>
        MOV     [EDI+4*(3*2+0)],EAX

        MOV     EAX,[cosB]
        FixMul  <DWORD PTR [sinA]>
        MOV     [EDI+4*(3*0+1)],EAX

        MOV     EAX,[cosB]
        FixMul  <DWORD PTR [cosA]>
        MOV     [EDI+4*(3*1+1)],EAX

        MOV     EAX,[sinB]
        NEG     EAX
        MOV     [EDI+4*(3*2+1)],EAX

        MOV     EAX,[sinC]
        FixMul  <DWORD PTR [cosA]>
        MOV     ECX,EAX
        MOV     EAX,[cosC]
        FixMul  <DWORD PTR [sinBsinA]>
        SUB     EAX,ECX
        MOV     [EDI+4*(3*0+2)],EAX

        MOV     EAX,[sinC]
        FixMul  <DWORD PTR [sinA]>
        MOV     ECX,EAX
        MOV     EAX,[cosC]
        FixMul  <DWORD PTR [sinBcosA]>
        ADD     EAX,ECX
        MOV     [EDI+4*(3*1+2)],EAX

        MOV     EAX,[cosC]
        FixMul  <DWORD PTR [cosB]>
        MOV     [EDI+4*(3*2+2)],EAX
                        
        POP     EAX EDX ECX EBX
        RET

; ========================================================
; extern void R3D_Rot3DMatrix(R3D_PRotMatrix dest, R3D_PRotMatrix m1, R3D_PRotMatrix m2);
; #pragma aux R3D_Rot3DMatrix parm caller [EDI] [ESI] [EBX]
PUBLIC R3D_Rot3DMatrix_
R3D_Rot3DMatrix_:
        PUSH    EDX ECX EAX
        REPT 3
          REPT 3
            MOV     EAX,[ESI]
            FixMul  <DWORD PTR [EBX]>
            MOV     ECX,EAX
            MOV     EAX,[ESI+4]
            FixMul  <DWORD PTR [EBX+12]>
            ADD     ECX,EAX
            MOV     EAX,[ESI+8]
            FixMul  <DWORD PTR [EBX+24]>
            ADD     EAX,ECX
            STOSD
            ADD     EBX,4
          ENDM
          SUB   EBX,12
          ADD   ESI,12
        ENDM
        POP     EAX ECX EDX
        RET


; ========================================================
; extern void R3D_Rot3DVector(R3D_PPosVector dest, R3D_PRotMatrix m, R3D_PPosVector v, int n, int size);
; #pragma aux R3D_Rot3DVector parm caller [EDI] [ESI] [EBX] [ECX] [EDX]
PUBLIC R3D_Rot3DVector_
R3D_Rot3DVector_:
        PUSH    EAX EBP

        MOV     [add1],EDX
    @@l:
        i = 0
        REPT 3
          MOV     EAX,[ESI+12*i+0]
          FixMul  <DWORD PTR [EBX]>
          MOV     EBP,EAX
          MOV     EAX,[ESI+12*i+4]
          FixMul  <DWORD PTR [EBX+4]>
          ADD     EBP,EAX
          MOV     EAX,[ESI+12*i+8]
          FixMul  <DWORD PTR [EBX+8]>
          ADD     EAX,EBP
          MOV     [EDI+4*i],EAX
          i = i + 1
        ENDM
        ADD     EBX,[add1]
        ADD     EDI,[add1]
        LOOP    @@l

        POP     EBP EAX
        RET


; ========================================================
; extern void R3D_Add3DVector(R3D_PPosVector dest, R3D_PPosVector v1, int n, int size);
; #pragma aux R3D_Add3DVector parm caller [EDI] [ESI] [ECX] [EBX]
PUBLIC R3D_Add3DVector_
R3D_Add3DVector_:
        PUSH    EBP EDX

        MOV     EDX,[ESI+0]
        MOV     EBP,[ESI+4]
        MOV     ESI,[ESI+8]

     @@l:
        ADD     [EDI+0],EDX
        ADD     [EDI+4],EBP
        ADD     [EDI+8],ESI
        ADD     EDI,EBX
        LOOP    @@l

        POP     EDX EBP
        RET

; ========================================================
; extern void R3D_Project3D(R3D_PProjPos dest, R3D_PPosVector v, int n, int size1, int size2);
; #pragma aux R3D_Project3D   parm caller [EDI] [ESI] [ECX] [EBX] [EDX]
PUBLIC R3D_Project3D_
R3D_Project3D_:
        PUSH   EBP EAX

        MOV     [add1],EBX
        MOV     [add2],EDX

;        MOV     EBX,256*5/6             ; MCGA aspect ratio....

    @@l:
         MOV    EBP,[ESI+8]
         CMP    EBP,60
         JG     @@calc
          MOV   EBP,60
     @@calc:
         MOV    EAX,[ESI+4]
         NEG    EAX
         MOV    EBX,[_R3D_FocusY]
         CDQ
;         SHLD   EDX,EAX,8
;         SHL    EAX,8
         IMUL   EBX
         IDIV   EBP
         ADD    EAX,[_R3D_CenterY]
         MOV    [EDI+4],EAX

         MOV    EAX,[ESI]
         MOV    EBX,[_R3D_FocusX]
         CDQ
;         SHLD   EDX,EAX,8
;         SHL    EAX,8
         IMUL   EBX
         IDIV   EBP
         ADD    EAX,[_R3D_CenterX]
         MOV    [EDI],EAX
        ADD     EDI,[add1]
        ADD     ESI,[add2]
        LOOP    @@l

        POP    EAX EBP
        RET


ENDS
END

; ----------------------- ROT3DA.ASM -------------------------
