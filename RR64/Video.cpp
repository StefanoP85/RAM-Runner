/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Video.cpp
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Video and asset handling
 * ============================================================================
 */
#include "Console.h"
#include "Core32.h"
#include "Video.h"

namespace RamRunner::Video
{
	// Offset della finestra di gioco, 24 pixel a destra e 24 pixel in basso
	constexpr uint32_t OffsetX = 24 * 4;
	constexpr uint32_t OffsetY = 24 * 4096;
	constexpr uint32_t OffsetTile = OffsetX + OffsetY;

	// Indirizzo FISICO del LFB
	uint32_t VideoLinearFrameBuffer;

	// Indirizzo LOGICO del LFB
	uint32_t MappedVideoMemory;

	// Frame buffer allocato in RAM
	uint32_t FrameBuffer;

	// Handle del buffer allocato in RAM
	uint32_t FrameBufferHandle;

	// Indirizzo LOGICO del buffer del labirinto
	uint32_t MazeBuffer;

	// Indirizzo LOGICO del buffer per grafica
	uint32_t GraphicBuffer;

	// Indirizzo LOGICO degli asset in memoria
	uint32_t AssetBuffer;

	// Modo grafico
	uint16_t GraphicMode;

	// Tabella dei file nell'archivio RR.DAT
	TFileEntry FileEntries[MaxArchiveFileCount];

	// Numero dei file nell'archivio RR.DAT
	uint32_t ArchiveFileCount;

	// Handle del file RR.DAT
	uint16_t ArchiveFileHandle;

	// Buffer di lettura del file in memoria convenzionale, 32768 byte
	void* ReadBuffer;

	// Selettore del buffer di lettura in memoria convenzionale
	uint16_t ReadBufferSelector;

	// Puntatore ai pixel dei caratteri
	uint32_t CharSet;

	// Puntatore ai pixel degli sprite
	uint32_t SpriteSet;

	// Puntatore ai pixel delle maschere di trasparenza degli sprite
	uint32_t TransparentMaskSet;

	// Puntatore ai pixel delle tile
	uint32_t TileSet;

	bool VbeExists()
	{
		uint16_t Selector = 0;
		uint8_t* Buffer = nullptr;
		bool Result = false;

		if (!Processor::DpmiAllocateConventional(768, &Selector, (void**)&Buffer))
			return false;

		uint32_t BufferLinear = reinterpret_cast<uint32_t>(Buffer);
		uint32_t DosSeg = (BufferLinear >> 4);
		uint8_t* VbeInfo = Buffer;
		uint8_t* VbeModeInfo = VbeInfo + 512; // Inizia dopo 512 byte
		VbeInfo[0] = 'V';
		VbeInfo[1] = 'B';
		VbeInfo[2] = 'E';
		VbeInfo[3] = '2';
		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x4F00;
		Regs.DS = (uint16_t)DosSeg;
		Regs.ES = (uint16_t)DosSeg;
		Regs.EDI = 0;
		if (!Processor::DpmiSimulateRealModeInterrupt(0x10, &Regs) || (Regs.EAX & 0xFFFF) != 0x004F)
		{
			Processor::DpmiDeallocateConventional(Selector);
			return false;
		}

		uint16_t ModeOff = *reinterpret_cast<uint16_t*>(&VbeInfo[14]);
		uint16_t ModeSeg = *reinterpret_cast<uint16_t*>(&VbeInfo[16]);
		uint16_t* ModeList = reinterpret_cast<uint16_t*>((ModeSeg << 4) + ModeOff);
		for (int i = 0; ModeList[i] != 0xFFFF; i++)
		{
			uint16_t CurrentMode = ModeList[i];
			Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
			Regs.EAX = 0x4F01;
			Regs.EBX = 0;
			Regs.ECX = CurrentMode;
			Regs.EDX = 0;
			Regs.DS = (uint16_t)DosSeg;
			Regs.ES = (uint16_t)DosSeg;
			Regs.EDI = 512; // Scrive le informazioni sulla modalità nel secondo blocco (offset 512)
			if (!Processor::DpmiSimulateRealModeInterrupt(0x10, &Regs) || (Regs.EAX & 0xFFFF) != 0x004F)
				continue;
			uint16_t Attributes = *reinterpret_cast<uint16_t*>(&VbeModeInfo[0]);
			uint16_t Width = *reinterpret_cast<uint16_t*>(&VbeModeInfo[18]);
			uint16_t Height = *reinterpret_cast<uint16_t*>(&VbeModeInfo[20]);
			uint8_t Bpp = VbeModeInfo[25];
			uint32_t LFBAddress = *reinterpret_cast<uint32_t*>(&VbeModeInfo[40]);
			if ((Width == 1024) && (Height == 768) && (Bpp == 32) && (Attributes & 0x0080))
			{
				VideoLinearFrameBuffer = LFBAddress;
				GraphicMode = CurrentMode
					| 0x4000  // 0x4000 abilita il Linear Frame Buffer (LFB)
					| 0x8000; // 0x8000 conserva la VRAM;
				Result = true;
				break;
			}
		}
		Processor::DpmiDeallocateConventional(Selector);
		return Result;
	}

