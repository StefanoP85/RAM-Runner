/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Core32.cpp
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Main source for the 32 bit Core 0
 * ============================================================================
 */
#include <intrin.h>
#include "Console.h"
#include "Core32.h"
#include "Video.h"

namespace RamRunner::Core32
{
	// Organizzazione del blocco di memoria
	constexpr uint32_t MemOffsetFrameBuffer = 0;
	constexpr uint32_t MemOffsetMazeBuffer = MemOffsetFrameBuffer + sizeof(Video::TFullScreen);
	constexpr uint32_t MemOffsetGraphicBuffer = MemOffsetMazeBuffer + sizeof(Video::TFullScreen);
	constexpr uint32_t MemOffsetCharSet = MemOffsetGraphicBuffer + sizeof(Video::TTextScreen);
	constexpr uint32_t MemOffsetTileSet = MemOffsetCharSet + sizeof(Video::TCharSet);
	constexpr uint32_t MemOffsetSpriteSet = MemOffsetTileSet + sizeof(Video::TTileSet);
	constexpr uint32_t MemOffsetTransparentMaskSet = MemOffsetSpriteSet + sizeof(Video::TSpriteSet);
	constexpr uint32_t MemOffsetKernel = (MemOffsetTransparentMaskSet + sizeof(Video::TSpriteSet) + 1048575) & 0xFFF00000;
	constexpr uint32_t MemOffsetPML4 = MemOffsetKernel;
	constexpr uint32_t MemOffsetPML3 = MemOffsetPML4 + 4096;
	constexpr uint32_t MemOffsetPML2 = MemOffsetPML3 + 4096;
	constexpr uint32_t MemOffsetGDT = MemOffsetPML2 + 4096 * 4;
	constexpr uint32_t MemOffsetIDT = MemOffsetGDT + 4096;
	constexpr uint32_t MemOffsetCommandQueue = MemOffsetIDT + 4096;
	constexpr uint32_t MemOffsetRR64Code = MemOffsetCommandQueue + 4096 + sizeof(TCommandQueue);

	// Codice binario del modulo a 64 bit incorporato come dati
	extern "C" uint8_t RamRunner64[];
	extern "C" uint32_t RamRunner64SIZE;

	// Area di memoria dedicata al codice di trampolino del Core 1
	uint32_t StartupBlockBuffer;
	uint16_t StartupBlockSelector;
	uint32_t LapicLinearBase;

	// Puntatori alle strutture del micro kernel
	uint32_t ISRProblem;
	uint32_t StartupCounter;
	uint32_t AddressTrampoline;
	uint32_t WorkAreaAddress;
	PWorkArea WorkArea;
	uint32_t CommandQueueLinearAddress;

	// Verifica la variabile incrementata dalle ISR critiche
	bool HasCrashed()
	{
		volatile uint32_t* Pointer = (uint32_t*)ISRProblem;
		volatile uint32_t Result = *Pointer;
		return Result != 0;
	}

	// Verifica la variabile incrementata all'avvio del Core 1
	bool HasStarted()
	{
		volatile uint32_t* Pointer = (uint32_t*)StartupCounter;
		volatile uint32_t Result = *Pointer;
		return Result != 0;
	}

	uint32_t EmitGDTEntry(uint32_t CurrentPosition, uint32_t Base, uint32_t Limit, uint8_t AccessByte, uint8_t FlagsLimit4Bits)
	{
		uint32_t* Pointer;
		uint32_t High;
		uint32_t Low;

		// DWORD Bassa
		// Limite (0:15) nei bit 0:15
		// Base (0:15) nei bit 16:31
		Low = (Limit & 0xFFFF) | ((Base & 0xFFFF) << 16);

		// DWORD Alta
		// Base (16:23) nei bit 0:7
		// Access Byte nei bit 8:15
		// Limite (16:19) + Flags (G, D/B, L, AVL) nei bit 16:23
		// Base (24:31) nei bit 24:31
		High = ((Base >> 16) & 0xFF) | ((uint32_t)AccessByte << 8) | (((Limit >> 16) & 0x0F) << 16) | (((uint32_t)FlagsLimit4Bits & 0x0F) << 20) | (Base & 0xFF000000);

		// 3. Scrittura lineare in memoria tramite puntatore a 32 bit
		Pointer = (uint32_t*)CurrentPosition;
		Pointer[0] = Low;
		Pointer[1] = High;
		return CurrentPosition + 8;
	}

#define IDT_PRESENT      (1 << 15) // Present bit (P)
#define IDT_DPL_0        (0 << 13) // Descriptor Privilege Level 0
#define IDT_INT_GATE_64  0x0E      // 64-bit Interrupt Gate (disabilita le interruzioni hardware)
#define IDT_TRAP_GATE_64 0x0F      // 64-bit Trap Gate (mantiene le interruzioni abilitate)
	uint32_t EmitIDTEntry(uint32_t CurrentPosition, uint32_t IsrAddressLow, uint32_t IsrAddressHigh, uint16_t Selector, uint16_t GateType, uint8_t Ist)
	{
		uint32_t* Pointer;
		uint32_t Dword0;
		uint32_t Dword1;
		uint32_t Dword2;
		uint32_t Dword3;
		uint32_t ControlBits;

		Pointer = (uint32_t*)CurrentPosition;

		// Dword 0: Offset [15:0] nei bit 16-31, Selector nei bit 0-15 (o viceversa secondo specifiche)
		// Specifiche IDT Gate 64-bit:
		// [0-15]   Selector (16 bit)
		// [16-31]  Offset [15:0] (16 bit)
		Dword0 = ((uint32_t)Selector << 16) | (IsrAddressLow & 0xFFFF);

		// Dword 1:
		// [0-15]   Bits di controllo (IST, Type, DPL, Present)
		// [16-31]  Offset [31:16] (16 bit)
		ControlBits = (Ist & 0x07) |
			((GateType & 0x0F) << 8) |
			(0 << 13) | // DPL 0
			(1 << 15);  // Present
		Dword1 = (IsrAddressLow & 0xFFFF0000) | ControlBits;

		// Dword 2: Offset [63:32] (32 bit)
		Dword2 = IsrAddressHigh;

		// Dword 3: Riservato (deve essere 0)
		Dword3 = 0;

		Pointer[0] = Dword0;
		Pointer[1] = Dword1;
		Pointer[2] = Dword2;
		Pointer[3] = Dword3;

		return CurrentPosition + 16;
	}

#define MMU_PRESENT  (1UL << 0)
#define MMU_WRITABLE (1UL << 1)
#define MMU_USER     (1UL << 2)
#define MMU_PWT      (1UL << 3)
#define MMU_PCD      (1UL << 4)
#define MMU_PAGESIZE (1UL << 7)
	uint32_t EmitPageEntry(uint32_t CurrentPosition, uint32_t MappedAddress, uint32_t Flags)
	{
		uint32_t* Pointer;
		uint32_t High;
		uint32_t Low;

		Low = MappedAddress + Flags;
		High = 0;

		Pointer = (uint32_t*)CurrentPosition;
		Pointer[0] = Low;
		Pointer[1] = High;
		return CurrentPosition + 8;
	}

