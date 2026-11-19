/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Core64.cpp
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Main source for the 64 bit Core 1
 * ============================================================================
 */
#include <intrin.h>
#include "Core64.h"

namespace RamRunner::Processor64
{
	// Risolve l'errore "undefined reference to __ImageBase" in MSVC senza librerie standard
	extern "C" int __ImageBase = 0;

	// Offset della finestra di gioco, 24 pixel a destra e 24 pixel in basso
	constexpr uint64_t OffsetX = 24 * 4;
	constexpr uint64_t OffsetY = 24 * 4096;
	constexpr uint64_t OffsetTile = OffsetX + OffsetY;
	constexpr uint64_t OffsetSprite = (8 << 12) + (8 << 2);

	void Init(TProcessorInfo* ProcessorInfo)
	{
		ProcessorInfo->MaxEax = 0;
		ProcessorInfo->MaxEaxExt = 0;
		ProcessorInfo->FeatureFlags = 0;
		ProcessorInfo->Xsave = false;
		ProcessorInfo->OsXsave = false;
		ProcessorInfo->OsAvxState = false;
		ProcessorInfo->OsAvx512State = false;
		ProcessorInfo->VendorId[0] = '\0';
		ProcessorInfo->ProcessorBrand[0] = '\0';
		ProcessorInfo->SSEAvailable = false;
		ProcessorInfo->AVXAvailable = false;
		ProcessorInfo->AVX512Available;
	}
	void InitProcessorBrand(TProcessorInfo* ProcessorInfo)
	{
		if (ProcessorInfo->MaxEaxExt >= 0x80000004)
		{
			TRegs Regs2, Regs3, Regs4;
			char* Pointer = ProcessorInfo->ProcessorBrand;
			CallCPUID64(0x80000002, 0, &Regs2);
			CallCPUID64(0x80000003, 0, &Regs3);
			CallCPUID64(0x80000004, 0, &Regs4);
			*(uint32_t*)(Pointer + 0) = Regs2.EAX;
			*(uint32_t*)(Pointer + 4) = Regs2.EBX;
			*(uint32_t*)(Pointer + 8) = Regs2.ECX;
			*(uint32_t*)(Pointer + 12) = Regs2.EDX;
			*(uint32_t*)(Pointer + 16) = Regs3.EAX;
			*(uint32_t*)(Pointer + 20) = Regs3.EBX;
			*(uint32_t*)(Pointer + 24) = Regs3.ECX;
			*(uint32_t*)(Pointer + 28) = Regs3.EDX;
			*(uint32_t*)(Pointer + 32) = Regs4.EAX;
			*(uint32_t*)(Pointer + 36) = Regs4.EBX;
			*(uint32_t*)(Pointer + 40) = Regs4.ECX;
			*(uint32_t*)(Pointer + 44) = Regs4.EDX;
			ProcessorInfo->ProcessorBrand[sizeof(ProcessorInfo->ProcessorBrand) - 1] = '\0';
		}
		else
			ProcessorInfo->ProcessorBrand[0] = '\0';
	}
	void LoadInfo0(TProcessorInfo* ProcessorInfo)
	{
		TRegs Regs;
		// Ottiene MaxEax e VendorID
		CallCPUID64(0, 0, &Regs);
		ProcessorInfo->MaxEax = Regs.EAX;
		*(uint32_t*)(ProcessorInfo->VendorId + 0) = Regs.EBX;
		*(uint32_t*)(ProcessorInfo->VendorId + 4) = Regs.EDX;
		*(uint32_t*)(ProcessorInfo->VendorId + 8) = Regs.ECX;
		ProcessorInfo->VendorId[sizeof(ProcessorInfo->VendorId) - 1] = '\0';
		// Ottiene MaxEaxExt
		CallCPUID64(0x80000000, 0, &Regs);
		// Se la CPU non supporta le funzioni estese, restituirà un valore < 0x80000000
		if (Regs.EAX >= 0x80000000)
			ProcessorInfo->MaxEaxExt = Regs.EAX;
		else
			ProcessorInfo->MaxEaxExt = 0; // Nessuna funzione estesa supportata
	}
	void LoadInfo1(TProcessorInfo* ProcessorInfo)
	{
		TRegs Regs;
		if (ProcessorInfo->MaxEax < 1)
			return;
		CallCPUID64(1, 0, &Regs);
		// CPUID.(EAX=01H, ECX=00H):ECX.SSE3[bit 0]
		if (Regs.ECX & (0x1 << 0))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::SSE3;
		// CPUID.(EAX=01H, ECX=00H):ECX.PCLMULQDQ[bit 1]
		if (Regs.ECX & (0x1 << 1))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::PCLMULQDQ;
		// CPUID.(EAX=01H, ECX=00H):ECX.SSSE3[bit 9]
		if (Regs.ECX & (0x1 << 9))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::SSSE3;
		// CPUID.(EAX=01H, ECX=00H):ECX.SSE4.1[bit 19]
		if (Regs.ECX & (0x1 << 19))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::SSE4_1;
		// CPUID.(EAX=01H, ECX=00H):ECX.SSE4.2[bit 20]
		if (Regs.ECX & (0x1 << 20))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::SSE4_2;
		// CPUID.(EAX=01H, ECX=00H):ECX.MOVBE[bit 22]
		if (Regs.ECX & (0x1 << 22))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::MOVBE;
		// CPUID.(EAX=01H, ECX=00H):ECX.POPCNT[bit 23]
		if (Regs.ECX & (0x1 << 23))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::POPCNT;
		// CPUID.(EAX=01H, ECX=00H):ECX.RDRAND[bit 30]
		if (Regs.ECX & (0x1 << 30))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::RDRAND;
		// CPUID.(EAX=01H, ECX=00H):EDX.MMX[bit 23]
		if (Regs.EDX & (0x1 << 23))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::MMX;
		// CPUID.(EAX=01H, ECX=00H):EDX.FXSR[bit 24]
		if (Regs.EDX & (0x1 << 24))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::FXSR;
		// CPUID.(EAX=01H, ECX=00H):EDX.SSE[bit 25]
		if (Regs.EDX & (0x1 << 25))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::SSE;
		// CPUID.(EAX=01H, ECX=00H):EDX.SSE2[bit 26]
		if (Regs.EDX & (0x1 << 26))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::SSE2;
	}
	void LoadInfo2(TProcessorInfo* ProcessorInfo)
	{
		TRegs Regs;
		if (ProcessorInfo->MaxEax < 7)
			return;
		CallCPUID64(7, 0, &Regs);
		// CPUID.(EAX=07H, ECX=00H):ECX.PREFETCHWT1[bit 0]
		if (Regs.ECX & (0x1 << 0))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::PREFETCHWT1;
		// CPUID.(EAX=07H, ECX=00H):EBX.BMI1[bit 3]
		if (Regs.EBX & (0x1 << 3))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::BMI1;
		// CPUID.(EAX=07H, ECX=00H):EBX.BMI2[bit 8]
		if (Regs.EBX & (0x1 << 8))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::BMI2;
		// CPUID.(EAX=07H, ECX=00H):EBX.ERMSB[bit 9]
		// ERMSB = Enhanced REP MOVSB/STOSB
		if (Regs.EBX & (0x1 << 9))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::ERMSB;
		// CPUID.(EAX=07H, ECX=00H):EBX.RDSEED[bit 18]
		if (Regs.EBX & (0x1 << 18))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::RDSEED;
		// CPUID.(EAX=07H, ECX=00H):EBX.ADX[bit 19]
		if (Regs.EBX & (0x1 << 19))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::ADX;
		// CPUID.(EAX=07H, ECX=00H):EBX.CLWB[bit 24]
		if (Regs.EBX & (0x1 << 24))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::CLWB;
	}
	void LoadInfo3(TProcessorInfo* ProcessorInfo)
	{
		TRegs Regs;
		if (ProcessorInfo->MaxEaxExt < 0x80000001)
			return;
		CallCPUID64(0x80000001, 0, &Regs);
		// CPUID.(EAX=80000001H, ECX=00H):ECX.LZCNT[bit 5]
		if (Regs.ECX & (0x1 << 5))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::LZCNT;
		// CPUID.(EAX=80000001H, ECX=00H):ECX.PREFETCHW[bit 8]
		if (Regs.ECX & (0x1 << 8))
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::PREFETCHW;
	}
	/*void LoadInfo4(TProcessorInfo* ProcessorInfo)
	{
		TRegs Regs_EAX01h;
		TRegs Regs_EAX07h;
		if (ProcessorInfo->MaxEax < 7)
			return;
		CallCPUID64(1, 0, &Regs_EAX01h);
		CallCPUID64(7, 0, &Regs_EAX07h);
		// Test CPUID.(EAX=01H, ECX=00H):ECX.XSAVE[bit 26]
		ProcessorInfo->Xsave = (Regs_EAX01h.ECX & (0x1 << 27)) ? true : false;
		// Test CPUID.(EAX=01H, ECX=00H):ECX.OSXSAVE[bit 27]
		ProcessorInfo->OsXsave = (Regs_EAX01h.ECX & (0x1 << 27)) ? true : false;
		// CPUID.(EAX=01H, ECX=00H):ECX.AVX[bit 28]
		if (Regs_EAX01h.ECX & (0x1 << 28))
		{
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX;
			// CPUID.(EAX=01H, ECX=00H):ECX.FMA[bit 12]
			if (Regs_EAX01h.ECX & (0x1 << 12))
				ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::FMA;
			// CPUID.(EAX=01H, ECX=00H):ECX.F16C[bit 29]
			if (Regs_EAX01h.ECX & (0x1 << 29))
				ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::F16C;
			// CPUID.(EAX=07H, ECX=00H):EBX.AVX2[bit 5]
			if (Regs_EAX07h.EBX & (0x1 << 5))
				ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX2;
			// CPUID.(EAX=07H, ECX=00H):EBX.AVX512F[bit 16]
			if (Regs_EAX07h.EBX & (0x1 << 16))
			{
				ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512F;
				// CPUID.(EAX=07H, ECX=00H):EBX.AVX512DQ[bit 17]
				if (Regs_EAX07h.EBX & (0x1 << 17))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512DQ;
				// CPUID.(EAX=07H, ECX=00H):EBX.AVX512_IFMA[bit 21]
				if (Regs_EAX07h.EBX & (0x1 << 21))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512_IFMA;
				// CPUID.(EAX=07H, ECX=00H):EBX.AVX512PF[bit 26]
				if (Regs_EAX07h.EBX & (0x1 << 26))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512PF;
				// CPUID.(EAX=07H, ECX=00H):EBX.AVX512ER[bit 27]
				if (Regs_EAX07h.EBX & (0x1 << 27))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512ER;
				// CPUID.(EAX=07H, ECX=00H):EBX.AVX512CD[bit 28]
				if (Regs_EAX07h.EBX & (0x1 << 28))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512CD;
				// CPUID.(EAX=07H, ECX=00H):EBX.AVX512BW[bit 30]
				if (Regs_EAX07h.EBX & (0x1 << 30))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512BW;
				// CPUID.(EAX=07H, ECX=00H):EBX.AVX512VL[bit 31]
				if (Regs_EAX07h.EBX & (0x1 << 31))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512VL;
				// CPUID.(EAX=07H, ECX=00H):ECX.AVX512_VBMI[bit 1]
				if (Regs_EAX07h.ECX & (0x1 << 1))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512_VBMI;
				// CPUID.(EAX=07H, ECX=00H):ECX.AVX512_VBMI2[bit 6]
				if (Regs_EAX07h.ECX & (0x1 << 6))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512_VBMI2;
				// CPUID.(EAX=07H, ECX=00H):ECX.AVX512_VNNI[bit 11]
				if (Regs_EAX07h.ECX & (0x1 << 11))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512_VNNI;
				// CPUID.(EAX=07H, ECX=00H):ECX.AVX512_BITALG[bit 12]
				if (Regs_EAX07h.ECX & (0x1 << 12))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512_BITALG;
				// CPUID.(EAX=07H, ECX=00H):ECX.AVX512_VPOPCNTDQ[bit 14]
				if (Regs_EAX07h.ECX & (0x1 << 14))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512_VPOPCNTDQ;
				// CPUID.(EAX=07H, ECX=00H):EDX.AVX512_4FMAPS[bit 2]
				if (Regs_EAX07h.EDX & (0x1 << 2))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512_4FMAPS;
				// CPUID.(EAX=07H, ECX=00H):EDX.AVX512_4VNNIW[bit 3]
				if (Regs_EAX07h.EDX & (0x1 << 3))
					ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512_4VNNIW;
			}
		}
	}*/
	void LoadInfo4(TProcessorInfo* ProcessorInfo)
	{
		TRegs Regs_EAX01h;
		TRegs Regs_EAX07h;
		if (ProcessorInfo->MaxEax < 1)
			return;
		CallCPUID64(1, 0, &Regs_EAX01h);
		// Test XSAVE e OSXSAVE (bit 26 e 27 di ECX alla funzione 1)
		ProcessorInfo->Xsave = (Regs_EAX01h.ECX & (0x1 << 26)) ? true : false;
		ProcessorInfo->OsXsave = (Regs_EAX01h.ECX & (0x1 << 27)) ? true : false;
		// CPUID.(EAX=01H):ECX.AVX[bit 28]
		if (Regs_EAX01h.ECX & (0x1 << 28))
		{
			ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX;
			if (Regs_EAX01h.ECX & (0x1 << 12))
				ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::FMA;
			if (Regs_EAX01h.ECX & (0x1 << 29))
				ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::F16C;
		}
		if (ProcessorInfo->MaxEax >= 7)
		{
			CallCPUID64(7, 0, &Regs_EAX07h);
			// CPUID.(EAX=07H, ECX=00H):EBX.AVX2[bit 5]
			if (Regs_EAX07h.EBX & (0x1 << 5))
				ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX2;
			// Eventuali controlli AVX-512 (se supportati dalla CPU e dall'OS)
			if (Regs_EAX07h.EBX & (0x1 << 16))
				ProcessorInfo->FeatureFlags |= (uint64_t)TFeatureFlags::AVX512F;
		}
	}
	void LoadInfo(TProcessorInfo* ProcessorInfo)
	{
		Init(ProcessorInfo);
		LoadInfo0(ProcessorInfo);
		LoadInfo1(ProcessorInfo);
		LoadInfo2(ProcessorInfo);
		LoadInfo3(ProcessorInfo);
		LoadInfo4(ProcessorInfo);
		InitProcessorBrand(ProcessorInfo);
	}