	bool SetMode(uint16_t GraphicMode)
	{
		Processor::TDpmiRequestRegs Regs;
		Processor::MemZero(&Regs, sizeof(Processor::TDpmiRequestRegs));
		Regs.EAX = 0x4F02;
		Regs.EBX = GraphicMode;
		return (Processor::DpmiSimulateRealModeInterrupt(0x10, &Regs)) && ((Regs.EAX & 0xFFFF) == 0x004F);
	}

	bool AllocateFrameBuffer()
	{
		// Mappa la memoria del LFB in RAM
		if (!Processor::DpmiPhysicalAddressMapping((void*)VideoLinearFrameBuffer, 1024 * 768 * 4, (void**)&MappedVideoMemory))
			return false;

		// Ottiene l'indirizzo del blocco XMS allineato a 64 byte
		FrameBuffer = Processor::GetXMSAddress();
		MazeBuffer = FrameBuffer + sizeof(TFullScreen);
		GraphicBuffer = MazeBuffer + sizeof(TFullScreen);
		AssetBuffer = GraphicBuffer + sizeof(TTextScreen);
		return true;
	}

	void DeallocateFrameBuffer()
	{
		// Libera la mappatura del LFB in RAM
		Processor::DpmiFreePhysicalAddress((void*)MappedVideoMemory);
	}

	bool EnterGraphicMode()
	{
		return SetMode(GraphicMode);
	}

	void ExitGraphicMode()
	{
		Console::TextMode80x25();
	}

	void ClearFrameBuffer()
	{
		//ClearFrameBufferX32(FrameBuffer);
		Core32::PushCommand(Core32::cmClearScreen, 0, 0, 0);
	}

	void CopyFrameBuffer()
	{
		//CopyFrameBufferX32(MappedVideoMemory, FrameBuffer);
		Core32::PushCommand(Core32::cmRenderFrame, 0, 0, 0);
	}

	void RenderFrame()
	{
		//WaitVerticalRetraceX32();
		//CopyFrameBuffer();
		Core32::PushCommand(Core32::cmRenderFrame, 0, 0, 0);
	}

	void DrawSprite(uint32_t X, uint32_t Y, uint32_t SpriteIndex)
	{
		Core32::PushCommand(Core32::cmDrawSprite, X, Y, SpriteIndex);
	}

	void DrawTile(uint32_t X, uint32_t Y, uint32_t TileIndex)
	{
		Core32::PushCommand(Core32::cmDrawTile, X, Y, TileIndex);
	}

	void DrawChar(uint32_t X, uint32_t Y, char Char)
	{
		const char* CharactersTiles = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!$>";
		int16_t Pos;
		int16_t CharIndex;

		CharIndex = 0;
		for (Pos = 0; Pos < MaxChar; Pos++)
			if (Char == CharactersTiles[Pos])
			{
				CharIndex = Pos;
				break;
			}
		Core32::PushCommand(Core32::cmDrawChar, X, Y, CharIndex);
	}

	void DrawString(uint32_t X, uint32_t Y, const char* AString)
	{
		while (*AString)
		{
			DrawChar(X, Y, *AString);
			X++;
			AString++;
		}
	}

	void DrawFullScreen(const char* ScreenName)
	{
		LoadGraphic(ScreenName, FrameBuffer);
	}

