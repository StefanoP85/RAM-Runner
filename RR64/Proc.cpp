/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Proc.cpp
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Common routines
 * ============================================================================
 */
#include "Proc.h"

namespace RamRunner::Processor
{
	// Risolve l'errore "undefined reference to _fltused" in MSVC senza librerie standard
	extern "C" int _fltused = 0;
	
	// Analisi della CPU
	double CpuFrequencyHz;
	bool Is64bit;
	bool IsMultiCore;

	// Gestione della memoria XMS
	uint32_t XMSPhysicalAddress;
	uint32_t XMSLinearAddress;

	double Int64ToDouble(uint32_t Low, uint32_t High)
	{
		return (double)Low + ((double)High * 4294967296.0);
	}

	uint32_t GetClockTicks()
	{
		uint32_t* Pointer = (uint32_t*)0x046C;
		return *Pointer;
	}

	double CalibrateCpuFrequencyHz(uint32_t WaitTicks)
	{
		volatile uint32_t StartTick;
		volatile uint32_t StopTick;
		uint32_t T1Low, T1High;
		uint32_t T2Low, T2High;
		double T1, T2;
		double ElapsedTicks;
		double ElapsedSeconds;

		__asm { finit }

		// Sincronizzazione con il cambio del tick del BIOS
		StartTick = GetClockTicks();
		while (GetClockTicks() == StartTick)
			;
		CallRDTSC(&T1Low, &T1High);
		StopTick = GetClockTicks();
		while ((GetClockTicks() - StartTick) < WaitTicks)
			;
		CallRDTSC(&T2Low, &T2High);
		T1 = Int64ToDouble(T1Low, T1High);
		T2 = Int64ToDouble(T2Low, T2High);
		ElapsedTicks = T2 - T1;
		ElapsedSeconds = (19886.0 * 3.0 * WaitTicks) / 1193182.0;
		return ElapsedTicks / ElapsedSeconds;
	}

	void CheckProcessor(void)
	{
		TRegs Regs;
		uint32_t MaxEax;
		uint32_t MaxEaxExt;
		uint32_t Cores, IntelCores, AmdCores;

		// Misura la frequenza della CPU
		CpuFrequencyHz = CalibrateCpuFrequencyHz(20);
		// Ottiene MaxEax
		CallCPUID(0, 0, &Regs);
		MaxEax = Regs.EAX;
		// Ottiene MaxEaxExt
		CallCPUID(0x80000000, 0, &Regs);
		// Se la CPU non supporta le funzioni estese, restituirà un valore < 0x80000000
		if (Regs.EAX >= 0x80000000)
			MaxEaxExt = Regs.EAX;
		else
			MaxEaxExt = 0; // Nessuna funzione estesa supportata
		// Verifica CPU a 64 bit
		if (MaxEaxExt >= 0x80000001)
		{
			CallCPUID(0x80000001, 0, &Regs);
			if (Regs.EDX & 0x20000000)
				Is64bit = true;
			else
				Is64bit = false;
		}
		else
			Is64bit = false;
		// Verifica se la CPU ha almeno due core fisici
		Cores = 1;
		// Controllo per CPU Intel (Leaf 0x04)
		if (MaxEax >= 0x00000004)
		{
			CallCPUID(0x00000004, 0, &Regs);
			IntelCores = (Regs.EAX >> 26) + 1;
			if (IntelCores > Cores)
				Cores = IntelCores;
		}
		// Controllo per CPU AMD (Extended Leaf 0x80000008)
		if (MaxEaxExt >= 0x80000008)
		{
			CallCPUID(0x80000008, 0, &Regs);
			AmdCores = (Regs.ECX & 0xFF) + 1;
			if (AmdCores > Cores)
				Cores = AmdCores;
		}
		IsMultiCore = (Cores > 1);
	}

	bool DosFileClose(uint16_t Handle)
	{
		bool Result;

		Result = false;
		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x3E00;  // Servizio DOS: AH = 3E, Close File
		Regs.EBX = Handle;  // Handle
		if ((Processor::DpmiSimulateRealModeInterrupt(0x21, &Regs)) && ((Regs.Flags & 0x0001) == 0))
			Result = true;
		return Result;
	}

	uint16_t DosFileOpen(const char* FileName)
	{
		uint32_t Index;
		char* DosFileName;
		uint16_t Selector;
		uint16_t Result;

		Result = 0;
		DosFileName = nullptr;
		if (!Processor::DpmiAllocateConventional(128, &Selector, (void**)&DosFileName))
			return 0;

		for (Index = 0; FileName[Index] != '\0'; Index++)
			DosFileName[Index] = FileName[Index];
		DosFileName[Index] = '\0';
		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x3D00;  // Servizio DOS: AH = 3D, Open File; AL = 00, read only
		Regs.EDX = 0;       // Indirizzo 0
		Regs.DS = (uint16_t)(reinterpret_cast<uint32_t>(DosFileName) >> 4);
		Regs.ES = Regs.DS;
		if ((DpmiSimulateRealModeInterrupt(0x21, &Regs)) && ((Regs.Flags & 0x0001) == 0))
			Result = Regs.EAX & 0x0FFFF;
		Processor::DpmiDeallocateConventional(Selector);
		return Result;
	}

