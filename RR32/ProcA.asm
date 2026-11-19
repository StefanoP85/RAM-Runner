; ============================================================================
; Project: RAM-Runner (32-bit DOS Engine)
; File:    ProcA.asm
; Author:  Stefano Pollazzon (www.github.com/StefanoP85)
; Description: Common routines
; ============================================================================
OPTION CASEMAP:NONE
.686
.MODEL FLAT, C

.DATA

.CODE

CallRDTSC PROC USES EBX ESI EDI RegEAX:DWORD, RegEDX:DWORD
	XOR     EAX, EAX
	CPUID
	RDTSC
	MOV     EDI, [RegEAX]
	MOV     DWORD PTR [EDI], EAX
	MOV     EDI, [RegEDX]
	MOV     DWORD PTR [EDI], EDX
	RET
CallRDTSC ENDP

IsPentiumOrNewer PROC USES EBX ESI EDI
	PUSHFD
	POP     EAX
	MOV     ECX, EAX
	XOR     EAX, 00200000h
	PUSH    EAX
	POPFD
	PUSHFD
	POP     EAX
	PUSH    ECX
	POPFD
	; Controlla se il bit 21 ha cambiato valore
	XOR     EAX, ECX
	TEST    EAX, 00200000h
	SETNZ   AL                         ; AL = 1 se il bit è cambiato
	RET
IsPentiumOrNewer ENDP

DpmiAllocate PROC USES EBX ESI EDI BufferSize:DWORD, Handle:DWORD, Buffer:DWORD
    MOV    EBX, [BufferSize]
    MOV    ECX, EBX
    AND    ECX, 0FFFFh
    SHR    EBX, 16                     ; BX:CX = Dimensione in byte (es. 0000h : C000h per 48KB)
    MOV    AX, 0501h                   ; Servizio DPMI: Allocate Linear Memory Block
    INT    31h
    JC     @@Error
    
    ; Ricomponiamo l'indirizzo lineare a 32 bit da BX:CX in EAX
    MOVZX  EAX, BX
    SHL    EAX, 16
    MOV    AX, CX
    MOV    EBX, [Buffer]
    MOV    DWORD PTR [EBX], EAX
    ; Ricomponiamo l'handle a 32 bit da SI:DI in EAX
    MOVZX  EAX, SI
    SHL    EAX, 16
    MOV    AX, DI
    MOV    EBX, [Handle]
    MOV    DWORD PTR [EBX], EAX
    MOV    AL, 1                       ; Ritorna True (1) in caso di successo
    JMP    @@Exit

@@Error:
    XOR    AL, AL                      ; Ritorna False (0) in caso di errore
@@Exit:
    RET
DpmiAllocate ENDP

DpmiDeallocate PROC USES EBX ESI EDI Handle:DWORD
	MOV    AX, 0502h                   ; Servizio DPMI: Free Memory Block
	MOV    SI, WORD PTR [Handle + 2]   ; 16 bit alti dell'handle
	MOV    DI, WORD PTR [Handle]       ; 16 bit bassi dell'handle
	INT    31h
    SETNC  AL
    RET
DpmiDeallocate ENDP

DpmiAllocateConventional PROC USES EBX ESI EDI BufferSize:DWORD, Selector:DWORD, Buffer:DWORD
    MOV    EBX, [BufferSize]
    SHR    EBX, 4                      ; EBX = Dimensione in paragrafi
    MOV    AX, 0100h                   ; Servizio DPMI: Allocate DOS Memory Block
    INT    31h
    JC     @@Error
    
    ; AX = segmento real-mode, DX = selettore
    MOVZX  EAX, AX
    SHL    EAX, 4
    MOV    EBX, [Selector]
    MOV    WORD PTR [EBX], DX
    MOV    EBX, [Buffer]
    MOV    DWORD PTR [EBX], EAX
    MOV    AL, 1                       ; Ritorna True (1) in caso di successo
    JMP    @@Exit

@@Error:
    XOR    AL, AL                      ; Ritorna False (0) in caso di errore
@@Exit:
    RET
DpmiAllocateConventional ENDP

DpmiDeallocateConventional PROC USES EBX ESI EDI Selector:WORD
	MOV    AX, 0101h                   ; Servizio DPMI: Free DOS Memory Block
	MOV    DX, [Selector]              ; 16 bit bassi dell'handle
	INT    31h
    SETNC  AL
    RET
