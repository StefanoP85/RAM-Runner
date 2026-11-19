/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Core32.h
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Main source for the 32 bit Core 0
 * ============================================================================
 */
#pragma once
#include "Proc.h"

namespace RamRunner::Core32
{
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

#pragma pack(push, 1)
	struct TWorkArea
	{
		uint32_t CharSet;
		uint32_t CharSetHigh;
		uint32_t SpriteSet;
		uint32_t SpriteSetHigh;
		uint32_t TransparentMaskSet;
		uint32_t TransparentMaskSetHigh;
		uint32_t TileSet;
		uint32_t TileSetHigh;
		uint32_t VideoMemory;
		uint32_t VideoMemoryHigh;
		uint32_t FrameBuffer;
		uint32_t FrameBufferHigh;
		uint32_t CommandQueue;
		uint32_t CommandQueueHigh;
		uint32_t CommandSize;
		uint32_t CommandSizeHigh;
		uint32_t CommandHead;
		uint32_t CommandHeadHigh;
		uint32_t CommandTail;
		uint32_t CommandTailHigh;
	};
#pragma pack(pop)
	using PWorkArea = TWorkArea*;

	bool HasCrashed();
	bool HasStarted();
	bool LoadAndStartCore1();
	void StopAndUnloadCore1();
	void ClearCommandQueue();
	void PushCommand(uint32_t CommandCode, uint32_t Arg0, uint32_t Arg1, uint32_t Arg2);
}