	void DrawHelpScreen(const char* ScreenName)
	{
		LoadGraphic(ScreenName, FrameBuffer + 96 * 4096);
	}

	void DrawTextScreen(const char* ScreenName)
	{
		uint32_t Destination;
		uint32_t Source;

		LoadGraphic(ScreenName, GraphicBuffer);
		Source = GraphicBuffer;
		Destination = FrameBuffer + 212 * 4096 + 330 * 4;
		for (int i = 0; i < 306; i++)
		{
			Processor::MemCopy((void*)Destination, (void*)Source, 364 * 4);
			Source += 364 * 4;
			Destination += 4096;
		}
	}

	void DrawLine(uint32_t X0, uint32_t Y0, uint32_t X1, uint32_t Y1, uint32_t Color)
	{
		uint32_t* Pointer;
		int32_t dX, dY, StepX, StepY;
		int32_t Error, Inc1, Inc2;

		Pointer = (uint32_t*)FrameBuffer + (Y0 << 10) + X0; // Non serve moltiplicare per 4
		if (X1 > X0)
		{
			dX = X1 - X0;
			StepX = 1;
		}
		else
		{
			dX = X0 - X1;
			StepX = -1;
		}
		if (Y1 > Y0)
		{
			dY = Y1 - Y0;
			StepY = 1024;
		}
		else
		{
			dY = Y0 - Y1;
			StepY = -1024;
		}
		*Pointer = Color;
		if (dX >= dY)
		{
			if (dX == 0)
				return;
			Error = (dY << 1) - dX;
			Inc1 = dY << 1;        // 2 * dY
			Inc2 = (dY - dX) << 1; // 2 * (dY - dX)
			for (int i = 0; i < dX; ++i)
			{
				Pointer += StepX;
				if (Error >= 0)
				{
					Pointer += StepY;
					Error += Inc2;
				}
				else
				{
					Error += Inc1;
				}
				*Pointer = Color;
			}
		}
		else
		{
			Error = (dX << 1) - dY;
			Inc1 = dX << 1;        // 2 * dX
			Inc2 = (dX - dY) << 1; // 2 * (dX - dY)
			for (int i = 0; i < dY; ++i)
			{
				Pointer += StepY;
				if (Error >= 0)
				{
					Pointer += StepX;
					Error += Inc2;
				}
				else
				{
					Error += Inc1;
				}
				*Pointer = Color;
			}
		}
	}

	void CacheMaze(const uint8_t* Maze, uint32_t SizeX, uint32_t SizeY)
	{
		uint32_t Destination;
		uint32_t Source;
		uint32_t MazeSize;
		uint8_t TileIndex;

		MazeSize = SizeX * SizeY;
		ClearFrameBufferX32(MazeBuffer);
		for (uint32_t i = 0; i < MazeSize; i++)
		{
			uint32_t X = i % 28;
			uint32_t Y = i / 28;
			TileIndex = *Maze;
			Maze++;
			Destination = MazeBuffer + (Y * 24 * 4096) + (X * 24 * 4) + OffsetTile;
			Source = TileSet + TileIndex * sizeof(TTile);
			DrawTileX32(Destination, Source);
		}
		DrawString(30, 34, "INSPECTING");
		DrawString(30, 35, "DEVICE");
		DrawString(30, 38, "RECOVERED");
		DrawString(30, 39, "BYTES");
		DrawString(30, 42, "REMAINING");
		DrawString(30, 43, "BACKUPS");
	}

	void DrawMaze()
	{
		Core32::PushCommand(Core32::cmDrawMaze, 0, 0, 0);
	}

	void DrawMenu()
	{
		DrawFullScreen("Splash0");
	}

	void DrawHelpOnHelp()
	{
		DrawFullScreen("Splash0");
		DrawTextScreen("Text0");
	}

	void DrawHelp0()
	{
		DrawFullScreen("Splash0");
		DrawHelpScreen("Help0");
	}

	void DrawHelp1()
	{
		DrawFullScreen("Splash0");
		DrawHelpScreen("Help1");
	}

	void DrawHelp2()
	{
		DrawFullScreen("Splash0");
		DrawHelpScreen("Help2");
	}

