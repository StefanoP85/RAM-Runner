; ============================================================================
; Project: RAM-Runner (64-bit Dual-Core DOS Engine)
; File:    RR64.ASM
; Author:  Stefano Pollazzon (www.github.com/StefanoP85)
; Description: Envelope for the 64 bit code.
; ============================================================================
OPTION CASEMAP:NONE
.586
.MODEL FLAT, C
.DATA

PUBLIC RamRunner64
PUBLIC RamRunner64SIZE

RamRunner64 LABEL BYTE
INCBIN "RR64.BIN"
RamRunner64SIZE DWORD $ - RamRunner64

END
