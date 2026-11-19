; ============================================================================
; Project: RAM-Runner (64-bit Dual-Core DOS Engine)
; File:    AdLibA.asm
; Author:  Stefano Pollazzon (www.github.com/StefanoP85)
; Description: AdLib and Sound Blaster handling
; ============================================================================
OPTION CASEMAP:NONE
.686
.MODEL FLAT, C

.DATA

.CODE

; Lettura da OPL2
ReadAdLib PROC
	MOV    DX, 388h
	IN     AL, DX

	RET
ReadAdLib ENDP

; Scrittura sicura con ritardo hardware OPL2
WriteAdLib PROC Reg:DWORD, Value:DWORD
	MOV    DX, 388h
	MOV    AL, BYTE PTR [Reg]
	OUT    DX, AL

	REPT 6
		IN AL, DX
	ENDM

	INC    DX                          ; Porta 389h
	MOV    AL, BYTE PTR [Value]
	OUT    DX, AL

	REPT 35
		IN AL, DX
	ENDM

	RET
WriteAdLib ENDP

PUBLIC ReadAdLib
PUBLIC WriteAdLib

END
