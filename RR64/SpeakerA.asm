; ============================================================================
; Project: RAM-Runner (64-bit Dual-Core DOS Engine)
; File:    SpeakerA.asm
; Author:  Stefano Pollazzon (www.github.com/StefanoP85)
; Description: PC Speaker management
; ============================================================================
OPTION CASEMAP:NONE
.686
.MODEL FLAT, C

.DATA

.CODE

SpeakerStartSound PROC Frequency: DWORD
	MOV    AL, 0B6h
	OUT    43h, AL
	MOV    DX, 12h
	MOV    AX, 34DEh                   ; DX:AX = 1234DEh (1193182)
	IDIV   WORD PTR [Frequency]        ; AX = 1193182 / Frequency
	OUT    42h, AL
	MOV    AL, AH
	OUT    42h, AL
	IN     AL, 61h
	OR     AL, 03h
	OUT    61h, AL
	RET
SpeakerStartSound ENDP

SpeakerStopSound PROC
	IN     AL, 61h
	AND    AL, (NOT 03h)
	OUT    61h, AL
	RET
SpeakerStopSound ENDP

PUBLIC SpeakerStartSound
PUBLIC SpeakerStopSound

END
