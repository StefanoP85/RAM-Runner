; ============================================================================
; Project: RAM-Runner (32-bit DOS Engine)
; File:    VideoA.asm
; Author:  Stefano Pollazzon (www.github.com/StefanoP85)
; Description: 32 bit video handling
; ============================================================================
OPTION CASEMAP:NONE
.686
.MODEL FLAT, C

.DATA

.CODE

Clamp PROC USES EBX Value:DWORD, Min:DWORD, Max:DWORD
    MOV    EAX, [Value]

    MOV    ECX, [Min]
    CMP    EAX, ECX
    SETL   BL                          ; Se Value < Min allora BL = 1 altrimenti BL = 0
    MOVZX  EBX, BL
    NEG    EBX
    MOV    EDX, EBX
    AND    EDX, ECX
    NOT    EBX
    AND    EAX, EBX
    OR     EAX, EDX

    MOV    ECX, [Max]
    CMP    EAX, ECX
    SETG   BL                          ; Se Value > Max allora BL = 1 altrimenti BL = 0
    MOVZX  EBX, BL
    NEG    EBX
    MOV    EDX, EBX
    AND    EDX, ECX
    NOT    EBX
    AND    EAX, EBX
    OR     EAX, EDX

    RET
Clamp ENDP

ClearFrameBufferX32 PROC USES EBX ESI EDI Destination:DWORD
	MOV    EDI, [Destination]
	XOR    EAX, EAX
	MOV    ECX, 1024 * 768
	CLD
	REP    STOSD
	RET
ClearFrameBufferX32 ENDP

CopyFrameBufferX32 PROC USES EBX ESI EDI Destination:DWORD, Source:DWORD
	MOV    EDI, [Destination]
	MOV    ESI, [Source]
	MOV    ECX, 1024 * 768
	CLD
	REP    MOVSD
	RET
CopyFrameBufferX32 ENDP

DrawSpriteX32 PROC USES EBX ESI EDI Destination:DWORD, Source:DWORD, TransparentMask:DWORD
	MOV    EBX, [TransparentMask]
	MOV    ESI, [Source]
	MOV    EDI, [Destination]
	PUSH   EBP                         ; Libera EBP come registro addizionale, per il numero di righe
	MOV    EBP, 64
@@CopyLoop:
	REPT 64
		MOV    EAX, [ESI]              ; EAX = byte dello sprite
		MOV    ECX, [EDI]              ; ECX = pixel sullo schermo
		MOV    EDX, [EBX]              ; EDX = maschere di trasparenza
		AND    EAX, EDX                ; EAX = AND(sprite, maschere)
		NOT    EDX                     ; EDX = maschere invertire
		AND    ECX, EDX                ; ECX = AND(pixel, NOT(maschere))
		OR     EAX, ECX                ; EAX = OR(AND(sprite, maschere), AND(pixel, NOT(maschere)))
		MOV    [EDI], EAX              ; scrive 4 pixel
		ADD    EBX, 4
		ADD    ESI, 4
		ADD    EDI, 4
	ENDM
	ADD    EDI, 4096 - 64 * 4
	DEC    EBP
	JNZ    @@CopyLoop
	POP    EBP                         ; ripristina EBP
	RET
DrawSpriteX32 ENDP

DrawTileX32 PROC USES EBX ESI EDI Destination:DWORD, Source:DWORD
	MOV    ESI, [Source]
	MOV    EDI, [Destination]
	MOV    ECX, 24
	CLD
@@CopyLoop:
	; Copia 24 pixel
	REPT 24
		MOVSD
	ENDM
	ADD    EDI, 4096 - 24 * 4
	DEC    ECX
	JNZ    @@CopyLoop

	RET
DrawTileX32 ENDP

WaitVerticalRetraceX32 PROC
@@VSync:
	MOV    DX, 03DAh
	IN     AL, DX
	AND    AL, 8
	OR     AL, AL
	JZ     @@VSync
@@NoVSync:
	MOV    DX, 03DAh
	IN     AL, DX
	AND    AL, 8
	OR     AL, AL
	JNZ    @@NoVSync
	RET
WaitVerticalRetraceX32 ENDP

PUBLIC Clamp
PUBLIC ClearFrameBufferX32
PUBLIC CopyFrameBufferX32
PUBLIC DrawSpriteX32
PUBLIC DrawTileX32
PUBLIC WaitVerticalRetraceX32

END