	/*void EnableExtensions(TProcessorInfo* ProcessorInfo)
	{
		uint32_t RegEAX, RegEDX;

		// ABILITAZIONE HARDWARE FPU & SSE (Registri CR0 e CR4)
		uint64_t cr0 = __readcr0();
		cr0 &= ~(1ULL << 2);  // Azzera CR0.EM (Emulation) -> Forza l'uso della FPU hardware
		cr0 |= (1ULL << 1);   // Imposta CR0.MP (Monitor Coprocessor)
		__writecr0(cr0);
		uint64_t cr4 = __readcr4();
		cr4 |= (1UL << 9);    // Imposta CR4.OSFXSR -> Abilita i registri XMM e istruzioni FXSAVE/FXRSTOR
		cr4 |= (1UL << 10);   // Imposta CR4.OSXMMEXCPT -> Gestione nativa eccezioni #XM
		if (ProcessorInfo->Xsave)
			cr4 |= (1UL << 18);   // Imposta CR4.OSXSAVE -> Sblocca XGETBV e XSETBV
		__writecr4(cr4);
		// Di base, lo stato SSE richiesto per PAND/PANDN/POR a questo punto è attivo
		ProcessorInfo->SSEAvailable = true;
		ProcessorInfo->AVXAvailable = false; // Default di sicurezza, viene verificato sotto

		// ATTIVAZIONE DI AVX
		if ((ProcessorInfo->FeatureFlags & TFeatureFlags::AVX) && (ProcessorInfo->FeatureFlags & TFeatureFlags::AVX2))
		{
			// Configura la maschera degli stati estesi per XCR0 (Extended Control Register 0)
			CallXGETBV64(0, &RegEAX, &RegEDX);
			RegEAX |= 0x01; // Bit 0 = x87 FPU/MMX State (Sempre 1)
			RegEAX |= 0x02; // Bit 1 = SSE State (Abilita i registri XMM)
			RegEAX |= 0x04; // Bit 2 = AVX State (Abilita i registri YMM superiori per VPBLENDVB)
			uint64_t xcr0_mask = (1ULL << 0) | (1ULL << 1) | (1ULL << 2);
			uint32_t eax_mask = (uint32_t)(xcr0_mask & 0xFFFFFFFF);
			uint32_t edx_mask = (uint32_t)((xcr0_mask >> 32) & 0xFFFFFFFF);
			CallXSETBV64(0, RegEAX, RegEDX);
			ProcessorInfo->OsXsave = true;
			ProcessorInfo->AVXAvailable = true;
		}
	}*/

