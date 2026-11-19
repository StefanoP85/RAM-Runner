/*
 * ============================================================================
 * Project: RAM-Runner (32-bit DOS Engine)
 * File:    Video.h
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Video and asset handling
 * ============================================================================
 */
#pragma once
#include "Proc.h"

namespace RamRunner::Video
{
	constexpr uint32_t MaxChar = 40;
	constexpr uint32_t MaxSprite = 86;
	constexpr uint32_t MaxTile = 13;
	using TChar = uint8_t[24 * 24 * 4];
	using TTile = uint8_t[24 * 24 * 4];
	using TSprite = uint8_t[64 * 64 * 4];
	using TTextScreen = uint8_t[364 * 306 * 4];
	using THelpScreen = uint8_t[1024 * 576 * 4];
	using TFullScreen = uint8_t[1024 * 768 * 4];
	using PChar = TChar*;
	using PTile = TTile*;
	using PSprite = TSprite*;
	using PTextScreen = TTextScreen*;
	using PHelpScreen = THelpScreen*;
	using PFullScreen = TFullScreen*;
	using TCharSet = TTile[MaxChar];
	using TSpriteSet = TSprite[MaxSprite];
	using TTileSet = TTile[MaxTile];
	using PCharSet = TCharSet*;
	using PSpriteSet = TSpriteSet*;
	using PTileSet = TTileSet*;
#pragma pack(push, 1)
	struct TFileEntry
	{
		char     Name[24];   // Nome della risorsa senza estensione
		uint32_t Offset;     // Posizione di inizio nel file .DAT
		uint32_t Size;       // Dimensione in byte del blocco pixel
	};
	struct TArchiveHeader
	{
		char     Magic[12];   // "RamRunnerPK\0"
		uint32_t FileCount;
	};
#pragma pack(pop)
	using PFileEntry = TFileEntry*;
	using PArchiveHeader = TArchiveHeader*;
	constexpr uint16_t MaxArchiveFileCount = 256;

	extern "C"
	{
		int32_t Clamp(int32_t Value, int32_t Min, int32_t Max);
		void ClearFrameBufferX32(uint32_t Destination);
		void CopyFrameBufferX32(uint32_t Destination, uint32_t Source);
		void DrawSpriteX32(uint32_t Destination, uint32_t Sprite, uint32_t TransparentMask);
		void DrawTileX32(uint32_t Destination, uint32_t Source);
		void WaitVerticalRetraceX32();
	}
	bool VbeExists();
	bool SetMode();
	bool AllocateFrameBuffer();
	void DeallocateFrameBuffer();
	bool EnterGraphicMode();
	void ExitGraphicMode();
	void ClearFrameBuffer();
	void CopyFrameBuffer();
	void DrawChar(uint32_t X, uint32_t Y, uint32_t CharIndex);
	void DrawSprite(uint32_t X, uint32_t Y, uint32_t SpriteIndex);
	void DrawTile(uint32_t X, uint32_t Y, uint32_t TileIndex);
	void DrawChar(uint32_t X, uint32_t Y, char Char);
	void DrawString(uint32_t X, uint32_t Y, const char* AString);
	void DrawLine(uint32_t X0, uint32_t Y0, uint32_t X1, uint32_t Y1, uint32_t Color);
	void RenderFrame();
	void DrawMenu();
	bool LoadAssets();
	void UnloadAssets();
	bool LoadGraphic(const char* FileName, uint32_t Destination);
	void CacheMaze(const uint8_t* Maze, uint32_t SizeX, uint32_t SizeY);
	void DrawMaze();
	void DrawMenu();
	void DrawHelp0();
	void DrawHelp1();
	void DrawHelp2();
	void DrawHelp3();
	void DrawHelp4();
	void DrawHelp5();
	void DrawHelpOnHelp();
	void DrawGameSplashScreen(const char* ScreenName);
	void DrawGameTextScreen(const char* ScreenName);
}