	void DrawHelp3()
	{
		DrawFullScreen("Splash0");
		DrawHelpScreen("Help3");
	}

	void DrawHelp4()
	{
		DrawFullScreen("Splash0");
		DrawHelpScreen("Help4");
	}

	void DrawHelp5()
	{
		DrawFullScreen("Splash0");
		DrawHelpScreen("Help5");
	}

	void DrawGameSplashScreen(const char* ScreenName)
	{
		DrawFullScreen(ScreenName);
	}

	void DrawGameTextScreen(const char* ScreenName)
	{
		DrawFullScreen("Splash0");
		DrawTextScreen(ScreenName);
	}

	int32_t GetFileEntryIndex(const char* FileName)
	{
		int32_t First;
		int32_t Last;
		int32_t Pivot;
		int32_t Comparison;

		/* // Ricerca sequenziale
		for (int i = 0; i < MaxArchiveFileCount; i++)
		{
			if (Processor::StrComp(FileEntries[i].Name, FileName) == 0)
			{
				return i;
			}
		}
		return -1; */
		// Ricerca dicotomica
		First = 0;
		Last = ArchiveFileCount - 1;
		while (First <= Last)
		{
			Pivot = (First + Last) >> 1;
			Comparison = Processor::StrComp(FileEntries[Pivot].Name, FileName);
			if (Comparison == 0)
				return Pivot;
			if (Comparison < 0)
				First = Pivot + 1;
			else
				Last = Pivot - 1;
		}
		return -1;
	}

	bool OpenArchive()
	{
		PArchiveHeader ArchiveHeader;
		PFileEntry FileEntry;

		if (!Processor::DpmiAllocateConventional(32768, &ReadBufferSelector, &ReadBuffer))
		{
			return false;
		}

		ArchiveFileHandle = Processor::DosFileOpen("RR.DAT");
		if (ArchiveFileHandle == 0)
		{
			Processor::DpmiDeallocateConventional(ReadBufferSelector);
			return false;
		}
		if (Processor::DosFileRead(ArchiveFileHandle, ReadBuffer, sizeof(TArchiveHeader)) != sizeof(TArchiveHeader))
		{
			Processor::DosFileClose(ArchiveFileHandle);
			Processor::DpmiDeallocateConventional(ReadBufferSelector);
			return false;
		}
		ArchiveHeader = reinterpret_cast<PArchiveHeader>(ReadBuffer);
		if (Processor::StrComp(ArchiveHeader->Magic, "RamRunnerPK") != 0)
		{
			Processor::DosFileClose(ArchiveFileHandle);
			Processor::DpmiDeallocateConventional(ReadBufferSelector);
			return false;
		}
		ArchiveFileCount = Clamp(ArchiveHeader->FileCount, 0, MaxArchiveFileCount);
		FileEntry = &FileEntries[0];
		for (uint32_t i = 0; i < ArchiveFileCount; i++)
		{
			if (Processor::DosFileRead(ArchiveFileHandle, ReadBuffer, sizeof(TFileEntry)) != sizeof(TFileEntry))
			{
				Processor::DosFileClose(ArchiveFileHandle);
				Processor::DpmiDeallocateConventional(ReadBufferSelector);
				return false;
			}
			Processor::MemCopy(FileEntry, ReadBuffer, sizeof(TFileEntry));
			FileEntry++;
		}
		// Non chiudere il file handle!
		//FileClose(ArchiveFileHandle);
		//Processor::DpmiDeallocateConventional(ReadBufferSelector);
		return true;
	}

	bool ReadArchive(const char* FileName, uint32_t Destination, uint32_t* FileSize)
	{
		int32_t FileEntryIndex;
		uint16_t BytesReading;
		int32_t BytesRemaining;
		uint8_t* Dest;

		FileEntryIndex = GetFileEntryIndex(FileName);
		if (FileEntryIndex < 0)
			return false;

		if (!Processor::DosFileSeek(ArchiveFileHandle, FileEntries[FileEntryIndex].Offset))
		{
			return false;
		}
		*FileSize = FileEntries[FileEntryIndex].Size;
		BytesRemaining = FileEntries[FileEntryIndex].Size;
		Dest = reinterpret_cast<uint8_t*>((void*)Destination);
		while (BytesRemaining > 0)
		{
			if (BytesRemaining > 32768)
				BytesReading = 32768;
			else
				BytesReading = (uint16_t)BytesRemaining;
			if (Processor::DosFileRead(ArchiveFileHandle, ReadBuffer, BytesReading) != BytesReading)
				return false;
			Processor::MemCopy(Dest, ReadBuffer, BytesReading);
			Dest += BytesReading;
			BytesRemaining -= BytesReading;
		}
		return true;
	}