	uint32_t EmitTablePointer(uint32_t CurrentPosition, uint32_t TableAddress, uint16_t Limit)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		// 1. Scrive il limite a 16 bit (2 byte)
		*(uint16_t*)Pointer = Limit;
		// 2. Scrive l'indirizzo base a 32 bit (4 byte)
		*(uint32_t*)(Pointer + 2) = TableAddress;
		// 3. Altri 4 byte a zero (per completare i 10 byte del pseudo-descrittore LGDT)
		*(uint32_t*)(Pointer + 6) = 0;
		return CurrentPosition + 10;
	}

	uint32_t EmitADD_RSP_8(uint32_t CurrentPosition)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer[0] = 0x48;
		Pointer[1] = 0x83;
		Pointer[2] = 0xC4;
		Pointer[3] = 0x08;
		return CurrentPosition + 4;
	}

	uint32_t EmitIRETQ(uint32_t CurrentPosition)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer[0] = 0x48;
		Pointer[1] = 0xCF;
		return CurrentPosition + 2;
	}

	uint32_t EmitJMPShort(uint32_t CurrentPosition, signed char RelativeOffset)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer[0] = 0xEB; // JMP op-code
		Pointer[1] = (unsigned char)RelativeOffset;
		return CurrentPosition + 2;
	}

	uint32_t EmitJMPNear(uint32_t CurrentPosition, uint32_t RelativeOffset)
	{
		uint32_t* Pointer32;
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer32 = (uint32_t*)(CurrentPosition + 1);
		Pointer[0] = 0xE9; // JMP op-code
		*Pointer32 = RelativeOffset;
		return CurrentPosition + 5;
	}

	uint32_t EmitJMPFar(uint32_t CurrentPosition, uint32_t AbsoluteOffset, uint16_t Selector)
	{
		uint16_t* Pointer16;
		uint32_t* Pointer32;
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer32 = (uint32_t*)(CurrentPosition + 1);
		Pointer16 = (uint16_t*)(CurrentPosition + 5);
		Pointer[0] = 0xEA; // JMP op-ocde
		*Pointer32 = AbsoluteOffset;
		*Pointer16 = Selector;
		return CurrentPosition + 7;
	}

	uint32_t EmitJMPFarPrefix(uint32_t CurrentPosition, uint32_t AbsoluteOffset, uint16_t Selector)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer[0] = 0x66; // 32 bit prefix
		return EmitJMPFar(CurrentPosition + 1, AbsoluteOffset, Selector);
	}

	uint32_t EmitJMP64(uint32_t CurrentPosition, uint64_t AbsoluteAddress)
	{
		uint64_t* Pointer64;
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer64 = (uint64_t*)(CurrentPosition + 2);
		Pointer[0] = 0x48; // REX.W
		Pointer[1] = 0xB8; // Op-code
		*Pointer64 = AbsoluteAddress;
		Pointer[10] = 0xFF; // Op-code
		Pointer[11] = 0xE0; // ModR/M
		return CurrentPosition + 12;
	}

	uint32_t EmitVariableAssignment(uint32_t CurrentPosition, uint32_t Address, uint32_t ImmediateValue)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		uint64_t AbsoluteAddress = (uint64_t)Address;

		// MOV RAX, imm64 (10 byte)
		// Opcode REX.W (48) + B8 + imm64
		Pointer[0] = 0x48;
		Pointer[1] = 0xB8;
		*(uint64_t*)(Pointer + 2) = AbsoluteAddress;
		Pointer += 10;

		// MOV EDX, ImmediateValue (5 byte)
		// Opcode B8 + imm32
		Pointer[0] = 0xBA;
		*(uint32_t*)(Pointer + 1) = ImmediateValue;
		Pointer += 5;

		// MOV [RAX], EDX (2 byte)
		// Opcode REX.W (89 /r) oppure formato base 89 10
		Pointer[0] = 0x89;
		Pointer[1] = 0x10;
		Pointer += 2;

		return CurrentPosition + 17;
	}

	uint32_t EmitMOV_RCX(uint32_t CurrentPosition, uint32_t ImmediateValue)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		// REX.W + B9 + imm64 (Carica un valore a 64 bit in RCX)
		Pointer[0] = 0x48;
		Pointer[1] = 0xB9;
		*(uint32_t*)(Pointer + 2) = ImmediateValue;
		*(uint32_t*)(Pointer + 6) = 0;
		return CurrentPosition + 10;
	}

	uint32_t EmitPAUSE(uint32_t CurrentPosition)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer[0] = 0xF3; // REP
		Pointer[1] = 0x90; // NOP
		return CurrentPosition + 2;
	}

	uint32_t EmitCLI(uint32_t CurrentPosition)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer[0] = 0xFA;
		return CurrentPosition + 1;
	}

	uint32_t EmitSTI(uint32_t CurrentPosition)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer[0] = 0xFB;
		return CurrentPosition + 1;
	}

	uint32_t EmitUD2(uint32_t CurrentPosition)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer[0] = 0x0F;
		Pointer[1] = 0x0B;
		return CurrentPosition + 2;
	}

	uint32_t EmitLGDT(uint32_t CurrentPosition, uint32_t GdtPointerAddress)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer[0] = 0x0F;
		Pointer[1] = 0x01;
		Pointer[2] = 0x15; // ModR/M per [disp32]
		*(uint32_t*)(Pointer + 3) = GdtPointerAddress;
		return CurrentPosition + 7;
	}

	uint32_t EmitLGDTPrefix(uint32_t CurrentPosition, uint32_t GdtPointerAddress)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		Pointer[0] = 0x67;
		return EmitLGDT(CurrentPosition + 1, GdtPointerAddress);
	}

	uint32_t EmitLIDT(uint32_t CurrentPosition, uint32_t IdtPointerAddress)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		uint64_t AbsoluteAddress = (uint64_t)IdtPointerAddress;

		// MOV RAX, imm64 (10 byte)
		Pointer[0] = 0x48;
		Pointer[1] = 0xB8;
		*(uint64_t*)(Pointer + 2) = AbsoluteAddress;
		Pointer += 10;

		// LIDT [RAX] (3 byte)
		Pointer[0] = 0x0F;
		Pointer[1] = 0x01;
		Pointer[2] = 0x18; // ModR/M indica [RAX] con estensione /3 per LIDT
		Pointer += 3;

		return CurrentPosition + 13;
	}

	uint32_t EmitVariable(uint32_t CurrentPosition, uint32_t Value)
	{
		uint32_t* Pointer = (uint32_t*)CurrentPosition;
		*Pointer = Value;
		return CurrentPosition + 4;
	}

	uint32_t EmitIsrStd(uint32_t CurrentPosition)
	{
		CurrentPosition = EmitIRETQ(CurrentPosition);
		return CurrentPosition;
	}

	uint32_t EmitIsrStdArgument(uint32_t CurrentPosition)
	{
		CurrentPosition = EmitADD_RSP_8(CurrentPosition);
		CurrentPosition = EmitIRETQ(CurrentPosition);
		return CurrentPosition;
	}

	uint32_t EmitIsrStdNeverReturning(uint32_t CurrentPosition, uint32_t SignalingVariableAddress, uint32_t SignalingValue)
	{
		CurrentPosition = EmitVariableAssignment(CurrentPosition, SignalingVariableAddress, SignalingValue);
		CurrentPosition = EmitPAUSE(CurrentPosition);
		CurrentPosition = EmitJMPShort(CurrentPosition, -4);
		return CurrentPosition;
	}

	uint32_t EmitIsrStdArgumentNeverReturning(uint32_t CurrentPosition, uint32_t SignalingVariableAddress, uint32_t SignalingValue)
	{
		CurrentPosition = EmitVariableAssignment(CurrentPosition, SignalingVariableAddress, SignalingValue);
		CurrentPosition = EmitADD_RSP_8(CurrentPosition);
		CurrentPosition = EmitPAUSE(CurrentPosition);
		CurrentPosition = EmitJMPShort(CurrentPosition, -4);
		return CurrentPosition;
	}

	uint32_t EmitEnablePE(uint32_t CurrentPosition)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		// MOV    EAX, CR0
		// OR     EAX, 0x00000001
		// MOV    CR0, EAX
		Pointer[0] = 0x0F; Pointer[1] = 0x20; Pointer[2] = 0xC0;
		Pointer[3] = 0x0D; *(uint32_t*)(Pointer + 4) = 0x00000001;
		Pointer[8] = 0x0F; Pointer[9] = 0x22; Pointer[10] = 0xC0;
		return CurrentPosition + 11;
	}

	uint32_t EmitEnablePAE(uint32_t CurrentPosition)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		// MOV    EAX, CR4
		// OR     EAX, 0x00000020
		// MOV    CR4, EAX
		Pointer[0] = 0x0F; Pointer[1] = 0x20; Pointer[2] = 0xE0;
		Pointer[3] = 0x0D; *(uint32_t*)(Pointer + 4) = 0x00000020;
		Pointer[8] = 0x0F; Pointer[9] = 0x22; Pointer[10] = 0xE0;
		return CurrentPosition + 11;
	}

	uint32_t EmitLoadCR3(uint32_t CurrentPosition, uint32_t PageMapPtrAddress)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		// MOV    EAX, [PageMapPtrAddress]
		// MOV    CR3, EAX
		Pointer[0] = 0xB8;//0xA1;
		*(uint32_t*)(Pointer + 1) = PageMapPtrAddress;
		Pointer[5] = 0x0F; Pointer[6] = 0x22; Pointer[7] = 0xD8;
		return CurrentPosition + 8;
	}

	uint32_t EmitEnableLME(uint32_t CurrentPosition)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		// MOV    ECX, 0xC0000080
		// RDMSR
		// OR     EAX, 0x00000100
		// WRMSR
		Pointer[0] = 0xB9; *(uint32_t*)(Pointer + 1) = 0xC0000080;
		Pointer[5] = 0x0F; Pointer[6] = 0x32;
		Pointer[7] = 0x0D; *(uint32_t*)(Pointer + 8) = 0x00000100;
		Pointer[12] = 0x0F; Pointer[13] = 0x30;
		return CurrentPosition + 14;
	}

	uint32_t EmitEnablePaging(uint32_t CurrentPosition)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		// MOV    EAX, CR0
		// OR     EAX, 0x80000000
		// MOV    CR0, EAX
		Pointer[0] = 0x0F; Pointer[1] = 0x20; Pointer[2] = 0xC0;
		Pointer[3] = 0x0D; *(uint32_t*)(Pointer + 4) = 0x80000000;
		Pointer[8] = 0x0F; Pointer[9] = 0x22; Pointer[10] = 0xC0;
		return CurrentPosition + 11;
	}

	uint32_t EmitAssignSelectors(uint32_t CurrentPosition, uint16_t Selector)
	{
		uint16_t* Mem;
		unsigned char* Pointer = (unsigned char*)CurrentPosition;

		// MOV AX, Selector
		Pointer[0] = 0x66; // Size prefix
		Pointer[1] = 0xB8; // MOV op-code
		Mem = (uint16_t*)(CurrentPosition + 2);
		*Mem = Selector;

		// MOV DS, AX
		Pointer[4] = 0x8E;
		Pointer[5] = 0xD8;

		// MOV ES, AX
		Pointer[6] = 0x8E;
		Pointer[7] = 0xC0;

		// MOV SS, AX
		Pointer[8] = 0x8E;
		Pointer[9] = 0xD0;

		return CurrentPosition + 10;
	}

	uint32_t EmitAssignStack(uint32_t CurrentPosition, uint32_t ImmediateValue)
	{
		uint32_t* Mem;
		unsigned char* Pointer = (unsigned char*)CurrentPosition;

		// MOV ESP, ImmediateValue
		Pointer[0] = 0xBC; // MOV op-code
		Mem = (uint32_t*)(CurrentPosition + 1);
		*Mem = ImmediateValue;

		// MOV EBP, ImmediateValue
		Pointer[5] = 0xBD; // MOV op-code
		Mem = (uint32_t*)(CurrentPosition + 6);
		*Mem = ImmediateValue;

		return CurrentPosition + 10;
	}

	uint32_t EmitPrintAndLoopReal(uint32_t CurrentPosition, char Character, uint8_t Attribute)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;

		// 1. MOV AX, 0xB800 (Segmento della memoria video) - 3 byte
		Pointer[0] = 0xB8;
		*(uint16_t*)(Pointer + 1) = 0xB800;
		Pointer += 3;

		// 2. MOV ES, AX (Carica ES con il segmento video) - 2 byte
		Pointer[0] = 0x8E;
		Pointer[1] = 0xC0;
		Pointer += 2;

		// 3. MOV BX, 0x08C0 (Offset della riga 14, colonna 0) - 3 byte
		Pointer[0] = 0xBB;
		*(uint16_t*)(Pointer + 1) = 0x08C0;
		Pointer += 3;

		// 4. MOV AX, 0x{Attribute}{Character} (Prepara i due byte da scrivere) - 3 byte
		uint16_t TextWord = ((uint16_t)Attribute << 8) | (unsigned char)Character;
		Pointer[0] = 0xB8;
		*(uint16_t*)(Pointer + 1) = TextWord;
		Pointer += 3;

		// 5. MOV ES:[BX], AX (Scrive carattere e attributo sullo schermo) - 2 byte
		Pointer[0] = 0x26; // Prefisso segmento ES
		Pointer[1] = 0x89; // MOV [bx], ax
		Pointer[2] = 0x07; // ModR/M per [BX]
		Pointer += 3;

		// 6. Loop infinito: JMP $ - 2 (EB FE) - 2 byte
		Pointer[0] = 0xEB;
		Pointer[1] = 0xFE;
		Pointer += 2;

		return (uint32_t)Pointer;
	}

	uint32_t EmitPrintAndLoop(uint32_t CurrentPosition, char Character, uint8_t Attribute)
	{
		unsigned char* Pointer = (unsigned char*)CurrentPosition;
		uint32_t VideoOffset = 0xB88C0; // Riga 14, Colonna 0

		// 1. MOV EAX, 0xB88C0 (Indirizzo della memoria video) - 5 byte
		Pointer[0] = 0xB8;
		*(uint32_t*)(Pointer + 1) = VideoOffset;
		Pointer += 5;

		// 2. MOV DX, 0x{Attribute}{Character} (Prepara i due byte da scrivere) - 4 byte
		//    Carichiamo un valore immediato a 16 bit nel registro DX
		uint16_t TextWord = ((uint16_t)Attribute << 8) | (unsigned char)Character;
		Pointer[0] = 0x66; // Prefisso operando a 16 bit
		Pointer[1] = 0xBA; // MOV DX, imm16
		*(uint16_t*)(Pointer + 2) = TextWord;
		Pointer += 4;

		// 3. MOV [EAX], DX (Scrive carattere e attributo direttamente sullo schermo) - 3 byte
		//    Usiamo un offset esplicito con base in EAX
		Pointer[0] = 0x66; // Prefisso operando a 16 bit per la word DX
		Pointer[1] = 0x11; // MOV [EAX], DX (Opcode 89 /1 oppure 11 a seconda del formato SIB, 
		// usiamo la codifica sicura: 66 89 10 -> MOV [EAX], DX)
		Pointer[1] = 0x89;
		Pointer[2] = 0x10;
		Pointer += 3;

		// 4. Loop infinito: JMP $ - 2 (EB FE) - 2 byte
		Pointer[0] = 0xEB;
		Pointer[1] = 0xFE;
		Pointer += 2;

		return (uint32_t)Pointer;
	}

	bool CreateStartupBlock()
	{
		uint32_t AddressPML2;
		uint32_t AddressPML3;
		uint32_t AddressPML4;
		uint32_t AddressGDT;
		uint32_t AddressIDT;
		uint32_t GDTPointer;
		uint32_t IDTPointer;
		uint32_t AddressIsrStd;
		uint32_t AddressIsrStdNeverReturning;
		uint32_t AddressIsrArgument;
		uint32_t AddressIsrArgumentNeverReturning;
		uint32_t CurrentPosition;
		uint32_t Index;
		uint32_t MappedAddress;
		uint32_t StackAddress;
		uint32_t CommandQueuePhysicalAddress;
		uint32_t EntryPoint;
		uint32_t XMSLinearAddress, XMSPhysicalAddress;

		// Alloca memoria "convezionale" per il trampolino
		if (!Processor::DpmiAllocateConventional(2 * 4096, &StartupBlockSelector, (void**)&StartupBlockBuffer))
		{
			return false;
		}

		// Allinea il blocco a 4096 byte
		AddressTrampoline = (StartupBlockBuffer + 4095) & 0xFFFFF000;
		XMSLinearAddress = Processor::GetXMSAddress();
		XMSPhysicalAddress = Processor::GetXMSPhysicalAddress();
		// Usa la fine del blocco XMS come stack (64 byte di sicurezza e 64 byte di allineamento in GetXMSPhysicalAddress)
		StackAddress = XMSPhysicalAddress + 12 * 1024 * 1024 - 128;
		AddressPML4 = XMSPhysicalAddress + MemOffsetPML4;
		AddressPML3 = XMSPhysicalAddress + MemOffsetPML3;
		AddressPML2 = XMSPhysicalAddress + MemOffsetPML2;
		AddressGDT = XMSPhysicalAddress + MemOffsetGDT;
		AddressIDT = XMSPhysicalAddress + MemOffsetIDT;

		// Crea la pagina della PML4
		CurrentPosition = XMSLinearAddress + MemOffsetPML4;
		CurrentPosition = EmitPageEntry(CurrentPosition, AddressPML3, MMU_USER | MMU_WRITABLE | MMU_PRESENT);

		// Crea la pagina della PML3 (PDPT), 4 GB
		CurrentPosition = XMSLinearAddress + MemOffsetPML3;
		MappedAddress = AddressPML2;
		for (Index = 0; Index < 4; Index++)
		{
			CurrentPosition = EmitPageEntry(CurrentPosition, MappedAddress, MMU_USER | MMU_WRITABLE | MMU_PRESENT);
			MappedAddress += 4096;
		}

		// Crea le pagine della PML2 (PD), pagine 2 MB, 4 GB
		CurrentPosition = XMSLinearAddress + MemOffsetPML2;
		MappedAddress = 0;
		for (Index = 0; Index < (4 * 512); Index++)
		{
			if (MappedAddress >= Video::VideoLinearFrameBuffer)
				CurrentPosition = EmitPageEntry(CurrentPosition, MappedAddress, MMU_PRESENT | MMU_WRITABLE | MMU_USER | MMU_PWT | MMU_PCD | MMU_PAGESIZE);
			else
				CurrentPosition = EmitPageEntry(CurrentPosition, MappedAddress, MMU_PRESENT | MMU_WRITABLE | MMU_USER | MMU_PAGESIZE);
			MappedAddress += 2 * 1024 * 1024;
		}

		// Crea la GDT
		CurrentPosition = XMSLinearAddress + MemOffsetGDT;
		// Null Descriptor -> Offset 0x00
		CurrentPosition = EmitGDTEntry(CurrentPosition, 0, 0, 0, 0);
		// 64-bit codice -> Offset 0x08 (Base=0, Limit=0xFFFFF, Access=0x9A, Flags=0x0A [Gran+64 bit])
		CurrentPosition = EmitGDTEntry(CurrentPosition, 0, 0xFFFFF, 0x9A, 0x0A);
		// 64-bit dati -> Offset 0x10 (Base=0, Limit=0xFFFFF, Access=0x92, Flags=0x08 [Gran+64 bit dati])
		CurrentPosition = EmitGDTEntry(CurrentPosition, 0, 0xFFFFF, 0x92, 0x08);
		// 32-bit codice -> Offset 0x18 (Base=0, Limit=0xFFFFF, Access=0x9A, Flags=0x0C [Gran+32 bit])
		CurrentPosition = EmitGDTEntry(CurrentPosition, 0, 0xFFFFF, 0x9A, 0x0C);
		// 32-bit dati -> Offset 0x20 (Base=0, Limit=0xFFFFF, Access=0x92, Flags=0x0C [Gran+32 bit])
		CurrentPosition = EmitGDTEntry(CurrentPosition, 0, 0xFFFFF, 0x92, 0x0C);

		// Crea i puntatori a GDT e IDT, le variabili ISRProblem e StartupCounter
		GDTPointer = CurrentPosition + XMSPhysicalAddress - XMSLinearAddress;
		CurrentPosition = EmitTablePointer(CurrentPosition, XMSPhysicalAddress + MemOffsetGDT, 5 * 8 - 1);
		IDTPointer = CurrentPosition + XMSPhysicalAddress - XMSLinearAddress;
		CurrentPosition = EmitTablePointer(CurrentPosition, XMSPhysicalAddress + MemOffsetIDT, 256 * 16 - 1);
		ISRProblem = CurrentPosition;
		CurrentPosition = EmitVariable(CurrentPosition, 0);
		StartupCounter = CurrentPosition;
		CurrentPosition = EmitVariable(CurrentPosition, 0);

		// Crea le tre funzioni ISR
		AddressIsrStd = CurrentPosition + XMSPhysicalAddress - XMSLinearAddress;
		CurrentPosition = EmitIsrStd(CurrentPosition);
		AddressIsrStdNeverReturning = CurrentPosition + XMSPhysicalAddress - XMSLinearAddress;
		CurrentPosition = EmitIsrStdNeverReturning(CurrentPosition, ISRProblem + XMSPhysicalAddress - XMSLinearAddress, 1);
		AddressIsrArgument = CurrentPosition + XMSPhysicalAddress - XMSLinearAddress;
		CurrentPosition = EmitIsrStdArgument(CurrentPosition);
		AddressIsrArgumentNeverReturning = CurrentPosition + XMSPhysicalAddress - XMSLinearAddress;
		CurrentPosition = EmitIsrStdArgumentNeverReturning(CurrentPosition, ISRProblem + XMSPhysicalAddress - XMSLinearAddress, 1);

		// Crea la WorkArea
		CurrentPosition = (CurrentPosition + 63) & 0xFFFFFFC0;
		CommandQueueLinearAddress = XMSLinearAddress + MemOffsetCommandQueue;
		CommandQueuePhysicalAddress = XMSPhysicalAddress + MemOffsetCommandQueue;
		WorkAreaAddress = CurrentPosition + XMSPhysicalAddress - XMSLinearAddress;
		WorkArea = reinterpret_cast<PWorkArea>((void*)CurrentPosition);
		WorkArea->VideoMemory = Video::VideoLinearFrameBuffer;
		WorkArea->VideoMemoryHigh = 0;
		WorkArea->FrameBuffer = XMSPhysicalAddress + MemOffsetFrameBuffer;
		WorkArea->FrameBufferHigh = 0;
		WorkArea->CommandQueue = CommandQueuePhysicalAddress;
		WorkArea->CommandQueueHigh = 0;
		WorkArea->CommandSize = MaxCommand;
		WorkArea->CommandSizeHigh = 0;
		WorkArea->CommandHead = 0;
		WorkArea->CommandHeadHigh = 0;
		WorkArea->CommandTail = 0;
		WorkArea->CommandTailHigh = 0;
		WorkArea->CharSet = XMSPhysicalAddress + MemOffsetCharSet;
		WorkArea->CharSetHigh = 0;
		WorkArea->SpriteSet = XMSPhysicalAddress + MemOffsetSpriteSet;
		WorkArea->SpriteSetHigh = 0;
		WorkArea->TransparentMaskSet = XMSPhysicalAddress + MemOffsetTransparentMaskSet;
		WorkArea->TransparentMaskSetHigh = 0;
		WorkArea->TileSet = XMSPhysicalAddress + MemOffsetTileSet;
		WorkArea->TileSetHigh = 0;
		CurrentPosition += sizeof(TWorkArea);

		// Crea la IDT
		CurrentPosition = XMSLinearAddress + MemOffsetIDT;
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStdNeverReturning, 0, 8, IDT_INT_GATE_64, 0); // 0x00 #DE
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_TRAP_GATE_64, 0); // 0x01 #DB
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x02 #NMI
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_TRAP_GATE_64, 0); // 0x03 #BP
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x04 #OF
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x05 #BR
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStdNeverReturning, 0, 8, IDT_INT_GATE_64, 0); // 0x06 #UD
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x07 #NM
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStdNeverReturning, 0, 8, IDT_INT_GATE_64, 0); // 0x08 #DF
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x09
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrArgument, 0, 8, IDT_INT_GATE_64, 0); // 0x0A #TS
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStdNeverReturning, 0, 8, IDT_INT_GATE_64, 0); // 0x0B #NP
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStdNeverReturning, 0, 8, IDT_INT_GATE_64, 0); // 0x0C #SS
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrArgumentNeverReturning, 0, 8, IDT_INT_GATE_64, 0); // 0x0D #GP
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrArgumentNeverReturning, 0, 8, IDT_INT_GATE_64, 0); // 0x0E #PF
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x0F
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStdNeverReturning, 0, 8, IDT_INT_GATE_64, 0); // 0x10 #MF
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrArgument, 0, 8, IDT_INT_GATE_64, 0); // 0x11 #AC
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x12 #MC
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x13 #XM
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x14 #VE
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrArgument, 0, 8, IDT_INT_GATE_64, 0); // 0x15 #CP
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x16
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x17
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x18
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x19
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x1A
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x1B
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x1C #HV
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrArgument, 0, 8, IDT_INT_GATE_64, 0); // 0x1D #VC
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrArgument, 0, 8, IDT_INT_GATE_64, 0); // 0x1E #SX
		CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0); // 0x1F
		for (Index = 0x20; Index <= 0xFF; Index++)
			CurrentPosition = EmitIDTEntry(CurrentPosition, AddressIsrStd, 0, 8, IDT_INT_GATE_64, 0);

		// Copia il codice eseguibile del Core 1
		EntryPoint = XMSPhysicalAddress + MemOffsetRR64Code;
		Processor::MemCopy((void*)(XMSLinearAddress + MemOffsetRR64Code), (void*)(&RamRunner64), RamRunner64SIZE);

		// Crea il codice del trampolino
		// Il processore entra in real mode
		CurrentPosition = AddressTrampoline;
		CurrentPosition = EmitCLI(CurrentPosition);
		//CurrentPosition = EmitPrintAndLoopReal(CurrentPosition, 'Z', 7);
		//CurrentPosition = EmitJMPShort(CurrentPosition, -14);
		CurrentPosition = EmitLGDTPrefix(CurrentPosition, GDTPointer);
		CurrentPosition = EmitEnablePE(CurrentPosition);
		CurrentPosition = EmitJMPFarPrefix(CurrentPosition, CurrentPosition + 8, 0x18);
		// Il processore entra in protected mode 32 bit
		CurrentPosition = EmitAssignSelectors(CurrentPosition, 0x20);
		CurrentPosition = EmitAssignStack(CurrentPosition, StackAddress);
		//CurrentPosition = EmitPrintAndLoop(CurrentPosition, 'Z', 7);
		//CurrentPosition = EmitJMPShort(CurrentPosition, -14);
		CurrentPosition = EmitEnablePAE(CurrentPosition);
		CurrentPosition = EmitLoadCR3(CurrentPosition, AddressPML4);
		CurrentPosition = EmitEnableLME(CurrentPosition);
		CurrentPosition = EmitEnablePaging(CurrentPosition);
		CurrentPosition = EmitJMPFar(CurrentPosition, CurrentPosition + 7, 0x08);
		// Il processore entra in long mode
		CurrentPosition = EmitLIDT(CurrentPosition, IDTPointer);
		CurrentPosition = EmitAssignSelectors(CurrentPosition, 0x10);
		// Incremento della variabile di segnalazione
		CurrentPosition = EmitVariableAssignment(CurrentPosition, StartupCounter + XMSPhysicalAddress - XMSLinearAddress, 1);
		CurrentPosition = EmitMOV_RCX(CurrentPosition, WorkAreaAddress);
		//CurrentPosition = EmitPAUSE(CurrentPosition);
		//CurrentPosition = EmitJMPShort(CurrentPosition, -4);
		//CurrentPosition = EmitUD2(CurrentPosition);
		// Salta all'entry point del modulo a 64 bit
		CurrentPosition = EmitJMP64(CurrentPosition, EntryPoint);

		return true;
	}

	bool DestroyStartupBlock()
	{
		// Dealloca memoria "convezionale"
		return Processor::DpmiDeallocateConventional(StartupBlockSelector);
	}

	void DelayTicks(uint32_t Ticks)
	{
		_mm_lfence();
		uint64_t Start = __rdtsc();
		_mm_lfence();
		while (true)
		{
			_mm_pause();
			_mm_lfence();
			uint64_t Current = __rdtsc();
			if ((Current - Start) >= Ticks)
				break;
		}
	}

	bool InitLapicMapping()
	{
		uint32_t PhysicalAPIC = 0xFEE00000; // Base standard dell'APIC hardware
		uint32_t Size = 4096;               // L'APIC occupa una pagina da 4KB
		bool Result;

		Result = Processor::DpmiPhysicalAddressMapping((void*)PhysicalAPIC, Size, (void**)&LapicLinearBase);
		if (!Result || LapicLinearBase == 0)
			return false;
		return true;
	}

	bool DoneLapicMapping()
	{
		uint32_t PhysicalAPIC = 0xFEE00000; // Base standard dell'APIC hardware

		return Processor::DpmiFreePhysicalAddress((void*)PhysicalAPIC);
	}

	void SendIIPI(uint32_t ApicId, uint32_t LapicBase)
	{
		volatile uint32_t* IcrLow = (volatile uint32_t*)(LapicBase + 0x0300);
		volatile uint32_t* IcrHigh = (volatile uint32_t*)(LapicBase + 0x0310);

		// Aspetta che l'APIC sia libero (bit 12 == 0)
		while (*IcrLow & 0x00001000)
			_mm_pause();

		// Scrive il target core in ICR High
		*IcrHigh = (ApicId << 24);

		// Invia INIT IPI (0x4500)
		*IcrLow = 0x000C4500;

		// Aspetta che l'IPI sia inviato
		while (*IcrLow & 0x00001000)
			_mm_pause();
	}

	void SendSIPI(uint32_t ApicId, uint32_t VectorPage, uint32_t LapicBase)
	{
		volatile uint32_t* IcrLow = (volatile uint32_t*)(LapicBase + 0x0300);
		volatile uint32_t* IcrHigh = (volatile uint32_t*)(LapicBase + 0x0310);

		while (*IcrLow & 0x00001000)
			_mm_pause();

		*IcrHigh = (ApicId << 24);

		// Invia STARTUP IPI (0x4600 | VectorPage)
		*IcrLow = 0x000C4600 | (VectorPage & 0xFF);

		while (*IcrLow & 0x00001000)
			_mm_pause();
	}

	void EnableLocalApicSoftware()
	{
		// Il registro Spurious Interrupt Vector si trova a 0xFEE000F0
		//volatile uint32_t* Pointer = (volatile uint32_t*)0xFEE000F0;
		volatile uint32_t* Pointer = (volatile uint32_t*)(LapicLinearBase + 0x00F0);

		// Legge il valore attuale, imposta il bit 8 (Software Enable) 
		// e assegna un vettore di interrupt spurio (es. 0xFF)
		*Pointer = (*Pointer & ~0xFF) | 0x1FF;
	}

	bool RunStartup()
	{
		uint32_t VectorPage = AddressTrampoline >> 12;

		if (!InitLapicMapping())
			return false;

		__asm { finit }

		// EnableLocalApicHardware(); #GPF a Ring-3
		EnableLocalApicSoftware();

		// Invia INIT IPI
		SendIIPI(1, LapicLinearBase);
		DelayTicks(Processor::DoubleToInt32(Processor::CpuFrequencyHz * 0.01));

		// Invia il primo STARTUP IPI
		SendSIPI(1, VectorPage, LapicLinearBase);
		DelayTicks(Processor::DoubleToInt32(Processor::CpuFrequencyHz * 0.0002));

		// Invia il secondo STARTUP IPI se necessario
		if (!HasStarted())
		{
			SendSIPI(1, VectorPage, LapicLinearBase);
			DelayTicks(Processor::DoubleToInt32(Processor::CpuFrequencyHz * 0.0002));
		}

		return true;
	}

	bool LoadAndStartCore1()
	{
		// Configura il trampolino per saltare all'indirizzo della Dos64Main
		// Avvia il Core 1 ed esegue il trampolino
		if (!CreateStartupBlock())
			return false;
		//ClearCommandQueue();
		return RunStartup();
	}

	void StopAndUnloadCore1()
	{
		// Spegne il Core 1 e libera la memoria mappata
		PushCommand(cmShutdown, 0, 0, 0);
		SendIIPI(1, LapicLinearBase);
		DelayTicks(Processor::DoubleToInt32(Processor::CpuFrequencyHz * 0.001));
		DoneLapicMapping();
		DestroyStartupBlock();
	}

	void ClearCommandQueue()
	{
		WorkArea->CommandHead = 0;
		WorkArea->CommandTail = 0;
	}

	void PushCommand(uint32_t CommandCode, uint32_t Arg0, uint32_t Arg1, uint32_t Arg2)
	{
		volatile uint32_t CommandHead;
		volatile uint32_t CommandTail;
		volatile uint32_t CommandAddress;
		volatile PCommand Command;
		uint32_t NextHead;

		// Calcoliamo il prossimo indice
		CommandHead = WorkArea->CommandHead;
		CommandTail = WorkArea->CommandTail;
		NextHead = (CommandHead + 1) % WorkArea->CommandSize;

		// Controlla se la coda è piena
		if (NextHead == CommandTail)
			return;

		// Scriviamo i dati del comando nell'elemento corrente
		CommandAddress = CommandQueueLinearAddress + NextHead * sizeof(TCommand);
		Command = reinterpret_cast<PCommand>((void*)CommandAddress);
		Command->CommandCode = CommandCode;
		Command->Arg0 = Arg0;
		Command->Arg1 = Arg1;
		Command->Arg2 = Arg2;

		_mm_sfence();
		WorkArea->CommandHead = NextHead;
	}
}
