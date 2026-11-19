; ============================================================================
; Project: RAM-Runner (64-bit Dual-Core DOS Engine)
; File:    Main64.asm
; Author:  Stefano Pollazzon (www.github.com/StefanoP85)
; Description: Entry point for the Core 1
; ============================================================================
.code

EXTRN Dos64Main:PROC
PUBLIC EntryPoint

EntryPoint PROC

PauseLoop:
; Allineamento forzato a 16 byte (Garantisce che gli ultimi 4 bit siano 0)
	AND    RSP, 0FFFFFFFFFFFFFFF0h
; Allocazione dello Shadow Space (32 byte) richiesto da MSVC
	SUB    RSP, 32
; Sincronizza RBP
	MOV    RBP, RSP
	JMP    Dos64Main
EntryPoint ENDP

END