	void EnableExtensions(TProcessorInfo* ProcessorInfo)
	{
		TRegs Regs;
		uint32_t RegEAX, RegEDX;

		// 1. Abilitazione FPU hardware e gestione eccezioni SSE in CR0/CR4
		uint64_t cr0 = __readcr0();
		cr0 |= (1ULL << 1);   // Imposta CR0.MP
		cr0 &= ~(1ULL << 2);  // Azzera CR0.EM
		cr0 &= ~(1ULL << 3);  // Azzera CR0.TS
		__writecr0(cr0);
		uint64_t cr4 = __readcr4();
		cr4 |= (1UL << 9);    // CR4.OSFXSR (Abilita registri XMM e FXSAVE/FXRSTOR)
		cr4 |= (1UL << 10);   // CR4.OSXMMEXCPT (Eccezioni SSE native)
		// Le SSE di base sono sempre presenti nelle CPU a 64 bit
		ProcessorInfo->SSEAvailable = true;
		ProcessorInfo->AVXAvailable = false;
		// 2. Controllo e attivazione AVX (richiede supporto XSAVE della CPU e abilitazione OSXSAVE)
		if ((ProcessorInfo->Xsave) && (ProcessorInfo->FeatureFlags & TFeatureFlags::AVX) && (ProcessorInfo->FeatureFlags & TFeatureFlags::AVX2))
		{
			cr4 |= (1UL << 18);   // Imposta CR4.OSXSAVE
			__writecr4(cr4);
			// Legge lo stato attuale di XCR0 e abilita i bit 0 (x87), 1 (SSE) e 2 (AVX)
			CallXGETBV64(0, &RegEAX, &RegEDX);
			// Verifica che il sistema operativo abbia già sbloccato X87 e SSE, e sblocca AVX (bit 2)
			RegEAX |= 0x01; // Bit 0 = x87 FPU/MMX State (Sempre 1)
			RegEAX |= 0x02; // Bit 1 = SSE State (Abilita i registri XMM)
			RegEAX |= 0x04; // Bit 2 = AVX State (Abilita i registri YMM superiori per VPBLENDVB)
			CallXSETBV64(0, RegEAX, RegEDX);
			CallCPUID64(1, 0, &Regs);
			ProcessorInfo->OsXsave = (Regs.ECX & (0x1 << 27)) ? true : false;
			if (ProcessorInfo->OsXsave)
			{
				CallXGETBV64(0, &RegEAX, &RegEDX);
				ProcessorInfo->OsAvxState = (((RegEAX >> 1) & 0x03) == 0x03) ? true : false;
				if (ProcessorInfo->OsAvxState)
				{
					ProcessorInfo->AVXAvailable = true;
				}
			}
		}
		else
		{
			__writecr4(cr4);
		}
	}

