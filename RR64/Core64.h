/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Core64.h
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Main source for the 64 bit Core 1
 * ============================================================================
 */
#pragma once

typedef signed char            int8_t;
typedef signed short int       int16_t;
typedef signed long int        int32_t;
typedef signed long long int   int64_t;
typedef unsigned char          uint8_t;
typedef unsigned short         uint16_t;
typedef unsigned long int      uint32_t;
typedef unsigned long long int uint64_t;

namespace RamRunner::Processor64
{
	struct TRegs
	{
		uint32_t EAX;
		uint32_t EBX;
		uint32_t ECX;
		uint32_t EDX;
	};
	enum TCacheType : uint32_t { Unknown, Data, Code, Unified };
	enum TFeatureFlags : uint64_t
	{
		FXSR = 1ULL << 0,
		MMX = 1ULL << 1,
		MOVBE = 1ULL << 2,
		SSE = 1ULL << 3,
		SSE2 = 1ULL << 4,
		SSE3 = 1ULL << 5,
		SSSE3 = 1ULL << 6,
		SSE4_1 = 1ULL << 7,
		SSE4_2 = 1ULL << 8,
		PCLMULQDQ = 1ULL << 9,
		POPCNT = 1ULL << 10,
		PREFETCHW = 1ULL << 11,
		PREFETCHWT1 = 1ULL << 12,
		RDRAND = 1ULL << 13,
		RDSEED = 1ULL << 14,
		ERMSB = 1ULL << 15,
		AVX = 1ULL << 16,
		AVX2 = 1ULL << 17,
		F16C = 1ULL << 18,
		FMA = 1ULL << 19,
		BMI1 = 1ULL << 20,
		BMI2 = 1ULL << 21,
		LZCNT = 1ULL << 22,
		ADX = 1ULL << 23,
		AVX512F = 1ULL << 24,
		AVX512ER = 1ULL << 25,
		AVX512PF = 1ULL << 26,
		AVX512DQ = 1ULL << 27,
		AVX512CD = 1ULL << 28,
		AVX512BW = 1ULL << 29,
		AVX512VL = 1ULL << 30,
		AVX512_IFMA = 1ULL << 31,
		AVX512_VBMI = 1ULL << 32,
		AVX512_4FMAPS = 1ULL << 33,
		AVX512_4VNNIW = 1ULL << 34,
		AVX512_VPOPCNTDQ = 1ULL << 35,
		AVX512_VNNI = 1ULL << 36,
		AVX512_VBMI2 = 1ULL << 37,
		AVX512_BITALG = 1ULL << 38,
		CLWB = 1ULL << 39
	};
	struct TProcessorInfo
	{
		uint32_t MaxEax;
		uint32_t MaxEaxExt;
		uint64_t FeatureFlags;
		char VendorId[13];
		char ProcessorBrand[49];
		bool Xsave;
		bool OsXsave;
		bool OsAvxState;
		bool OsAvx512State;
		bool SSEAvailable;
		bool AVXAvailable;
		bool AVX512Available;
	};
	using PProcessorInfo = TProcessorInfo*;
#pragma pack(push, 1)
	struct TWorkArea
	{
		uint64_t CharSet;
		uint64_t SpriteSet;
		uint64_t TransparentMaskSet;
		uint64_t TileSet;
		uint64_t VideoMemory;
		uint64_t FrameBuffer;
		uint64_t CommandQueue;
		uint64_t CommandSize;
		uint64_t CommandHead;
		uint64_t CommandTail;
	};
#pragma pack(pop)
	using PWorkArea = TWorkArea*;

	// Gestione dei comandi
	constexpr uint32_t MaxCommand = 1024;
	constexpr uint32_t cmNop = 0;
	constexpr uint32_t cmClearScreen = 1;
	constexpr uint32_t cmDrawMaze = 2;
	constexpr uint32_t cmDrawChar = 3;
	constexpr uint32_t cmDrawSprite = 4;
	constexpr uint32_t cmDrawTile = 5;
	constexpr uint32_t cmRenderFrame = 6;
	constexpr uint32_t cmShutdown = 255;
#pragma pack(push, 1)
	struct TCommand
	{
		uint32_t CommandCode;
		uint32_t Arg0;
		uint32_t Arg1;
		uint32_t Arg2;
	};
#pragma pack(pop)
	using PCommand = TCommand*;
	using TCommandQueue = TCommand[MaxCommand];
	constexpr uint32_t MaxChar = 40;
	constexpr uint32_t MaxSprite = 86;
	constexpr uint32_t MaxTile = 13;
	using TChar = uint8_t[24 * 24 * 4];
	using TTile = uint8_t[24 * 24 * 4];
	using TSprite = uint8_t[64 * 64 * 4];
	using PChar = TChar*;
	using PTile = TTile*;
	using PSprite = TSprite*;
	using TCharSet = TTile[MaxChar];
	using TSpriteSet = TSprite[MaxSprite];
	using TTileSet = TTile[MaxTile];
	using TDrawSprite = void(*)(uint64_t Destination, uint64_t Sprite, uint64_t TransparentMask);

	extern "C"
	{
		void CallCPUID64(uint32_t EAX, uint32_t ECX, TRegs* Regs);
		uint64_t CallRDTSCP();
		void CallXGETBV64(uint32_t ECX, uint32_t* EAX, uint32_t* EDX);
		void CallXSETBV64(uint32_t RegECX, uint32_t RegEAX, uint32_t RegEDX);
		int64_t Clamp(int64_t Value, int64_t Min, int64_t Max);
		void ClearFrameBufferX64(uint64_t Destination);
		void CopyFrameBufferX64(uint64_t Destination, uint64_t Source);
		void DrawSpriteX64(uint64_t Destination, uint64_t Sprite, uint64_t TransparentMask);
		void DrawSpriteSSE(uint64_t Destination, uint64_t Sprite, uint64_t TransparentMask);
		void DrawSpriteAVX(uint64_t Destination, uint64_t Sprite, uint64_t TransparentMask);
		void DrawTileX64(uint64_t Destination, uint64_t Source);
		void WaitVerticalRetraceX64();
	}
}
