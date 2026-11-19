/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Console.cpp
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Keyboard and text mode screen handling
 * ============================================================================
 */
#include "Console.h"

namespace RamRunner::Console
{
	void DosReadKbd()
	{
		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x0700;
		Processor::DpmiSimulateRealModeInterrupt(0x21, &Regs);
	}

	uint8_t DosEmptyReadKbd()
	{
		uint8_t Result;

		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x0C07;
		Processor::DpmiSimulateRealModeInterrupt(0x21, &Regs);
		Result = (uint8_t)Regs.EAX;
		return Result;
	}

	uint8_t DosIsCharAvail()
	{
		uint8_t Result;

		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x0B00;
		Processor::DpmiSimulateRealModeInterrupt(0x21, &Regs);
		Result = (uint8_t)Regs.EAX;
		return Result;
	}

	void GotoXY(uint8_t X, uint8_t Y)
	{
		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x0200;
		Regs.EBX = 0;
		Regs.EDX = (Y << 8) | X;
		Processor::DpmiSimulateRealModeInterrupt(0x10, &Regs);
	}

	void ClrScr()
	{
		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x0600;
		Regs.EBX = 0x0700;
		Regs.ECX = 0;
		Regs.EDX = 0x314F;
		Processor::DpmiSimulateRealModeInterrupt(0x10, &Regs);
		Regs.EAX = 0x0200;
		Regs.EBX = 0;
		Regs.EDX = 0;
		Processor::DpmiSimulateRealModeInterrupt(0x10, &Regs);
	}

	void TextMode80x25()
	{
		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x0003;
		Processor::DpmiSimulateRealModeInterrupt(0x10, &Regs);
	}

	uint16_t BIOSReadKey()
	{
		uint16_t Result;

		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x1000;
		Processor::DpmiSimulateRealModeInterrupt(0x16, &Regs);
		Result = (uint16_t)Regs.EAX;
		return Result;
	}

	uint8_t ReadKey()
	{
		uint16_t Key;
		uint8_t ScanCode, AsciiCode;

		Key = BIOSReadKey();
		AsciiCode = Key & 0x00FF;
		ScanCode = (Key & 0xFF00) >> 8;
		if (AsciiCode == 224)
			return ScanCode;
		else
			return AsciiCode;
	}

	void FlushKeyboard()
	{
		while (DosIsCharAvail())
			DosReadKbd();
	}

	void DrawChar(uint16_t X, uint16_t Y, char Code, char Attribute)
	{
		char* Pointer;

		Pointer = (char*)(void*)(0xB8000 + (Y << 7) + (Y << 5) + (X << 1));
		*Pointer = Code;
		Pointer++;
		*Pointer = Attribute;
	}

	void DrawString(uint16_t X, uint16_t Y, const char* AString, uint8_t Attribute)
	{
		char* Pointer;

		Pointer = (char*)(void*)(0xB8000 + (Y << 7) + (Y << 5) + (X << 1));
		while (*AString)
		{
			*Pointer = *AString;
			Pointer++;
			*Pointer = Attribute;
			Pointer++;
			AString++;
		}
	}
}