	void TelevisionNoise()
	{
		uint64_t RandSeed = 1;
		while (true)
		{
			RandSeed = (RandSeed * 1103515245 + 12309) & 0x7FFFFFFF;
			uint8_t* Pointer = (uint8_t*)(void*)(0xB8000 + (RandSeed % 4000));
			RandSeed = (RandSeed * 1103515245 + 12309) & 0x7FFFFFFF;
			*Pointer = RandSeed % 256;
		}
	}

	PCommand PopCommand(const PWorkArea WorkArea)
	{
		volatile uint64_t CommandHead;
		volatile uint64_t CommandTail;
		volatile uint64_t CommandAddress;
		PCommand Command;
		uint64_t NextTail;

		// Controlla se la coda è vuota
		CommandHead = WorkArea->CommandHead;
		CommandTail = WorkArea->CommandTail;
		if (CommandHead == CommandTail)
			return nullptr;

		// Ottiene il puntatore al prossimo comando
		NextTail = (CommandTail + 1) % WorkArea->CommandSize;
		CommandAddress = WorkArea->CommandQueue + NextTail * sizeof(TCommand);
		Command = reinterpret_cast<PCommand>((void*)CommandAddress);
		WorkArea->CommandTail = NextTail;
		return Command;
	}

	void Main(const PWorkArea WorkArea)
	{
		TProcessorInfo ProcessorInfo;
		uint64_t MazeBufferPointer;
		uint64_t FrameBufferPointer;
		uint64_t VideoMemoryPointer;
		uint64_t CharSet;
		uint64_t SpriteSet;
		uint64_t TransparentMaskSet;
		uint64_t TileSet;
		TDrawSprite DrawSprite;
		uint64_t Index;
		uint64_t X;
		uint64_t Y;
		volatile PCommand Command;
		bool Running;

		// Analizza la CPU e abilita le estensioni AVX
		LoadInfo(&ProcessorInfo);
		EnableExtensions(&ProcessorInfo);
		if (ProcessorInfo.AVXAvailable)
			DrawSprite = DrawSpriteAVX;
		else
			if (ProcessorInfo.SSEAvailable)
				DrawSprite = DrawSpriteSSE;
			else
				DrawSprite = DrawSpriteX64;

		// Pulisce lo schermo
		FrameBufferPointer = WorkArea->FrameBuffer;
		VideoMemoryPointer = WorkArea->VideoMemory;
		MazeBufferPointer = FrameBufferPointer + 1024 * 768 * 4;
		CharSet = WorkArea->CharSet;
		SpriteSet = WorkArea->SpriteSet;
		TransparentMaskSet = WorkArea->TransparentMaskSet;
		TileSet = WorkArea->TileSet;
		ClearFrameBufferX64(VideoMemoryPointer);
		ClearFrameBufferX64(FrameBufferPointer);

		Running = true;
		// Ciclo di esecuzione dei comandi nella coda
		while (Running)
		{
			if (!(Command = PopCommand(WorkArea)))
			{
				_mm_pause();
				continue;
			}
			if (Command->CommandCode == cmNop)
			{
				_mm_pause();
			}
			if (Command->CommandCode == cmClearScreen)
			{
				ClearFrameBufferX64(FrameBufferPointer);
			}
			if (Command->CommandCode == cmDrawMaze)
			{
				CopyFrameBufferX64(FrameBufferPointer, MazeBufferPointer);
			}
			if (Command->CommandCode == cmDrawChar)
			{
				X = Command->Arg0;
				Y = Command->Arg1;
				Index = Command->Arg2;
				X = ((X << 1) + X) << 3;
				Y = ((Y << 1) + Y) << 3;
				DrawTileX64(FrameBufferPointer + (Y << 12) + (X << 2) + OffsetTile, CharSet + Index * sizeof(TChar));
			}
			if (Command->CommandCode == cmDrawSprite)
			{
				X = Command->Arg0;
				Y = Command->Arg1;
				X = (X << 1) + X;
				Y = (Y << 1) + Y;
				Index = Command->Arg2;
				DrawSprite(FrameBufferPointer + (Y << 12) + (X << 2) - OffsetSprite, SpriteSet + Index * sizeof(TSprite), TransparentMaskSet + Index * sizeof(TSprite));
			}
			if (Command->CommandCode == cmDrawTile)
			{
				X = Command->Arg0;
				Y = Command->Arg1;
				Index = Command->Arg2;
				X = ((X << 1) + X) << 3;
				Y = ((Y << 1) + Y) << 3;
				DrawTileX64(FrameBufferPointer + (Y << 12) + (X << 2), TileSet + Index * sizeof(TTile));
			}
			if (Command->CommandCode == cmRenderFrame)
			{
				WaitVerticalRetraceX64();
				CopyFrameBufferX64(VideoMemoryPointer, FrameBufferPointer);
			}
			if (Command->CommandCode == cmShutdown)
				Running = false;
		}
		while (true)
		{
			__halt(); // Ferma la CPU in uno stato di attesa a bassissimo consumo
			_mm_pause();
		}
	}
}

// Entry Point del modulo 64 bit
extern "C" void Dos64Main(const RamRunner::Processor64::PWorkArea WorkArea)
{
	RamRunner::Processor64::Main(WorkArea);
}