DpmiDeallocateConventional ENDP

DpmiPhysicalAddressMapping PROC USES EBX ESI EDI PhysicalAddress:DWORD, BufferSize:DWORD, LinearAddress:DWORD
    MOV    EBX, [PhysicalAddress]
    MOV    ECX, EBX
    AND    ECX, 0FFFFh
    SHR    EBX, 16                     ; BX:CX = Indirizzo fisico (High:Low)
    MOV    ESI, [BufferSize]
    MOV    EDI, ESI
    AND    EDI, 0FFFFh
    SHR    ESI, 16                     ; SI:DI = Dimensione
    MOV    AX, 0800h                   ; Servizio DPMI: Map Physical Address
    INT    31h
    JC     @@Error
    
    ; Restituisce l'indirizzo lineare mappato in BX:CX
    MOVZX  EAX, BX
    SHL    EAX, 16
    MOV    AX, CX
    MOV    EBX, [LinearAddress]
    MOV    DWORD PTR [EBX], EAX
    MOV    AL, 1
    JMP    @@Exit

@@Error:
    XOR    AL, AL
@@Exit:
    RET
DpmiPhysicalAddressMapping ENDP

DpmiFreePhysicalAddress PROC USES EBX ESI EDI LinearAddress:DWORD
    MOV    EBX, [LinearAddress]
    MOV    ECX, EBX
    AND    ECX, 0FFFFh
    SHR    EBX, 16                     ; BX:CX = Indirizzo fisico (High:Low)
    MOV    AX, 0801h                   ; Servizio DPMI: Map Physical Address
    INT    31h
    SETNC  AL
    RET
DpmiFreePhysicalAddress ENDP

DpmiLockRegion PROC USES EBX ESI EDI Address:DWORD, RegionSize:DWORD
    MOV    EBX, [Address]
    MOV    ECX, [RegionSize]
    MOV    ESI, ECX
    SHR    ESI, 16
    MOV    DI, CX
    MOV    CX, BX
    SHR    EBX, 16
    MOV    AX, 0600h
    INT    31h
    SETNC  AL
    RET
DpmiLockRegion ENDP

DpmiUnlockRegion PROC USES EBX ESI EDI Address:DWORD, RegionSize:DWORD
    MOV    EBX, [Address]
    MOV    ECX, [RegionSize]
    MOV    ESI, ECX
    SHR    ESI, 16
    MOV    DI, CX
    MOV    CX, BX
    SHR    EBX, 16
    MOV    AX, 0601h
    INT    31h
    SETNC  AL
    RET
DpmiUnlockRegion ENDP

DpmiSimulateRealModeInterrupt PROC USES EBX ESI EDI IntNo:BYTE, RegsPtr:DWORD
    MOV    BL, [IntNo]                 ; BL = Numero dell'interrupt da chiamare (es. 10h)
    XOR    CX, CX                      ; CX = 0 (nessun parametro da copiare nello stack RM)
    MOV    EDI, [RegsPtr]              ; EDI = Puntatore alla struttura RegsPtr
    MOV    AX, 0300h                   ; Servizio DPMI: Simulate Real Mode Interrupt
    INT    31h
    SETNC  AL
    RET
DpmiSimulateRealModeInterrupt ENDP

DoubleToInt32 PROC DoubleValue:QWORD
    FLD   QWORD PTR [DoubleValue]
    SUB   ESP, 4
    FISTP DWORD PTR [ESP]
    POP   EAX
    RET
DoubleToInt32 ENDP

DoubleToInt64 PROC DoubleValue:QWORD
    FLD   QWORD PTR [DoubleValue]
    SUB   ESP, 8
    FISTP QWORD PTR [ESP]
    POP   EAX
    POP   EDX
    RET
DoubleToInt64 ENDP

PUBLIC CallRDTSC
PUBLIC IsPentiumOrNewer
PUBLIC DpmiAllocate
PUBLIC DpmiDeallocate
PUBLIC DpmiAllocateConventional
PUBLIC DpmiDeallocateConventional
PUBLIC DpmiPhysicalAddressMapping
PUBLIC DpmiFreePhysicalAddress
PUBLIC DpmiLockRegion
PUBLIC DpmiUnlockRegion
PUBLIC DpmiSimulateRealModeInterrupt
PUBLIC DoubleToInt32
PUBLIC DoubleToInt64

END
