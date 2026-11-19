; ============================================================================
; Project: RAM-Runner (64-bit Dual-Core DOS Engine)
; File:    Core64A.asm
; Author:  Stefano Pollazzon (www.github.com/StefanoP85)
; Description: 64 bit video handling
; ============================================================================
OPTION CASEMAP:NONE
.DATA

TRegs STRUCT
    RegEAX DD ?
    RegEBX DD ?
    RegECX DD ?
    RegEDX DD ?
TRegs ENDS

.CODE

; extern "C" uint32_t CallCPUID64(uint32_t RegEAX, uint32_t RegECX, TRegs* Regs);
CallCPUID64 PROC
	; ECX = RegEAX
	; EDX = RegECX
	; R8  = PTR Regs
	PUSH   RBX
	MOV    EAX, ECX
	MOV    ECX, EDX
	CPUID
	MOV    DWORD PTR [R8 + TRegs.RegEAX], EAX
	MOV    DWORD PTR [R8 + TRegs.RegEBX], EBX
	MOV    DWORD PTR [R8 + TRegs.RegECX], ECX
	MOV    DWORD PTR [R8 + TRegs.RegEDX], EDX
	POP    RBX
	RET
CallCPUID64 ENDP

; extern "C" uint64_t CallRDTSCP();
CallRDTSCP PROC
	RDTSCP
	SHL    RDX, 32
	OR     RAX, RDX
	RET
CallRDTSCP ENDP

; extern "C" uint32_t CallXGETBV64(uint32_t RegECX, uint32_t* RegEAX, uint32_t* RegEDX);
CallXGETBV64 PROC
	; ECX = RegECX
	; EDX = PTR RegEAX
	; R8  = PTR RegEDX
	MOV    R9, RDX
	XGETBV
	MOV    DWORD PTR [R9], EAX
	MOV    DWORD PTR [R8], EDX
	RET
CallXGETBV64 ENDP

; extern "C" void CallXSETBV64(uint32_t RegECX, uint32_t RegEAX, uint32_t RegEDX);
CallXSETBV64 PROC
    ; ECX = RegECX (Indice del registro XCR extendend control, 0 per XCR0)
    ; EDX = RegEAX (Bit 0-31 della maschera di abilitazione delle feature)
    ; R8D = RegEDX (Bit 32-63 della maschera di abilitazione delle feature)
    MOV    EAX, EDX
    MOV    EDX, R8D
    XSETBV
    RET
CallXSETBV64 ENDP

; int64_t Clamp(int64_t Value, int64_t Min, int64_t Max);
Clamp PROC
	; RCX = Value
	; RDX = Min
	; R8 = Max
	MOV    RAX, RCX
	CMP    RAX, RDX
	CMOVL  RAX, RDX                    ; RAX = Max(Value, Min)
	CMP    RAX, R8 
	CMOVG  RAX, R8                     ; RAX = Min(Max(Value, Min), Max)
	RET
Clamp ENDP

; extern "C" void ClearFrameBuffer(void* Destination);
ClearFrameBufferX64 PROC
	; ECX = Destination
	MOV    R8, RDI
	MOV    RDI, RCX
	XOR    RAX, RAX
	MOV    RCX, 393216
	CLD
	REP    STOSQ
	MOV    RDI, R8
	RET
ClearFrameBufferX64 ENDP

; extern "C" void CopyFrameBuffer(void* Destination, void* Source);
CopyFrameBufferX64 PROC
	; RCX = Destination
	; RDX = Source
	MOV    R8, RDI
	MOV    R9, RSI
	MOV    RDI, RCX
	MOV    RSI, RDX
	MOV    RCX, 393216
	CLD
	REP    MOVSQ
	MOV    RSI, R9
	MOV    RDI, R8
	RET
CopyFrameBufferX64 ENDP

; void DrawTile(uint64_t Destination, uint64_t Source)
DrawTileX64 PROC
	; RCX = Destination
	; RDX = Source
	MOV    R8, RDI
	MOV    R9, RSI
	MOV    RDI, RCX
	MOV    RSI, RDX
	MOV    RCX, 24
@@TileLoop:
	MOV    RDX, [RSI]
	MOV    [RDI], RDX
	MOV    RDX, [RSI + 8]
	MOV    [RDI + 8], RDX
	MOV    RDX, [RSI + 16]
	MOV    [RDI + 16], RDX
	MOV    RDX, [RSI + 24]
	MOV    [RDI + 24], RDX
	MOV    RDX, [RSI + 32]
	MOV    [RDI + 32], RDX
	MOV    RDX, [RSI + 40]
	MOV    [RDI + 40], RDX
	MOV    RDX, [RSI + 48]
	MOV    [RDI + 48], RDX
	MOV    RDX, [RSI + 56]
	MOV    [RDI + 56], RDX
	MOV    RDX, [RSI + 64]
	MOV    [RDI + 64], RDX
	MOV    RDX, [RSI + 72]
	MOV    [RDI + 72], RDX
	MOV    RDX, [RSI + 80]
	MOV    [RDI + 80], RDX
	MOV    RDX, [RSI + 88]
	MOV    [RDI + 88], RDX
	ADD    RSI, 96
	ADD    RDI, 4096
	DEC    RCX
	JNZ    @@TileLoop
	MOV    RSI, R9
	MOV    RDI, R8
	RET
