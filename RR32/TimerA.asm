; ============================================================================
; Project: RAM-Runner (32-bit DOS Engine)
; File:    TimerA.asm
; Author:  Stefano Pollazzon (www.github.com/StefanoP85)
; Description: Game timer at 60 Hz
; ============================================================================
OPTION CASEMAP:NONE
.686
.MODEL FLAT, C

Clock8253     EQU 1193182
CountDivider  EQU 19886
CountInterval EQU 3 ; 65536 div 19886 = 3
Frequency     EQU 60 ; Clock8253 div CountDivider = ~60 Hz
PIT_Data0     EQU 40h
PIT_Control   EQU 43h
PIT_Select0   EQU 00h
PIT_LSBMSB    EQU 30h
PIT_Mode2     EQU 04h ; Rate Generator

.DATA
UserClockCounter DD 0
IsrCount         DW 0
OldTimerISR      DF 0

.CODE

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE
NewTimerISR PROC
    INC    [UserClockCounter]
    DEC    [IsrCount]
    JNZ    @@SendEOI
    MOV    [IsrCount], CountInterval
    JMP    FWORD PTR [OldTimerISR]
@@SendEOI:
    PUSH   EAX
    MOV    AL, 20h
	OUT    20h, AL
	POP    EAX
	IRETD
NewTimerISR_End:
NewTimerISR_Length EQU (NewTimerISR_End - NewTimerISR)
NewTimerISR ENDP
OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

InstallTimer PROC USES EBX ESI EDI
; Blocca le pagine della ISR
    MOV    EDX, NewTimerISR_Length
    MOV    ESI, EDX
    SHR    ESI, 16                     ; SI = 16 bit alti della dimensione
    MOV    DI, DX                      ; DI = 16 bit bassi della dimensione
    MOV    EDX, OFFSET NewTimerISR
    MOV    EBX, EDX
    SHR    EBX, 16                     ; BX = 16 bit alti dell'indirizzo
    MOV    CX, DX                      ; CX = 16 bit bassi dell'indirizzo
    MOV    AX, 0600h                   ; AX = funzione Lock Linear Region
    INT    31h
    JC     @@Error
; Blocca le pagine della variabile UserClockCounter
    XOR    ESI, ESI                    ; SI = 16 bit alti della dimensione (ZERO)
    MOV    DI, 4                       ; DI = 16 bit bassi della dimensione (4)
    MOV    EDX, OFFSET UserClockCounter
    MOV    EBX, EDX
    SHR    EBX, 16                     ; BX = 16 bit alti dell'indirizzo
    MOV    CX, DX                      ; CX = 16 bit bassi dell'indirizzo
    MOV    AX, 0600h                   ; AX = funzione Lock Linear Region
    INT    31h
    JC     @@Error
; Ottiene l'handler corrente
    MOV    AX, 0204h                   ; AX = funzione Get Protected Mode Interrupt Vector
    MOV    BL, 08h                     ; IRQ 0 (Interrupt 08h)
    INT    31h
    MOV    DWORD PTR [OldTimerISR], EDX
    MOV    WORD PTR [OldTimerISR + 4], CX
; Installa il gestore ISR
    MOV    [IsrCount], CountInterval   ; Importante!
    MOV    AX, 0205h                   ; AX = funzione Set Protected Mode Interrupt Vector
    MOV    BL, 08h                     ; IRQ 0 (Interrupt 08h)
    MOV    CX, CS
    MOV    EDX, OFFSET NewTimerISR
    INT    31h
; Programma il PIT
    MOV    AL, PIT_Select0 or PIT_LSBMSB or PIT_Mode2
    OUT    PIT_Control, AL
    MOV    AX, CountDivider
    OUT    PIT_Data0, AL
    MOV    AL, AH
    OUT    PIT_Data0, AL
; Ritorna con valore 1 = True
    MOV    AL, 01
    JMP    @@Exit

@@Error:
; Ritorna con valore = False
    XOR    AL, AL
@@Exit:
    RET
InstallTimer ENDP

UninstallTimer PROC USES EBX ESI EDI
; Ripristina la frequenza standard del PIT (1193182 Hz / 65536 = ~18.2 Hz)
    MOV    AL, PIT_Select0 or PIT_LSBMSB or PIT_Mode2
    OUT    PIT_Control, AL
    XOR    AL, AL                      ; AX = 0 (Divisore 0 corrisponde a 65536)
    OUT    PIT_Data0, AL
    OUT    PIT_Data0, AL
; Ripristina il vecchio vettore di interrupt (Funzione DPMI 0205h)
    MOV    AX, 0205h                   ; AX = funzione Set Protected Mode Interrupt Vector
    MOV    BL, 08h                     ; IRQ 0 (Interrupt 08h)
    MOV    CX, WORD PTR [OldTimerISR + 4] ; Selettore CS originale
    MOV    EDX, DWORD PTR [OldTimerISR]   ; Offset EIP originale
    INT    31h
; Sblocca le pagine della ISR
    MOV    EDX, NewTimerISR_Length
    MOV    ESI, EDX
    SHR    ESI, 16                     ; SI = 16 bit alti della dimensione
    MOV    DI, DX                      ; DI = 16 bit bassi della dimensione
    MOV    EDX, OFFSET NewTimerISR
    MOV    EBX, EDX
    SHR    EBX, 16                     ; BX = 16 bit alti dell'indirizzo
    MOV    CX, DX                      ; CX = 16 bit bassi dell'indirizzo
    MOV    AX, 0601h                   ; AX = funzione Unlock Linear Region
    INT    31h
; Sblocca le pagine della variabile UserClockCounter (4 byte)
    XOR    ESI, ESI                    ; SI = 0
    MOV    DI, 4                       ; DI = 4 byte
    MOV    EDX, OFFSET UserClockCounter
    MOV    EBX, EDX
    SHR    EBX, 16                     ; BX = 16 bit alti dell'indirizzo
    MOV    CX, DX                      ; CX = 16 bit bassi dell'indirizzo
    MOV    AX, 0601h                   ; AX = funzione Unlock Linear Region
    INT    31h

    RET
UninstallTimer ENDP

PUBLIC UserClockCounter
PUBLIC NewTimerISR
PUBLIC InstallTimer
PUBLIC UninstallTimer

END