	void CloseArchive()
	{
		Processor::DpmiDeallocateConventional(ReadBufferSelector);
		Processor::DosFileClose(ArchiveFileHandle);
	}

	bool LoadGraphic(const char* FileName, uint32_t Destination)
	{
		uint32_t Size;
		return ReadArchive(FileName, Destination, &Size);
	}

	bool LoadAsset(const char* FileName, uint32_t* Destination)
	{
		uint32_t Dest;
		uint32_t Size;

		Dest = *Destination;
		if (ReadArchive(FileName, Dest, &Size))
		{
			Dest += Size;
			*Destination = Dest;
			return true;
		}
		else
		{
			Console::DrawString(40, 15, FileName, 7);
			return false;
		}
	}

	bool LoadAssetList(const char* ListName, uint32_t* Destination)
	{
		char Buffer[2024];
		char LumpName[24];
		uint32_t LumpSize;
		uint32_t LumpNameLength;

		if (!ReadArchive(ListName, (uint32_t)&Buffer, &LumpSize))
			return false;

		LumpNameLength = 0;
		LumpName[0] = '\0';

		for (uint32_t i = 0; i < LumpSize; i++)
		{
			char CurrentChar = Buffer[i];
			if (CurrentChar == '\n')
			{
				if (LumpNameLength > 0)
				{
					if (LumpNameLength < sizeof(LumpName))
						LumpName[LumpNameLength] = '\0';
					else
						LumpName[sizeof(LumpName) - 1] = '\0';

					if (!LoadAsset(LumpName, Destination))
						return false;
				}
				LumpNameLength = 0;
				LumpName[0] = '\0';
			}
			else
			{
				if (CurrentChar >= 32) // Ignora caratteri di controllo come \r (ASCII 13)
				{
					if (LumpNameLength < sizeof(LumpName) - 1)
					{
						LumpName[LumpNameLength] = CurrentChar;
						LumpNameLength++;
					}
				}
			}
		}

		// Gestione dell'eventuale ultima riga senza newline finale
		if (LumpNameLength > 0)
		{
			LumpName[LumpNameLength < sizeof(LumpName) ? LumpNameLength : sizeof(LumpName) - 1] = '\0';
			if (!LoadAsset(LumpName, Destination))
				return false;
		}

		return true;
	}

	bool LoadAssets()
	{
		uint32_t Destination;
		uint32_t* Sprite;
		uint32_t* TransparentMask;

		if (!OpenArchive())
		{
			return false;
		}

		Destination = AssetBuffer;
		Console::DrawString(0, 6, "Character set...", Console::Red);
		CharSet = Destination;
		if (!LoadAssetList("CharSet", &Destination))
		{
			return false;
		}
		Console::DrawString(0, 7, "Tile set...", Console::Green);
		TileSet = Destination;
		if (!LoadAssetList("TileSet", &Destination))
		{
			return false;
		}
		Console::DrawString(0, 8, "Sprite set...", Console::Blue);
		SpriteSet = Destination;
		if (!LoadAssetList("SpriteSet", &Destination))
		{
			return false;
		}
		Console::DrawString(0, 9, "Mask set...", Console::Yellow);
		TransparentMaskSet = Destination;
		Sprite = reinterpret_cast<uint32_t*>((void*)SpriteSet);
		TransparentMask = reinterpret_cast<uint32_t*>((void*)TransparentMaskSet);
		for (int i = (sizeof(TSpriteSet) >> 2); i > 0; i--)
		{
			if (*Sprite)
				*TransparentMask = 0xFFFFFFFF;
			else
				*TransparentMask = 0x00000000;
			Sprite++;
			TransparentMask++;
		}
		return true;
	}

	void UnloadAssets()
	{
		CloseArchive();
	}
}