DrawTileX64 ENDP

; extern "C" void DrawSpriteX64(uint64_t Destination, uint64_t Sprite, uint64_t TransparentMask);
DrawSpriteX64 PROC
	; RCX = Destination
	; RDX = Sprite
	; R8 = TransparentMask
	MOV    R9, RBX
	MOV    R10, RDI
	MOV    R11, RSI
	MOV    RDI, RCX
	MOV    RSI, RDX
	MOV    RCX, 64
@@SpriteLoop:
	REPT 32
		MOV    RAX, [RSI]              ; RAX = byte dello sprite
		MOV    RBX, [RDI]              ; RBX = byte sullo schermo
		MOV    RDX, [R8]               ; RDX = maschere di trasparenza
		AND    RAX, RDX                ; RAX = AND(sprite, maschere)
		NOT    RDX                     ; RDX = maschere invertire
		AND    RBX, RDX                ; RBX = AND(pixel, NOT(maschere))
		OR     RAX, RBX                ; RAX = OR(AND(sprite, maschere), AND(pixel, NOT(maschere)))
		MOV    [RDI], RAX              ; scrive 8 pixel
		ADD    R8, 8
		ADD    RSI, 8
		ADD    RDI, 8
	ENDM
	ADD    RDI, 3840
	DEC    RCX
	JNZ    @@SpriteLoop
	MOV    RSI, R11
	MOV    RDI, R10
	MOV    RBX, R9
	RET
DrawSpriteX64 ENDP

; extern "C" void DrawSpriteSSE(uint64_t Destination, uint64_t Sprite, uint64_t TransparentMask);
DrawSpriteSSE PROC
	; RCX = Destination
	; RDX = Sprite
	; R8 = TransparentMask
	MOV    R9, RDI
	MOV    R10, RSI
	MOV    RDI, RCX
	MOV    RSI, RDX
	MOV    RCX, 64
@@SpriteLoop:
	REPT 16
		MOVDQA XMM0, [RSI]             ; XMM0 = byte dello sprite
		MOVDQU XMM1, [RDI]             ; XMM1 = byte sullo schermo
		MOVDQA XMM2, [R8]              ; XMM2 = maschere di trasparenza
		PAND   XMM0, XMM2              ; XMM0 = AND(sprite, maschere)
		PANDN  XMM2, XMM1              ; XMM3 = AND(NOT(maschere), pixel)
		POR    XMM0, XMM2              ; XMM0 = OR(AND(sprite, maschere), AND(pixel, NOT(maschere)))
		MOVDQU [RDI], XMM0
		ADD    RSI, 16
		ADD    R8,  16
		ADD    RDI, 16
	ENDM
    ADD    RDI, 3840
	DEC    RCX
	JNZ    @@SpriteLoop
	MOV    RSI, R10
	MOV    RDI, R9
	RET
DrawSpriteSSE ENDP

; extern "C" void DrawSpriteAVX(uint64_t Destination, uint64_t Sprite, uint64_t TransparentMask);
DrawSpriteAVX PROC
	; RCX = Destination (LFB)
	; RDX = Sprite
	; R8  = TransparentMask
	MOV    R9, RDI
	MOV    R10, RSI
	MOV    RDI, RCX
	MOV    RSI, RDX
	MOV    RCX, 64
@@SpriteLoop:
	REPT 8
		VMOVDQA YMM0, YMMWORD PTR [RSI]
		VMOVDQU YMM1, YMMWORD PTR [RDI]
		VMOVDQA YMM2, YMMWORD PTR [R8]
		VPBLENDVB YMM0, YMM1, YMM0, YMM2   ; YMM0 = Blend(Schermo, Sprite, Maschera)
		VMOVDQU YMMWORD PTR [RDI], YMM0
		ADD    RSI, 32
		ADD    R8,  32
		ADD    RDI, 32
	ENDM
	ADD    RDI, 3840	
	DEC    RCX
	JNZ    @@SpriteLoop
	MOV    RSI, R10
	MOV    RDI, R9
	RET
DrawSpriteAVX ENDP

; extern "C" void WaitVerticalRetrace();
WaitVerticalRetraceX64 PROC
@VSync:
	MOV    DX, 03DAh
	IN     AL, DX
	AND    AL, 8
	OR     AL, AL
	JZ     @VSync
@NoVSync:
	MOV    DX, 03DAh
	IN     AL, DX
	AND    AL, 8
	OR     AL, AL
	JNZ    @NoVSync
	RET
WaitVerticalRetraceX64 ENDP

PUBLIC CallCPUID64
PUBLIC CallRDTSCP
PUBLIC CallXGETBV64
PUBLIC CallXSETBV64
PUBLIC Clamp
PUBLIC ClearFrameBufferX64
PUBLIC CopyFrameBufferX64
PUBLIC DrawSpriteX64
PUBLIC DrawSpriteSSE
PUBLIC DrawSpriteAVX
PUBLIC DrawTileX64
PUBLIC WaitVerticalRetraceX64

END
