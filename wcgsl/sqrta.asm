; ----------------------------- SQRTA.ASM -------------------
; For use with WATCOM 9.5 + DOS4GW
; (C) Copyright 1993/4 by Jare & JCAB of Iguana.
; Original code by Jose Juan Garcia Quesada

        .386P
        .MODEL FLAT
        LOCALS @@

        CODESEG

PUBLIC SQR_Sqrt16_
SQR_Sqrt16_:
        PUSH    ECX
        MOV     ECX,32          ; Number of iterations
        JMP     DoSqrt

PUBLIC SQR_Sqrt_
SQR_Sqrt_:
        PUSH    ECX
        MOV     ECX,16          ; Number of iterations
DoSqrt:
        PUSH    EBP EBX EDX

        MOV     EDX,EAX         ; Argument into dx
        XOR     EBX,EBX         ; Clear the remainder
        XOR     EAX,EAX         ; Clear trial value and final result store
@@l:
        SHL     EAX,1           ; Double partial result
        RCL     EBP,1           ; Store outgoing bits for later retrieval.
        INC     EAX             ; Guess next bit is a 1
        SHLD    EBX,EDX,2       ; Fetch 2 new bits
        SHL     EDX,2           ; From argument
        SUB     EBX,EAX         ; Do a trial subtraction
        JNC     @@c1            ; Guess was right -> append a 1 bit
         ADD    EBX,EAX         ; Guess was wrong, put it back
         DEC    EAX             ; And clean up for next pass
         LOOP   @@l
         JMP    @@c2            ; Go scale result
    @@c1:
        INC     EAX             ; Convert xxxx01 to xxxx10, i.e. append a 1 bit
        LOOP    @@l
    @@c2:
        SHRD    EAX,EBP,1       ; Divide by 2 to get actual square root

        POP     EDX EBX EBP
        POP     ECX
        RET

END

; ----------------------------- SQRTA.ASM -------------------