	uint16_t DosFileRead(uint16_t Handle, void* Buffer, uint16_t Length)
	{
		uint16_t Result;

		Result = 0;
		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x3F00;  // Servizio DOS: AH = 3F, Read File
		Regs.EBX = Handle;  // Handle
		Regs.ECX = Length;  // Numero di byte da leggere
		Regs.EDX = 0;       // Indirizzo 0
		Regs.DS = (uint16_t)(reinterpret_cast<uint32_t>(Buffer) >> 4);
		Regs.ES = Regs.DS;
		if ((Processor::DpmiSimulateRealModeInterrupt(0x21, &Regs)) && ((Regs.Flags & 0x0001) == 0))
			Result = Regs.EAX & 0x0FFFF;
		return Result;
	}

	bool DosFileSeek(uint16_t Handle, uint32_t Position)
	{
		bool Result;

		Result = false;
		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x4200;  // Servizio DOS: AH = 42, Close File, AL = 00h, From start of file
		Regs.EBX = Handle;  // Handle
		Regs.ECX = (Position >> 16);
		Regs.EDX = (Position & 0xFFFF);
		if ((Processor::DpmiSimulateRealModeInterrupt(0x21, &Regs)) && ((Regs.Flags & 0x0001) == 0) && (((Regs.EDX & 0xFFFF) << 16) + (Regs.EAX & 0xFFFF)))
			Result = true;
		return Result;
	}

	void IntToString(int32_t Value, char* Buffer)
	{
		char Temp[12];
		char* Pointer;
		uint32_t I;
		uint32_t UValue;

		// Caso speciale Value = 0
		if (Value == 0)
		{
			Buffer[0] = '0';
			Buffer[1] = '\0';
			return;
		}

		I = 11;
		Temp[I] = '\0';
		UValue = (Value < 0) ? -Value : Value;
		// Estraiamo le cifre da destra a sinistra
		while (UValue > 0)
		{
			I--;
			Temp[I] = (char)((UValue % 10) + '0');
			UValue /= 10;
		}
		// Aggiungiamo il segno meno se il numero originale era negativo
		if (Value < 0)
		{
			I--;
			Temp[I] = '-';
		}
		Pointer = &Temp[I];
		while (*Pointer != '\0')
		{
			*Buffer = *Pointer;
			Buffer++;
			Pointer++;
		}
		*Buffer = '\0';
	}

	// Funzione di copia memoria in stile memcpy senza librerie standard
	void MemCopy(void* Destination, const void* Source, uint32_t NumBytes)
	{
		uint8_t* Dest = (uint8_t*)Destination;
		const uint8_t* Src = (const uint8_t*)Source;
		// Copia byte per byte fino a raggiungere il numero totale specificato
		while (NumBytes > 0)
		{
			*Dest = *Src;
			Dest++;
			Src++;
			NumBytes--;
		}
	}

	// Funzione di azzeramento della memoria senza librerie standard
	void MemZero(void* Destination, uint32_t NumBytes)
	{
		uint8_t* Dest = (uint8_t*)Destination;
		// Copia byte per byte fino a raggiungere il numero totale specificato
		while (NumBytes > 0)
		{
			*Dest = 0;
			Dest++;
			NumBytes--;
		}
	}

	// Funzione di confronto stringhe in stile strcmp senza librerie standard
	// Restituisce 0 se le stringhe sono identiche, 
	// un valore < 0 se String1 viene prima di String2, 
	// un valore > 0 se String1 viene dopo String2.
	int32_t StrComp(const char* String1, const char* String2)
	{
		// Cicla finché i caratteri sono uguali e non siamo arrivati alla fine della stringa (\0)
		while (*String1 && (*String1 == *String2))
		{
			String1++;
			String2++;
		}
		// Restituisce la differenza numerica del primo carattere divergente (o 0 se uguali)
		return *(unsigned char*)String1 - *(unsigned char*)String2;
	}

	// Inizializza il blocco di memoria XMS allocato dallo STUB
	bool XMSInitialize()
	{
		constexpr uint8_t RequestHeyYou = 0xB3;
		constexpr uint8_t RequestHandshake = 0x00;
		constexpr uint8_t RequestPhysAddress = 0x01;
		constexpr uint16_t ResponseHereIam = 0xA1C9;
		bool Result;

		TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		// Verifica presenza dello STUB START.EXE
		Regs.EAX = (RequestHeyYou << 8) | RequestHandshake;
		if ((DpmiSimulateRealModeInterrupt(0x2F, &Regs)) && ((Regs.EAX & 0xFFFF) == ResponseHereIam))
		{
			// Ottiene l'indirizzo FISICO del blocco XMS
			Regs.EAX = (RequestHeyYou << 8) | RequestPhysAddress;
			if (DpmiSimulateRealModeInterrupt(0x2F, &Regs))
			{
				XMSPhysicalAddress = Regs.EAX;
				Result = DpmiPhysicalAddressMapping((void*)XMSPhysicalAddress, 12 * 1024 * 1024, (void**)&XMSLinearAddress);
			}
			else
				Result = false;
		}
		else
			Result = false;
		return Result;
	}

	// Finalizza il blocco di memoria XMS allocato dallo STUB
	void XMSFinalize()
	{
		DpmiFreePhysicalAddress((void*)XMSLinearAddress);
	}

	uint32_t GetXMSAddress()
	{
		return ((XMSLinearAddress + 63) & 0xFFFFFFC0);
	}

	uint32_t GetXMSPhysicalAddress()
	{
		return ((XMSPhysicalAddress + 63) & 0xFFFFFFC0);
	}
}
