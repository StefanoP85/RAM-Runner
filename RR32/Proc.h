/*
 * ============================================================================
 * Project: RAM-Runner (32-bit DOS Engine)
 * File:    Proc.h
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Common routines
 * ============================================================================
 */
#pragma once

typedef signed char            int8_t;
typedef short int              int16_t;
typedef long int               int32_t;
typedef long long int          int64_t;
typedef unsigned char          uint8_t;
typedef unsigned short         uint16_t;
typedef unsigned long int      uint32_t;
typedef unsigned long long int uint64_t;

namespace RamRunner::Processor
{
#pragma pack(push, 1)
	struct TDpmiRequestRegs
	{
		uint32_t EDI;
		uint32_t ESI;
		uint32_t EBP;
		uint32_t Reserved;
		uint32_t EBX;
		uint32_t EDX;
		uint32_t ECX;
		uint32_t EAX;
		uint16_t Flags;
		uint16_t ES;
		uint16_t DS;
		uint16_t FS;
		uint16_t GS;
		uint16_t IP;
		uint16_t CS;
		uint16_t SP;
		uint16_t SS;
	};
#pragma pack(pop)

	extern "C"
	{
		void CallRDTSC(uint32_t* EAX, uint32_t* EDX);
		bool IsPentiumOrNewer();
		bool DpmiAllocate(uint32_t Size, uint32_t* Handle, void** Buffer);
		bool DpmiDeallocate(uint32_t Handle);
		bool DpmiAllocateConventional(uint32_t Size, uint16_t* Selector, void** Buffer);
		bool DpmiDeallocateConventional(uint16_t Selector);
		bool DpmiPhysicalAddressMapping(void* PhysicalAddress, uint32_t Size, void** LinearAddress);
		bool DpmiFreePhysicalAddress(void* LinearAddress);
		bool DpmiLockRegion(void* Address, uint32_t RegionSize);
		bool DpmiUnlockRegion(void* Address, uint32_t RegionSize);
		bool DpmiSimulateRealModeInterrupt(uint8_t IntNo, TDpmiRequestRegs* Regs);
		int32_t DoubleToInt32(double Value);
		int64_t DoubleToInt64(double Value);
	}
	double CalibrateCpuFrequencyHz(uint32_t WaitTicks);
	bool DosFileClose(uint16_t Handle);
	uint16_t DosFileOpen(const char* FileName);
	uint16_t DosFileRead(uint16_t Handle, void* Buffer, uint16_t Length);
	bool DosFileSeek(uint16_t Handle, uint32_t Position);
	double Int64ToDouble(uint32_t Low, uint32_t High);
	void IntToString(int32_t Value, char* Buffer);
	void MemCopy(void* Destination, const void* Source, uint32_t NumBytes);
	void MemZero(void* Destination, uint32_t NumBytes);
	int32_t StrComp(const char* String1, const char* String2);
}
