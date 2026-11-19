/*
 * ============================================================================
 * Project: RAM-Runner (32-bit DOS Engine)
 * File:    RR.cpp
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Main source for the game logic
 * ============================================================================
 */
#include <intrin.h>
#include "AdLib.h"
#include "Audio.h"
#include "Console.h"
#include "RR.h"
#include "Proc.h"
#include "Speaker.h"
#include "Timer.h"
#include "Video.h"

namespace RamRunner
{
	constexpr uint8_t PlayerCanMoveUp = 0x01;
	constexpr uint8_t PlayerCanMoveLeft = 0x02;
	constexpr uint8_t PlayerCanMoveDown = 0x04;
	constexpr uint8_t PlayerCanMoveRight = 0x08;
	constexpr uint8_t HackerCanMoveUp = 0x10;
	constexpr uint8_t HackerCanMoveLeft = 0x20;
	constexpr uint8_t HackerCanMoveDown = 0x40;
	constexpr uint8_t HackerCanMoveRight = 0x80;
	const uint8_t MazeTiles[MazeSize] = {
		 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		 4,  0,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  4,  0,  0,  4,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  0,  4,
		 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,
		 4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,
		 4,  0, 11, 11, 11, 11,  0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  6,  0,  0,  0,  0,  0, 11, 11, 11, 11,  0,  4,
		 4,  4, 11, 11, 11, 11,  4,  4,  4,  4,  4,  6,  6,  6,  6,  6,  6,  4,  4,  4,  4,  4, 11, 11, 11, 11,  4,  4,
		 4,  0, 11, 11, 11, 11,  0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  6,  0,  0,  0,  0,  0, 11, 11, 11, 11,  0,  4,
		 4,  0, 11, 11, 11, 11,  0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  6,  0,  0,  0,  0,  0, 11, 11, 11, 11,  0,  4,
		 4,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  6,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  4,
		 4,  0,  0,  0,  0,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  4,
		 4,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  4,  0,  0,  4,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  4,
		 4,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  4,  0,  0,  4,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  4,
		 4,  0,  0,  0,  0,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  4,
		 4,  0,  0,  0,  0,  4,  0,  0,  4,  0,  0,  7,  7,  7,  7,  7,  7,  0,  0,  4,  0,  0,  4,  0,  0,  0,  0,  4,
		 4,  0,  0,  2,  2,  2,  2,  0,  4,  0,  0,  7,  7,  7,  7,  7,  7,  0,  0,  4,  0,  9,  9,  9,  9,  0,  0,  4,
		 4,  4,  4,  2,  2,  2,  2,  0,  4,  0,  0,  7,  7,  7,  7,  7,  7,  0,  0,  4,  0,  9,  9,  9,  9,  0,  0,  4,
		 4,  0,  0,  2,  2,  2,  2,  0,  4,  0,  0,  7,  7,  7,  7,  7,  7,  0,  0,  4,  4,  9,  9,  9,  9,  4,  4,  4,
		 4,  0,  0,  2,  2,  2,  2,  0,  4,  0,  0,  7,  7,  7,  7,  7,  7,  0,  0,  4,  0,  9,  9,  9,  9,  0,  0,  4,
		 4,  0,  0,  0,  0,  4,  0,  0,  4,  0,  0,  7,  7,  7,  7,  7,  7,  0,  0,  4,  0,  9,  9,  9,  9,  0,  0,  4,
		 4,  0, 10, 10, 10, 10,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  4,
		 4,  0, 10, 10, 10, 10,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  3,  3,  3,  0,  0,  4,
		 4,  4, 10, 10, 10, 10,  0,  0,  4,  0,  0,  0,  4,  0,  0,  4,  0,  0,  0,  4,  0,  0,  3,  3,  3,  4,  4,  4,
		 4,  0, 10, 10, 10, 10,  0,  0,  4,  0,  0,  0,  4,  0,  0,  4,  0,  0,  0,  4,  0,  0,  3,  3,  3,  0,  0,  4,
		 4,  0,  0,  0,  0,  4,  0,  0,  4,  0,  0,  8,  8,  8,  8,  8,  8,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  4,
		 4,  0,  0,  0,  5,  5,  5,  5,  5,  0,  0,  8,  8,  8,  8,  8,  8,  0,  0,  4,  0, 12, 12, 12, 12, 12,  0,  4,
		 4,  0,  0,  0,  5,  5,  5,  5,  5,  0,  0,  8,  8,  8,  8,  8,  8,  0,  0,  4,  4, 12, 12, 12, 12, 12,  4,  4,
		 4,  0,  0,  0,  5,  5,  5,  5,  5,  0,  0,  8,  8,  8,  8,  8,  8,  0,  0,  4,  0, 12, 12, 12, 12, 12,  0,  4,
		 4,  0,  0,  0,  5,  5,  5,  5,  5,  0,  0,  0,  4,  0,  0,  4,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  4,
		 4,  0,  1,  1,  1,  1,  0,  0,  4,  0,  0,  0,  4,  0,  0,  4,  0,  0,  0,  4,  0,  0,  1,  1,  1,  1,  0,  4,
		 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4
	};
	const uint8_t MazeMovements[MazeSize] = {
		0xCC, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xEE, 0xAA, 0xAA, 0xEE, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x66,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC, 0xFF, 0xEE, 0xEE, 0xFF, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0xCC, 0xEE, 0xEE, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC, 0xEE, 0xEE, 0x66, 0x00, 0x55,
		0xDD, 0xAA, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0x77,
		0x55, 0x00, 0xDD, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0x77, 0x00, 0x55,
		0x55, 0x00, 0x99, 0xBB, 0xBB, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xBB, 0xBB, 0x33, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xBB, 0xFF, 0xBB, 0xBB, 0xFF, 0xBB, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x77, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x9D, 0xAA, 0xAA, 0xEE, 0xAA, 0xAA, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xAA, 0xAA, 0xEE, 0xAA, 0xAA, 0x77, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0xCC, 0xEE, 0xFF, 0x66, 0x00, 0x55, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x55, 0x00, 0xCC, 0xFF, 0xEE, 0x66, 0x00, 0x00, 0x55,
		0x5D, 0xAA, 0xAA, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x55, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x55, 0x00, 0xDD, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0x77, 0x00, 0x55, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0xDD, 0xAA, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0x77,
		0x55, 0x00, 0x00, 0x99, 0xBB, 0xFF, 0x33, 0x00, 0x55, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x55, 0x00, 0xDD, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0x99, 0xBB, 0xBB, 0xBB, 0xBB, 0x33, 0x00, 0x00, 0x55, 0x00, 0x99, 0xBB, 0xBB, 0x33, 0x00, 0x00, 0x55,
		0x55, 0x00, 0xCC, 0xEE, 0xEE, 0x67, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xAA, 0xAA, 0xAA, 0xEE, 0xAA, 0xAA, 0xEE, 0xAA, 0xAA, 0xAA, 0x7F, 0xAA, 0xAA, 0xEE, 0xEE, 0x66, 0x00, 0x00, 0x55,
		0xDD, 0xAA, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xAA, 0xAA, 0x57,
		0x55, 0x00, 0x99, 0xBB, 0xBB, 0x77, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x99, 0xBB, 0x33, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0xCC, 0xFF, 0xEE, 0xEE, 0xFF, 0x66, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0xCC, 0xFF, 0xEE, 0xEE, 0x77, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x55, 0x00, 0xCC, 0xEE, 0xEE, 0xEE, 0x66, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0xDD, 0xAA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0x77,
		0x55, 0x00, 0x00, 0x00, 0xDD, 0xFF, 0xFF, 0xFF, 0x77, 0x00, 0x00, 0x99, 0xFF, 0xBB, 0xBB, 0xFF, 0x33, 0x00, 0x00, 0x55, 0x00, 0x99, 0xBB, 0xBB, 0xBB, 0x33, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x99, 0xBB, 0xBB, 0xBB, 0x77, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55,
		0x99, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xBB, 0xAA, 0xAA, 0xAA, 0xBB, 0xAA, 0xAA, 0xBB, 0xAA, 0xAA, 0xAA, 0xBB, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x33
	};
	TLevelInfo LevelInfo[MaxLevelInfo]
	{
		{
			.LevelNumber = 0,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "Text1",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = false,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 10,
			.NumFiles = 5,
			.NumFloppys = 0,
			.NumCloudFiles = 2,
			.NumJunkFragments = 0,
			.NumLockedFiles = 0,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 208,
			.SpeedHigh = 176,
			.SpeedMedium = 132,
			.SpeedLow = 88,
			.SpeedPlayerFright = 256,
			.SpeedHighFright = 132,
			.SpeedMediumFright = 96,
			.SpeedLowFright = 64,
			.FrightenedCount = 8 * 60,
			.RegionLockedCount = 8 * 60,
			.InvincibleCount = 8 * 60
		},
		{
			.LevelNumber = 1,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "Splash1",
			.EndingTextScreen = "\0",
			.SpawnHigh = false,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 10,
			.NumFiles = 5,
			.NumFloppys = 0,
			.NumCloudFiles = 2,
			.NumJunkFragments = 0,
			.NumLockedFiles = 0,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 208,
			.SpeedHigh = 176,
			.SpeedMedium = 132,
			.SpeedLow = 88,
			.SpeedPlayerFright = 260,
			.SpeedHighFright = 132,
			.SpeedMediumFright = 96,
			.SpeedLowFright = 64,
			.FrightenedCount = 8 * 60,
			.RegionLockedCount = 8 * 60,
			.InvincibleCount = 8 * 60
		},
		{
			.LevelNumber = 2,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 14,
			.NumFiles = 7,
			.NumFloppys = 0,
			.NumCloudFiles = 3,
			.NumJunkFragments = 0,
			.NumLockedFiles = 0,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 208,
			.SpeedHigh = 192,
			.SpeedMedium = 144,
			.SpeedLow = 96,
			.SpeedPlayerFright = 260,
			.SpeedHighFright = 144,
			.SpeedMediumFright = 108,
			.SpeedLowFright = 72,
			.FrightenedCount = 8 * 60,
			.RegionLockedCount = 8 * 60,
			.InvincibleCount = 8 * 60
		},
		{
			.LevelNumber = 3,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 14,
			.NumFiles = 7,
			.NumFloppys = 0,
			.NumCloudFiles = 3,
			.NumJunkFragments = 0,
			.NumLockedFiles = 0,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 208,
			.SpeedHigh = 192,
			.SpeedMedium = 144,
			.SpeedLow = 96,
			.SpeedPlayerFright = 260,
			.SpeedHighFright = 144,
			.SpeedMediumFright = 108,
			.SpeedLowFright = 72,
			.FrightenedCount = 8 * 60,
			.RegionLockedCount = 8 * 60,
			.InvincibleCount = 8 * 60
		},
		{
			.LevelNumber = 4,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "Text2",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 12,
			.NumFiles = 5,
			.NumFloppys = 0,
			.NumCloudFiles = 2,
			.NumJunkFragments = 0,
			.NumLockedFiles = 0,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 240,
			.SpeedHigh = 224,
			.SpeedMedium = 168,
			.SpeedLow = 112,
			.SpeedPlayerFright = 300,
			.SpeedHighFright = 168,
			.SpeedMediumFright = 128,
			.SpeedLowFright = 84,
			.FrightenedCount = 6 * 60,
			.RegionLockedCount = 6 * 60,
			.InvincibleCount = 6 * 60
		},
		{
			.LevelNumber = 5,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 12,
			.NumFiles = 5,
			.NumFloppys = 0,
			.NumCloudFiles = 2,
			.NumJunkFragments = 0,
			.NumLockedFiles = 0,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 240,
			.SpeedHigh = 224,
			.SpeedMedium = 168,
			.SpeedLow = 112,
			.SpeedPlayerFright = 300,
			.SpeedHighFright = 168,
			.SpeedMediumFright = 128,
			.SpeedLowFright = 84,
			.FrightenedCount = 6 * 60,
			.RegionLockedCount = 6 * 60,
			.InvincibleCount = 6 * 60
		},
		{
			.LevelNumber = 6,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 16,
			.NumFiles = 7,
			.NumFloppys = 0,
			.NumCloudFiles = 3,
			.NumJunkFragments = 0,
			.NumLockedFiles = 0,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 240,
			.SpeedHigh = 240,
			.SpeedMedium = 180,
			.SpeedLow = 112,
			.SpeedPlayerFright = 300,
			.SpeedHighFright = 180,
			.SpeedMediumFright = 144,
			.SpeedLowFright = 84,
			.FrightenedCount = 6 * 60,
			.RegionLockedCount = 6 * 60,
			.InvincibleCount = 6 * 60
		},
		{
			.LevelNumber = 7,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 16,
			.NumFiles = 7,
			.NumFloppys = 0,
			.NumCloudFiles = 3,
			.NumJunkFragments = 0,
			.NumLockedFiles = 0,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 240,
			.SpeedHigh = 240,
			.SpeedMedium = 180,
			.SpeedLow = 112,
			.SpeedPlayerFright = 300,
			.SpeedHighFright = 180,
			.SpeedMediumFright = 144,
			.SpeedLowFright = 84,
			.FrightenedCount = 6 * 60,
			.RegionLockedCount = 6 * 60,
			.InvincibleCount = 6 * 60
		},
		{
			.LevelNumber = 8,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "Text3",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = true,
			.NumFileFragments = 11,
			.NumFiles = 4,
			.NumFloppys = 2,
			.NumCloudFiles = 1,
			.NumJunkFragments = 0,
			.NumLockedFiles = 0,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 256,
			.SpeedHigh = 288,
			.SpeedMedium = 216,
			.SpeedLow = 120,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 216,
			.SpeedMediumFright = 168,
			.SpeedLowFright = 88,
			.FrightenedCount = 6 * 60,
			.RegionLockedCount = 6 * 60,
			.InvincibleCount = 6 * 60
		},
		{
			.LevelNumber = 9,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "Splash2",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = true,
			.NumFileFragments = 11,
			.NumFiles = 4,
			.NumFloppys = 2,
			.NumCloudFiles = 1,
			.NumJunkFragments = 0,
			.NumLockedFiles = 0,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 256,
			.SpeedHigh = 288,
			.SpeedMedium = 216,
			.SpeedLow = 120,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 216,
			.SpeedMediumFright = 168,
			.SpeedLowFright = 88,
			.FrightenedCount = 6 * 60,
			.RegionLockedCount = 6 * 60,
			.InvincibleCount = 6 * 60
		},
		{
			.LevelNumber = 10,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = true,
			.NumFileFragments = 14,
			.NumFiles = 6,
			.NumFloppys = 0,
			.NumCloudFiles = 7,
			.NumJunkFragments = 4,
			.NumLockedFiles = 1,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 256,
			.SpeedHigh = 288,
			.SpeedMedium = 216,
			.SpeedLow = 120,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 216,
			.SpeedMediumFright = 168,
			.SpeedLowFright = 88,
			.FrightenedCount = 6 * 60,
			.RegionLockedCount = 6 * 60,
			.InvincibleCount = 6 * 60
		},
		{
			.LevelNumber = 11,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = true,
			.NumFileFragments = 14,
			.NumFiles = 6,
			.NumFloppys = 0,
			.NumCloudFiles = 7,
			.NumJunkFragments = 4,
			.NumLockedFiles = 1,
			.NumLockedCloudFiles = 0,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 256,
			.SpeedHigh = 288,
			.SpeedMedium = 216,
			.SpeedLow = 120,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 216,
			.SpeedMediumFright = 168,
			.SpeedLowFright = 88,
			.FrightenedCount = 6 * 60,
			.RegionLockedCount = 6 * 60,
			.InvincibleCount = 6 * 60
		},
		{
			.LevelNumber = 12,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "Text4",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 9,
			.NumFiles = 3,
			.NumFloppys = 0,
			.NumCloudFiles = 5,
			.NumJunkFragments = 16,
			.NumLockedFiles = 2,
			.NumLockedCloudFiles = 1,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 256,
			.SpeedHigh = 320,
			.SpeedMedium = 240,
			.SpeedLow = 128,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 240,
			.SpeedMediumFright = 180,
			.SpeedLowFright = 96,
			.FrightenedCount = 4 * 60,
			.RegionLockedCount = 4 * 60,
			.InvincibleCount = 4 * 60
		},
		{
			.LevelNumber = 13,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "Splash3",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 9,
			.NumFiles = 3,
			.NumFloppys = 0,
			.NumCloudFiles = 5,
			.NumJunkFragments = 16,
			.NumLockedFiles = 2,
			.NumLockedCloudFiles = 1,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 256,
			.SpeedHigh = 320,
			.SpeedMedium = 240,
			.SpeedLow = 128,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 240,
			.SpeedMediumFright = 180,
			.SpeedLowFright = 96,
			.FrightenedCount = 4 * 60,
			.RegionLockedCount = 4 * 60,
			.InvincibleCount = 4 * 60
		},
		{
			.LevelNumber = 14,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 11,
			.NumFiles = 5,
			.NumFloppys = 0,
			.NumCloudFiles = 4,
			.NumJunkFragments = 12,
			.NumLockedFiles = 3,
			.NumLockedCloudFiles = 1,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 256,
			.SpeedHigh = 320,
			.SpeedMedium = 240,
			.SpeedLow = 128,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 240,
			.SpeedMediumFright = 180,
			.SpeedLowFright = 96,
			.FrightenedCount = 4 * 60,
			.RegionLockedCount = 4 * 60,
			.InvincibleCount = 4 * 60
		},
		{
			.LevelNumber = 15,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = false,
			.SpawnLow = false,
			.NumFileFragments = 11,
			.NumFiles = 5,
			.NumFloppys = 0,
			.NumCloudFiles = 4,
			.NumJunkFragments = 12,
			.NumLockedFiles = 3,
			.NumLockedCloudFiles = 1,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 256,
			.SpeedHigh = 320,
			.SpeedMedium = 240,
			.SpeedLow = 128,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 240,
			.SpeedMediumFright = 180,
			.SpeedLowFright = 96,
			.FrightenedCount = 4 * 60,
			.RegionLockedCount = 4 * 60,
			.InvincibleCount = 4 * 60
		},
		{
			.LevelNumber = 16,
			.BeginningSplashScreen = "",
			.BeginningTextScreen = "Text5",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = true,
			.SpawnLow = true,
			.NumFileFragments = 7,
			.NumFiles = 3,
			.NumFloppys = 1,
			.NumCloudFiles = 3,
			.NumJunkFragments = 4,
			.NumLockedFiles = 5,
			.NumLockedCloudFiles = 2,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 2,
			.NumSSDs = 1,
			.NumPGPKeys = 2,
			.SpeedPlayer = 256,
			.SpeedHigh = 320,
			.SpeedMedium = 256,
			.SpeedLow = 128,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 240,
			.SpeedMediumFright = 192,
			.SpeedLowFright = 96,
			.FrightenedCount = 4 * 60,
			.RegionLockedCount = 4 * 60,
			.InvincibleCount = 4 * 60
		},
		{
			.LevelNumber = 17,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "Splash4",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = true,
			.SpawnLow = true,
			.NumFileFragments = 7,
			.NumFiles = 3,
			.NumFloppys = 1,
			.NumCloudFiles = 3,
			.NumJunkFragments = 4,
			.NumLockedFiles = 5,
			.NumLockedCloudFiles = 2,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 2,
			.NumSSDs = 1,
			.NumPGPKeys = 2,
			.SpeedPlayer = 256,
			.SpeedHigh = 320,
			.SpeedMedium = 256,
			.SpeedLow = 128,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 240,
			.SpeedMediumFright = 192,
			.SpeedLowFright = 96,
			.FrightenedCount = 4 * 60,
			.RegionLockedCount = 4 * 60,
			.InvincibleCount = 4 * 60
		},
		{
			.LevelNumber = 18,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "\0",
			.EndingTextScreen = "\0",
			.SpawnHigh = true,
			.SpawnMedium = true,
			.SpawnLow = true,
			.NumFileFragments = 7,
			.NumFiles = 2,
			.NumFloppys = 1,
			.NumCloudFiles = 2,
			.NumJunkFragments = 4,
			.NumLockedFiles = 7,
			.NumLockedCloudFiles = 3,
			.NumEncryptedUSB = 0,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 256,
			.SpeedHigh = 320,
			.SpeedMedium = 256,
			.SpeedLow = 128,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 240,
			.SpeedMediumFright = 192,
			.SpeedLowFright = 96,
			.FrightenedCount = 1 * 60,
			.RegionLockedCount = 1 * 60,
			.InvincibleCount = 1 * 60
		},
		{
			.LevelNumber = 19,
			.BeginningSplashScreen = "\0",
			.BeginningTextScreen = "\0",
			.EndingSplashScreen = "Splash5",
			.EndingTextScreen = "Text6",
			.SpawnHigh = true,
			.SpawnMedium = true,
			.SpawnLow = true,
			.NumFileFragments = 7,
			.NumFiles = 2,
			.NumFloppys = 1,
			.NumCloudFiles = 2,
			.NumJunkFragments = 4,
			.NumLockedFiles = 7,
			.NumLockedCloudFiles = 3,
			.NumEncryptedUSB = 1,
			.NumTerminalShells = 3,
			.NumSmartCards = 1,
			.NumSSDs = 1,
			.NumPGPKeys = 1,
			.SpeedPlayer = 256,
			.SpeedHigh = 320,
			.SpeedMedium = 256,
			.SpeedLow = 128,
			.SpeedPlayerFright = 320,
			.SpeedHighFright = 240,
			.SpeedMediumFright = 192,
			.SpeedLowFright = 96,
			.FrightenedCount = 1 * 60,
			.RegionLockedCount = 1 * 60,
			.InvincibleCount = 1 * 60
		},
	};
	TActorClass ActorClasses[30]
	{
		{ // 0x00 Classe vuota
			.Flags = cfNone,
			.Tier = ctNone,
			.BaseSprite = 0,
			.NumFrames = 0,
			.Directional = false,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x01 FORENSIC ANNA
			.Flags = cfLiving,
			.Tier = ctNone,
			.BaseSprite = 0,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x02 FORENSIC STEFANO
			.Flags = cfLiving,
			.Tier = ctNone,
			.BaseSprite = 4,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x03 THE LOUD BASEMENT STREAMER
			.Flags = cfLiving,
			.Tier = ctLow,
			.BaseSprite = 8,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = LoudBasementStreamerThink,
			.ActorChase = nullptr
		},
		{ // 0x04 THE CASUAL “COFFEE SHOP” GAME DEV GONE WRONG
			.Flags = cfLiving,
			.Tier = ctLow,
			.BaseSprite = 12,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = CasualCoffeShopDevThink,
			.ActorChase = nullptr
		},
		{ // 0x05 THE SNAKE OIL SALESMAN
			.Flags = cfLiving,
			.Tier = ctLow,
			.BaseSprite = 16,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = SnakeOilSalesmanThink,
			.ActorChase = nullptr
		},
		{ // 0x06 THE CLUMSY DIGITAL SQUATTER
			.Flags = cfLiving,
			.Tier = ctLow,
			.BaseSprite = 20,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = ClumsyDigitalSquatterThink,
			.ActorChase = nullptr
		},
		{ // 0x07 THE SCRIPT KIDDIE
			.Flags = cfLiving,
			.Tier = ctMedium,
			.BaseSprite = 24,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = SpecialActorTouchItem,
			.ActorThink = ScriptKiddieThink,
			.ActorChase = ScriptKiddieChase
		},
		{ // 0x08 THE ROGUE TECH-SUPPORT SABOTEUR
			.Flags = cfLiving,
			.Tier = ctMedium,
			.BaseSprite = 28,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = SpecialActorTouchItem,
			.ActorThink = RogueSaboteurThink,
			.ActorChase = RogueSaboteurChase
		},
		{ // 0x09 THE POCKET-CODER REBEL
			.Flags = cfLiving,
			.Tier = ctMedium,
			.BaseSprite = 32,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = SpecialActorTouchItem,
			.ActorThink = CoderRebelThink,
			.ActorChase = CoderRebelChase
		},
		{ // 0x0A THE SIGNAL-SCOUT ROOKIE
			.Flags = cfLiving,
			.Tier = ctMedium,
			.BaseSprite = 36,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = SpecialActorTouchItem,
			.ActorThink = CoderRookieThink,
			.ActorChase = CoderRookieChase
		},
		{ // 0x0B RED WANNACRY
			.Flags = cfLivingDanger,
			.Tier = ctHigh,
			.BaseSprite = 40,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = SpecialActorTouchItem,
			.ActorThink = HackerRedThink,
			.ActorChase = HackerRedChase
		},
		{ // 0x0C PINK DARKHOTEL
			.Flags = cfLivingDanger,
			.Tier = ctHigh,
			.BaseSprite = 44,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = SpecialActorTouchItem,
			.ActorThink = HackerPinkThink,
			.ActorChase = HackerPinkChase
		},
		{ // 0x0D BLUE ETERNALBLUE
			.Flags = cfLivingDanger,
			.Tier = ctHigh,
			.BaseSprite = 48,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = SpecialActorTouchItem,
			.ActorThink = HackerBlueThink,
			.ActorChase = HackerBlueChase
		},
		{ // 0x0E ORANGE MIRAI
			.Flags = cfLivingDanger,
			.Tier = ctHigh,
			.BaseSprite = 52,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = SpecialActorTouchItem,
			.ActorThink = HackerOrangeThink,
			.ActorChase = HackerOrangeChase
		},
		{ // 0x0F PURPLE DARKSIDE
			.Flags = cfLivingDanger,
			.Tier = ctHigh,
			.BaseSprite = 56,
			.NumFrames = 1,
			.Directional = true,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = SpecialActorTouchItem,
			.ActorThink = HackerPurpleThink,
			.ActorChase = HackerPurpleChase
		},
		{ // 0x10 FileFragment
			.Flags = cfMandatoryPickup,
			.Tier = ctNone,
			.BaseSprite = 60,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 16,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x11 JunkFragment
			.Flags = cfMandatoryPickup,
			.Tier = ctNone,
			.BaseSprite = 62,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x12 File
			.Flags = cfMandatoryPickup,
			.Tier = ctNone,
			.BaseSprite = 64,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 256,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x13 LockedFile
			.Flags = cfMandatoryPickup,
			.Tier = ctNone,
			.BaseSprite = 66,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x14 CloudFile
			.Flags = cfMandatoryPickup,
			.Tier = ctNone,
			.BaseSprite = 68,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 4096,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x15 LockedCloudFile
			.Flags = cfMandatoryPickup,
			.Tier = ctNone,
			.BaseSprite = 70,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x16 Floppy
			.Flags = cfPickup,
			.Tier = ctNone,
			.BaseSprite = 72,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 4096,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x17 TerminalShell
			.Flags = cfMandatoryPickup,
			.Tier = ctNone,
			.BaseSprite = 74,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 256,
			.SpecialPlayerTouch = SpecialPlayerTerminal,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x18 EncryptedUSB
			.Flags = cfPickup,
			.Tier = ctNone,
			.BaseSprite = 76,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 4096,
			.SpecialPlayerTouch = nullptr,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x19 SmartCard
			.Flags = cfPickup,
			.Tier = ctNone,
			.BaseSprite = 78,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = SpecialPlayerSmartCard,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x1A SSD
			.Flags = cfPickup,
			.Tier = ctNone,
			.BaseSprite = 80,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 4096,
			.SpecialPlayerTouch = SpecialPlayerSSD,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x1B PGPKey
			.Flags = cfPickup,
			.Tier = ctNone,
			.BaseSprite = 82,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = SpecialPlayerPGPKey,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		},
		{ // 0x1C DisturbanceScript
			.Flags = cfPickup,
			.Tier = ctNone,
			.BaseSprite = 84,
			.NumFrames = 2,
			.Directional = false,
			.RecoveringBytes = 0,
			.SpecialPlayerTouch = SpecialPlayerDisturbanceScript,
			.SpecialActorTouch = nullptr,
			.ActorThink = nullptr,
			.ActorChase = nullptr
		}
	};
	TActor Player;
	TActor Actors[MaxActor]; // Array di attori nel labirinto
	bool MazeActors[MazeSize]; // Puntatore agli attori nel labirinto
	uint32_t MazeRegions[Video::MaxTile][MazeSize]; // Struttura ordinata per creare gli oggetti
	uint32_t MazeRegionsCount[Video::MaxTile];
	uint32_t MazeRegionsFirst[Video::MaxTile];
	uint32_t InspectingDevice; // Numero di livello
	uint32_t RecoveredBytes; // Punteggio
	uint32_t RemainingBackups; // Vite extra
	uint32_t RemainingPickups; // Numero di oggetti da raccogliere
	uint32_t GameTicker; // Orologio del gioco
	uint32_t RandSeed;
	int32_t DiagonalDeltaX;
	int32_t DiagonalDeltaY;
	bool PlayerHurted;

	uint32_t Random(uint32_t Limit)
	{
		if (Limit == 0)
			return 0;

		// Algoritmo LCG standard (formula di Park-Miller / POSIX)
		RandSeed = (RandSeed * 1103515245 + 12309) & 0x7FFFFFFF;
		return RandSeed % Limit;
	}

	int32_t Clamp(int32_t Value, int32_t Min, int32_t Max)
	{
		if (Value < Min)
			return Min;
		else
			if (Value > Max)
				return Max;
			else
				return Value;
	}

	int32_t Abs(int32_t Value)
	{
		return (Value < 0) ? -Value : Value;
	}

	void UpdatePanel()
	{
		char TempBuffer[12];

		Processor::IntToString(InspectingDevice + 1, TempBuffer);
		Video::DrawString(30, 4, TempBuffer);
		Processor::IntToString(RecoveredBytes, TempBuffer);
		Video::DrawString(30, 8, TempBuffer);
		Processor::IntToString(RemainingBackups, TempBuffer);
		Video::DrawString(30, 12, TempBuffer);
	}

	void DrawActor(const TActor& Actor)
	{
		constexpr uint32_t FrameOffset[4]{ 3, 1, 0, 2 };
		uint32_t FrameIndex;
		uint32_t SpriteIndex;

		FrameIndex = (GameTicker >> 4) & 0x00000001;
		SpriteIndex = ActorClasses[Actor.ActorClass].BaseSprite;
		if (ActorClasses[Actor.ActorClass].Directional)
			SpriteIndex += FrameOffset[Actor.Direction];
		if (ActorClasses[Actor.ActorClass].NumFrames > 1)
			SpriteIndex += FrameIndex;
		Video::DrawSprite((Actor.PosX * 3) - 32, (Actor.PosY * 3) - 32, SpriteIndex);
	}

	void DrawActors()
	{
		for (uint32_t i = 0; i < MaxActor; i++)
			if (Actors[i].ActorClass)
				DrawActor(Actors[i]);
	}

	void ActorInitialize(TActor& Actor, uint32_t ActorClass)
	{
		Actor.ActorClass = ActorClass;
		Actor.Accumulator = 0;
		Actor.Direction = MoveDown;
		Actor.Flags = ActorClasses[ActorClass].Flags;
		Actor.AttackCounter = 0;
		Actor.ReadyCounter = 0;
		switch (ActorClasses[ActorClass].Tier)
		{
		case ctLow:
			Actor.Speed = LevelInfo[InspectingDevice].SpeedLow;
			break;
		case ctMedium:
			Actor.Speed = LevelInfo[InspectingDevice].SpeedMedium;
			break;
		case ctHigh:
			Actor.Speed = LevelInfo[InspectingDevice].SpeedHigh;
			break;
		default:
			Actor.Speed = 0;
			break;
		}
	}

	void ActorPosition(TActor& Actor, uint32_t X, uint32_t Y)
	{
		Actor.PosX = (X << 3) + 4;
		Actor.PosY = (Y << 3) + 4;
		Actor.TileX = X;
		Actor.TileY = Y;
	}

	bool ActorSpawn(uint32_t ActorClass, uint32_t X, uint32_t Y)
	{
		uint32_t TileIndex;

		// Non creare script prima del livello 12
		if ((ActorClass == 0x1C) && (InspectingDevice < 12))
			return false;

		// Verifica se la casella e' gìa' occupata
		TileIndex = X + Y * 28;
		if (MazeActors[TileIndex])
			return false;

		for (uint32_t i = 0; i < MaxActor; i++)
		{
			if (Actors[i].ActorClass == 0)
			{
				ActorInitialize(Actors[i], ActorClass);
				ActorPosition(Actors[i], X, Y);
				// Verifica se la classe rappresenta un personaggio
				if (ActorClasses[ActorClass].Flags != cfLiving)
					MazeActors[TileIndex] = true;
				return true;
			}
		}
		return false;
	}

	void ActorCreateLiving(uint32_t ActorClass, uint32_t Region)
	{
		uint32_t RandomTile;
		uint32_t X;
		uint32_t Y;

		RandomTile = Random(MazeRegionsCount[Region]);
		RandomTile = MazeRegions[Region][RandomTile];
		X = RandomTile % 28;
		Y = RandomTile / 28;
		ActorSpawn(ActorClass, X, Y);
	}

	bool ActorCreateItemSingle(uint32_t ActorClass, uint32_t Region)
	{
		constexpr int32_t MaxIterations = 8;
		bool Result;
		int32_t Iteration;
		uint32_t RandomTile;
		uint32_t X;
		uint32_t Y;

		Result = false;
		Iteration = 0;
		do
		{
			Iteration++;
			RandomTile = Random(MazeRegionsCount[Region]);
			RandomTile = MazeRegions[Region][RandomTile];
			X = RandomTile % 28;
			Y = RandomTile / 28;
			Result = ActorSpawn(ActorClass, X, Y);
		} while ((!Result) && (Iteration < MaxIterations));
		return Result;
	}

	uint32_t ActorCreateItemMultiple(uint32_t ActorClass)
	{
		uint32_t Result;

		Result = 0;
		for (uint32_t i = 5; i <= 12; i++)
			if (ActorCreateItemSingle(ActorClass, i))
				Result++;
		return Result;
	}

	void ActorCreateItem(uint32_t ActorClass, int32_t Count)
	{
		constexpr int32_t MaxIterations = 8;
		int32_t Iteration;
		int32_t Remaining;

		Iteration = 0;
		Remaining = Count;
		while ((Remaining >= 8) && (Iteration < MaxIterations))
		{
			Remaining -= ActorCreateItemMultiple(ActorClass);
			Iteration++;
		}
		Iteration = 0;
		while ((Remaining > 0) && (Iteration < MaxIterations))
		{
			if (ActorCreateItemSingle(ActorClass, 4))
				Remaining--;
			Iteration++;
			if (Iteration >= MaxIterations)
				break;
		}
	}

	void ActorDestroy(TActor& Actor)
	{
		if (ActorClasses[Actor.ActorClass].Flags != cfLiving)
			MazeActors[Actor.TileX + Actor.TileY * 28] = false;
		Actor.ActorClass = 0;
	}

	void ActorDestroyAll()
	{
		for (uint32_t i = 0; i < MaxActor; i++)
			Actors[i].ActorClass = 0;
		for (uint32_t i = 0; i < MazeSize; i++)
			MazeActors[i] = false;
	}

	void LevelInitialize()
	{
		uint32_t ActorClass;

		// Imposta la velocita' degli attori
		for (int i = 0x01; i <= 0x02; i++)
		{
			ActorClasses[i].SpeedNormal = LevelInfo[InspectingDevice].SpeedPlayer;
			ActorClasses[i].SpeedFright = LevelInfo[InspectingDevice].SpeedPlayerFright;
		}
		for (int i = 0x03; i <= 0x06; i++)
		{
			ActorClasses[i].SpeedNormal = LevelInfo[InspectingDevice].SpeedLow;
			ActorClasses[i].SpeedFright = LevelInfo[InspectingDevice].SpeedLowFright;
		}
		for (int i = 0x07; i <= 0x0A; i++)
		{
			ActorClasses[i].SpeedNormal = LevelInfo[InspectingDevice].SpeedMedium;
			ActorClasses[i].SpeedFright = LevelInfo[InspectingDevice].SpeedMediumFright;
		}
		for (int i = 0x0B; i <= 0x0F; i++)
		{
			ActorClasses[i].SpeedNormal = LevelInfo[InspectingDevice].SpeedHigh;
			ActorClasses[i].SpeedFright = LevelInfo[InspectingDevice].SpeedHighFright;
		}

		// Rimuove tutti gli attori precedenti
		ActorDestroyAll();

		// Crea i Player nel livello
		if (InspectingDevice & 0x01)
			ActorInitialize(Player, 0x01);
		else
			ActorInitialize(Player, 0x02);
		ActorPosition(Player, 4, 15);

		// Crea gli altri attori nel livello
		if (LevelInfo[InspectingDevice].SpawnLow)
		{
			ActorCreateLiving(0x03, 9);
			ActorCreateLiving(0x04, 9);
			ActorCreateLiving(0x05, 9);
			ActorCreateLiving(0x06, 9);
		}
		if (LevelInfo[InspectingDevice].SpawnMedium)
		{
			ActorCreateLiving(0x07, 9);
			ActorCreateLiving(0x08, 9);
			ActorCreateLiving(0x09, 9);
			ActorCreateLiving(0x0A, 9);
		}
		if (LevelInfo[InspectingDevice].SpawnHigh)
		{
			ActorCreateLiving(0x0B, 9);
			ActorCreateLiving(0x0C, 9);
			ActorCreateLiving(0x0D, 9);
			ActorCreateLiving(0x0E, 9);
			ActorCreateLiving(0x0F, 9);
		}

		// Crea gli oggetti nel livello
		ActorCreateItem(0x10, LevelInfo[InspectingDevice].NumFileFragments);
		ActorCreateItem(0x11, LevelInfo[InspectingDevice].NumJunkFragments);
		ActorCreateItem(0x12, LevelInfo[InspectingDevice].NumFiles);
		ActorCreateItem(0x13, LevelInfo[InspectingDevice].NumLockedFiles);
		ActorCreateItem(0x14, LevelInfo[InspectingDevice].NumCloudFiles);
		ActorCreateItem(0x15, LevelInfo[InspectingDevice].NumLockedCloudFiles);
		ActorCreateItem(0x16, LevelInfo[InspectingDevice].NumFloppys);
		ActorCreateItem(0x17, LevelInfo[InspectingDevice].NumTerminalShells);
		ActorCreateItem(0x18, LevelInfo[InspectingDevice].NumEncryptedUSB);
		ActorCreateItem(0x19, LevelInfo[InspectingDevice].NumSmartCards);
		ActorCreateItem(0x1A, LevelInfo[InspectingDevice].NumSSDs);
		ActorCreateItem(0x1B, LevelInfo[InspectingDevice].NumPGPKeys);

		// Conta gli oggetti daraccogliere obbligatoriamente
		RemainingPickups = 0;
		for (uint32_t i = 0; i < MaxActor; i++)
		{
			// Uso intezionale di operatore di assegnamento!
			if (ActorClass = Actors[i].ActorClass)
			{
				if (ActorClasses[ActorClass].Flags & cfMandatory)
				{
					RemainingPickups++;
				}
			}
		}
	}

	void ResetActors()
	{
		uint32_t RandomTile;
		uint32_t X;
		uint32_t Y;

		ActorPosition(Player, 4, 15);
		for (uint32_t i = 0; i < MaxActor; i++)
		{
			if ((Actors[i].ActorClass) && (Actors[i].Flags & cfLiving))
			{
				RandomTile = Random(MazeRegionsCount[9]);
				RandomTile = MazeRegions[9][RandomTile];
				X = RandomTile % 28;
				Y = RandomTile / 28;
				ActorPosition(Player, X, Y);
			}
		}
	}

	TDirection GetOppositeDirection(TDirection Direction)
	{
		switch (Direction)
		{
		case MoveUp:
			return MoveDown;
		case MoveLeft:
			return MoveRight;
		case MoveDown:
			return MoveUp;
		default:
			return MoveLeft;
		}
	}

	bool CanActorMove(const TActor& Actor)
	{
		uint8_t MoveFlags;

		MoveFlags = MazeMovements[Actor.TileX + Actor.TileY * 28];
		switch (Actor.Direction)
		{
		case MoveUp:
			return ((MoveFlags & HackerCanMoveUp) || (Actor.PosY > (Actor.TileY * 8 + 4)));
		case MoveLeft:
			return ((MoveFlags & HackerCanMoveLeft) || (Actor.PosX > (Actor.TileX * 8 + 4)));
		case MoveDown:
			return ((MoveFlags & HackerCanMoveDown) || (Actor.PosY < (Actor.TileY * 8 + 4)));
		default:
			return ((MoveFlags & HackerCanMoveRight) || (Actor.PosX < (Actor.TileX * 8 + 4)));
		}
	}

	bool CanActorChangeDir(const TActor& Actor)
	{
		return ((Actor.PosX & 0x07) == 0x04) && ((Actor.PosY & 0x07) == 0x04);
	}

	void ChooseRandomDirection(TActor& Actor)
	{
		TDirection ValidDirs[4];
		uint32_t Count;
		uint8_t MoveFlags;

		MoveFlags = MazeMovements[Actor.TileX + Actor.TileY * 28];
		Count = 0;
		if ((MoveFlags & HackerCanMoveUp) && ((!(Actor.Flags & cfRegionLocked)) || (MazeTiles[Actor.TileX + Actor.TileY * 28] == MazeTiles[Actor.TileX + (Actor.TileY - 1) * 28])))
			ValidDirs[Count++] = MoveUp;
		if ((MoveFlags & HackerCanMoveLeft) && ((!(Actor.Flags & cfRegionLocked)) || (MazeTiles[Actor.TileX + Actor.TileY * 28] == MazeTiles[(Actor.TileX - 1) + Actor.TileY * 28])))
			ValidDirs[Count++] = MoveLeft;
		if ((MoveFlags & HackerCanMoveDown) && ((!(Actor.Flags & cfRegionLocked)) || (MazeTiles[Actor.TileX + Actor.TileY * 28] == MazeTiles[Actor.TileX + (Actor.TileY + 1) * 28])))
			ValidDirs[Count++] = MoveDown;
		if ((MoveFlags & HackerCanMoveRight) && ((!(Actor.Flags & cfRegionLocked)) || (MazeTiles[Actor.TileX + Actor.TileY * 28] == MazeTiles[(Actor.TileX + 1) + Actor.TileY * 28])))
			ValidDirs[Count++] = MoveRight;
		if (Count > 0)
			Actor.Direction = ValidDirs[Random(Count)];
	}

	bool CanPlayerMove()
	{
		uint8_t MoveFlags;

		MoveFlags = MazeMovements[Player.TileX + Player.TileY * 28];
		switch (Player.Direction)
		{
		case MoveUp:
			return ((MoveFlags & PlayerCanMoveUp) || (Player.PosY > (Player.TileY * 8 + 4)));
		case MoveLeft:
			return ((MoveFlags & PlayerCanMoveLeft) || (Player.PosX > (Player.TileX * 8 + 4)));
		case MoveDown:
			return ((MoveFlags & PlayerCanMoveDown) || (Player.PosY < (Player.TileY * 8 + 4)));
		case MoveRight:
			return ((MoveFlags & PlayerCanMoveRight) || (Player.PosX < (Player.TileX * 8 + 4)));
		}
		return false;
	}

	bool CanPlayerChangeDir(TUserInput UserInput)
	{
		uint8_t MoveFlags;

		MoveFlags = MazeMovements[Player.TileX + Player.TileY * 28];
		switch (UserInput)
		{
		case InputUp:
			return (MoveFlags & PlayerCanMoveUp) != 0;
		case InputLeft:
			return (MoveFlags & PlayerCanMoveLeft) != 0;
		case InputDown:
			return (MoveFlags & PlayerCanMoveDown) != 0;
		case InputRight:
			return (MoveFlags & PlayerCanMoveRight) != 0;
		}
		return false;
	}

	void MoveActorUp(TActor& Actor)
	{
		Actor.PosY--;
		Actor.TileX = Actor.PosX >> 3;
		Actor.TileY = Actor.PosY >> 3;
	}

	void MoveActorLeft(TActor& Actor)
	{
		Actor.PosX--;
		Actor.TileX = Actor.PosX >> 3;
		Actor.TileY = Actor.PosY >> 3;
	}

	void MoveActorDown(TActor& Actor)
	{
		Actor.PosY++;
		Actor.TileX = Actor.PosX >> 3;
		Actor.TileY = Actor.PosY >> 3;
	}

	void MoveActorRight(TActor& Actor)
	{
		Actor.PosX++;
		Actor.TileX = Actor.PosX >> 3;
		Actor.TileY = Actor.PosY >> 3;
	}

	void MoveActor(TActor& Actor)
	{
		switch (Actor.Direction)
		{
		case MoveUp:
			Actor.PosY--;
			break;
		case MoveLeft:
			Actor.PosX--;
			break;
		case MoveDown:
			Actor.PosY++;
			break;
		case MoveRight:
			Actor.PosX++;
			break;
		}
		Actor.TileX = Actor.PosX >> 3;
		Actor.TileY = Actor.PosY >> 3;
	}

	void HandlePlayer()
	{
		if (Player.FrightenedCount > 0)
		{
			Player.FrightenedCount--;
			if (Player.FrightenedCount == 0)
				Player.Flags &= ~cfFrightened;
		}
		if (Player.InvincibleCount > 0)
		{
			Player.InvincibleCount--;
			if (Player.InvincibleCount == 0)
				Player.Flags &= ~cfInvincible;
		}
		if (Player.RegionLockedCount > 0)
		{
			Player.RegionLockedCount--;
			if (Player.RegionLockedCount == 0)
				Player.Flags &= ~cfRegionLocked;
		}
		if (Player.DisturbedCount > 0)
		{
			Player.DisturbedCount--;
			if (Player.DisturbedCount == 0)
				Player.Flags &= ~cfDisturbed;
		}
		if (CanPlayerMove())
		{
			if (Player.Flags & cfFrightened)
				Player.Speed = LevelInfo[InspectingDevice].SpeedPlayerFright;
			else
				Player.Speed = LevelInfo[InspectingDevice].SpeedPlayer;
			if (Player.Flags & cfDisturbed)
				Player.Speed >>= 2;
			Player.Accumulator += Player.Speed;
			while (Player.Accumulator >= 256)
			{
				MoveActor(Player);
				Player.Accumulator -= 256;
				if (DiagonalDeltaX > 0)
				{
					MoveActorRight(Player);
					DiagonalDeltaX--;
				}
				if (DiagonalDeltaX < 0)
				{
					MoveActorLeft(Player);
					DiagonalDeltaX++;
				}
				if (DiagonalDeltaY > 0)
				{
					MoveActorDown(Player);
					DiagonalDeltaY--;
				}
				if (DiagonalDeltaY < 0)
				{
					MoveActorUp(Player);
					DiagonalDeltaY++;
				}
			}
		}
	}

	bool CheckLineOfSight(int32_t X0, int32_t Y0, int32_t X1, int32_t Y1)
	{
		// X0, Y0 = Tile coordinate di partenza
		// X1, Y1 = Tile coordinate di arrivo
		int32_t dX = Abs(X1 - X0);
		int32_t dY = Abs(Y1 - Y0);
		int32_t X = X0;
		int32_t Y = Y0;
		int32_t N = 1 + dX + dY;
		int32_t xInc = (X1 > X0) ? 1 : -1;
		int32_t yInc = (Y1 > Y0) ? 1 : -1;
		int32_t Error = dX - dY;
		dX *= 2;
		dY *= 2;
		for (; N > 0; --N) 
		{
			// Verifica se la tile corrente (x, y) è un blocco/muro che ostruisce la vista
			uint32_t TileIndex = X + Y * 28;
			if (X < 0 || X > 27 || Y < 0 || Y > 29) 
				return false;
			// Le tile 0 e 1 bloccano la vista
			if (MazeTiles[TileIndex] < 2) 
			{
				return false; // Linea di vista bloccata!
			}
			if (X == X1 && Y == Y1) 
				break;
			if (Error > 0) 
			{
				X += xInc;
				Error -= dY;
			}
			else 
			{
				Y += yInc;
				Error += dX;
			}
		}
		return true; // Linea di vista completamente libera!
	}

	bool CheckAttackReady()
	{
		uint32_t Count;

		Count = 0;
		for (uint32_t i = 0; i < MaxActor; i++)
			if ((Actors[i].ActorClass) && (Actors[i].Flags & cfLiving) && (Actors[i].ReadyCounter))
				Count++;
		return (Count >= 3);
	}

	uint32_t CalcDistance(int32_t X0, int32_t Y0, int32_t X1, int32_t Y1)
	{
		return (X1 - X0) * (X1 - X0) + (Y1 - Y0) * (Y1 - Y0);
	}

	uint32_t CalcDistancePlayerOffset(int32_t X0, int32_t Y0, int32_t Offset)
	{
		int32_t PlayerX, PlayerY;

		PlayerX = Player.TileX;
		PlayerY = Player.TileY;
		switch (Player.Direction)
		{
		case MoveUp:
			PlayerY -= Offset;
			break;
		case MoveLeft:
			PlayerX -= Offset;
			break;
		case MoveDown:
			PlayerY += Offset;
			break;
		default:
			PlayerX += Offset;
		}
		return (PlayerX - X0) * (PlayerX - X0) + (PlayerY - Y0) * (PlayerY - Y0);
	}

	uint32_t CalcNextDistanceToPlayer(TActor& Actor, TDirection Direction)
	{
		uint32_t ActorX, ActorY;

		ActorX = Actor.PosX;
		ActorY = Actor.PosY;
		switch (Direction)
		{
		case MoveUp:
			ActorY--;
			break;
		case MoveLeft:
			ActorX--;
			break;
		case MoveDown:
			ActorY++;
			break;
		default:
			ActorX++;
		}
		return CalcDistance(ActorX, ActorY, Player.PosX, Player.PosY);
	}

	uint8_t GetActorValidDirections(const TActor& Actor, TDirection* Directions)
	{
		uint8_t ValidMovements;
		uint8_t Result;

		Result = 0;
		ValidMovements = MazeMovements[Actor.TileX + Actor.TileY * 28];
		if (ValidMovements & HackerCanMoveUp)
		{
			*Directions = MoveUp;
			Directions++;
			Result++;
		}
		if (ValidMovements & HackerCanMoveLeft)
		{
			*Directions = MoveLeft;
			Directions++;
			Result++;
		}
		if (ValidMovements & HackerCanMoveDown)
		{
			*Directions = MoveDown;
			Directions++;
			Result++;
		}
		if (ValidMovements & HackerCanMoveRight)
		{
			*Directions = MoveRight;
			Directions++;
			Result++;
		}
		return Result;
	}

	TDirection GetShortestDirectionToPlayer(const TActor& Actor)
	{
		uint32_t Distance, MinDistance;
		uint8_t ValidMovements;
		TDirection Result;

		Result = MoveDown;
		MinDistance = 32767;
		ValidMovements = MazeMovements[Actor.TileX + Actor.TileY * 28];
		if (ValidMovements & HackerCanMoveUp)
		{
			Distance = CalcDistance(Actor.TileX, Actor.TileY - 1, Player.TileX, Player.TileY);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				Result = MoveUp;
			}
		}
		if (ValidMovements & HackerCanMoveLeft)
		{
			Distance = CalcDistance(Actor.TileX - 1, Actor.TileY, Player.TileX, Player.TileY);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				Result = MoveLeft;
			}
		}
		if (ValidMovements & HackerCanMoveDown)
		{
			Distance = CalcDistance(Actor.TileX, Actor.TileY + 1, Player.TileX, Player.TileY);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				Result = MoveDown;
			}
		}
		if (ValidMovements & HackerCanMoveRight)
		{
			Distance = CalcDistance(Actor.TileX + 1, Actor.TileY, Player.TileX, Player.TileY);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				Result = MoveRight;
			}
		}
		return Result;
	}

	bool GetCorridorExit(uint32_t TileIndex, uint32_t* CorridorExitTile)
	{
		uint8_t TileRegion;
		bool Result;

		Result = false;
		TileRegion = MazeTiles[TileIndex];
		if ((TileIndex >= 28) && (MazeTiles[TileIndex - 28] == 4))
		{
			*CorridorExitTile = TileIndex - 28;
			Result = true;
		}
		if ((TileIndex >= 1) && (MazeTiles[TileIndex - 1] == 4))
		{
			*CorridorExitTile = TileIndex - 1;
			Result = true;
		}
		if ((TileIndex < (MazeSize - 28)) && (MazeTiles[TileIndex + 28] == 4))
		{
			*CorridorExitTile = TileIndex + 28;
			Result = true;
		}
		if ((TileIndex < (MazeSize - 1)) && (MazeTiles[TileIndex + 1] == 4))
		{
			*CorridorExitTile = TileIndex + 1;
			Result = true;
		}
		return Result;
	}

	TDirection GetNearestRegionExit(const TActor& Actor)
	{
		uint32_t ExitingTiles[15];
		uint32_t TestTile, ExitingTilesCount, Distance, MinDistance, PreferredExitingTile;
		int32_t TargetX, TargetY;
		uint8_t ActorRegion;
		TDirection Result;

		ActorRegion = MazeTiles[Actor.TileX + Actor.TileY * 28];
		ExitingTiles[0] = Actor.Direction;
		ExitingTilesCount = 0;
		// Cerca le uscite dal blocco verso i corridoi
		for (uint32_t i = 0; i < MazeRegionsCount[ActorRegion]; i++)
		{
			if (GetCorridorExit(MazeRegions[ActorRegion][i], &TestTile))
			{
				ExitingTiles[ExitingTilesCount] = TestTile;
				ExitingTilesCount++;
				if (ExitingTilesCount == 16)
					break;
			}
		}
		// Almeno una uscita ci deve essere!
		PreferredExitingTile = ExitingTiles[0];
		MinDistance = CalcDistance(Actor.TileX, Actor.TileY, ExitingTiles[0] % 28, ExitingTiles[0] / 28);
		for (uint32_t i = 1; i < ExitingTilesCount; i++)
		{
			Distance = CalcDistance(Actor.TileX, Actor.TileY, ExitingTiles[0] % 28, ExitingTiles[0] / 28);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				PreferredExitingTile = ExitingTiles[i];
			}
		}
		Result = Actor.Direction;
		TargetX = PreferredExitingTile % 28;
		TargetY = PreferredExitingTile / 28;
		if (TargetY < Actor.TileY)
			Result = MoveUp;
		if (TargetX < Actor.TileX)
			Result = MoveLeft;
		if (TargetY > Actor.TileY)
			Result = MoveDown;
		if (TargetX < Actor.TileX)
			Result = MoveRight;
		return Result;
	}

	void SetDirection(TActor& Actor, TDirection Direction)
	{
		if (!Actor.ChangeDirCounter)
		{
			Actor.Direction = Direction;
			Actor.ChangeDirCounter = 0;
		}
	}

	void LoudBasementStreamerThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection, ShortestDirection;
		uint8_t ActorRegion, PlayerRegion, ValidDirectionsCount;
		bool DirectionSet;

		ActorRegion = MazeTiles[Actor.TileX + Actor.TileY * 28];
		PlayerRegion = MazeTiles[Player.TileX + Player.TileY * 28];
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		if (ActorRegion > 4)
		{
			if (PlayerRegion == ActorRegion)
			{
				if (ValidDirectionsCount)
				{
					SetDirection(Actor, ValidDirections[Random(ValidDirectionsCount)]);
				}
				else
				{
					SetDirection(Actor, GetOppositeDirection(Actor.Direction));
				}
			}
			else
			{
				DirectionSet = false;
				OppositeDirection = GetOppositeDirection(Actor.Direction);
				ShortestDirection = GetShortestDirectionToPlayer(Actor);
				for (int i = 0; i < ValidDirectionsCount; i++)
					if (ShortestDirection == ValidDirections[i])
					{
						PreferredDirection = ShortestDirection;
						DirectionSet = true;
						break;
					}
				if (DirectionSet)
				{
					SetDirection(Actor, PreferredDirection);
				}
				else
				{
					SetDirection(Actor, ValidDirections[Random(ValidDirectionsCount)]);
				}
			}
		}
		else
		{
			DirectionSet = false;
			OppositeDirection = GetOppositeDirection(Actor.Direction);
			ShortestDirection = GetShortestDirectionToPlayer(Actor);
			for (int i = 0; i < ValidDirectionsCount; i++)
				if (ShortestDirection == ValidDirections[i])
				{
					PreferredDirection = ShortestDirection;
					DirectionSet = true;
					break;
				}
			if (DirectionSet)
			{
				if ((PreferredDirection != OppositeDirection) || (Random(255) < 8))
				{
					SetDirection(Actor, PreferredDirection);
				}
			}
			else
			{
				SetDirection(Actor, ValidDirections[Random(ValidDirectionsCount)]);
			}
		}
		if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
		{
			if (Random(255) < 4)
				ActorSpawn(0x11, Actor.TileX, Actor.TileY);
		}
	}

	void CasualCoffeShopDevThink(TActor& Actor)
	{
		TDirection ValidDirections[4], ValidDirectionsInBlock[4];
		TDirection OppositeDirection, PreferredDirection, ShortestDirection;
		uint8_t ActorRegion, PlayerRegion, ValidDirectionsCount, ValidDirectionsInBlockCount, NextRegion;
		bool DirectionSet, StayInRegion;

		ActorRegion = MazeTiles[Actor.TileX + Actor.TileY * 28];
		PlayerRegion = MazeTiles[Player.TileX + Player.TileY * 28];
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		if (ActorRegion > 4)
		{
			if (Random(255) < 200)
				StayInRegion = true;
			else
				StayInRegion = false;
			if (StayInRegion)
			{
				ValidDirectionsInBlockCount = 0;
				for (int i = 0; i < ValidDirectionsCount; i++)
				{
					switch (ValidDirections[i])
					{
					case MoveUp:
						NextRegion = MazeTiles[Actor.TileX + (Actor.TileY - 1) * 28];
						break;
					case MoveLeft:
						NextRegion = MazeTiles[(Actor.TileX - 1) + Actor.TileY * 28];
						break;
					case MoveDown:
						NextRegion = MazeTiles[Actor.TileX + (Actor.TileY + 1) * 28];
						break;
					default: // case MoveRight:
						NextRegion = MazeTiles[(Actor.TileX + 1) + Actor.TileY * 28];
					//	break;
					}
					if (NextRegion == ActorRegion)
					{
						ValidDirectionsInBlock[ValidDirectionsInBlockCount] = ValidDirections[i];
						ValidDirectionsInBlockCount++;
					}
					if (ValidDirectionsInBlockCount)
					{
						SetDirection(Actor, ValidDirectionsInBlock[Random(ValidDirectionsCount)]);
					}
					else
					{
						SetDirection(Actor, GetOppositeDirection(Actor.Direction));
					}
				}
			}
			else
			{
				SetDirection(Actor, GetNearestRegionExit(Actor));
			}
		}
		else
		{
			DirectionSet = false;
			OppositeDirection = GetOppositeDirection(Actor.Direction);
			ShortestDirection = Actor.Direction;
			for (int i = 0; i < ValidDirectionsCount; i++)
				if (ShortestDirection == ValidDirections[i])
				{
					PreferredDirection = ShortestDirection;
					DirectionSet = true;
					break;
				}
			if (DirectionSet)
			{
				if ((PreferredDirection != OppositeDirection) || (Random(255) < 8))
				{
					SetDirection(Actor, PreferredDirection);
				}
			}
			else
			{
				SetDirection(Actor, ValidDirections[Random(ValidDirectionsCount)]);
			}
		}
		if (Random(255) < 2)
			ActorSpawn(0x11, Actor.TileX, Actor.TileY);
	}

	void SnakeOilSalesmanThink(TActor &Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection, ShortestDirection;
		uint8_t ActorRegion, PlayerRegion, ValidDirectionsCount;
		bool DirectionSet;

		ActorRegion = MazeTiles[Actor.TileX + Actor.TileY * 28];
		PlayerRegion = MazeTiles[Player.TileX + Player.TileY * 28];
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		if (ActorRegion > 4)
		{
			if (PlayerRegion == ActorRegion)
			{
				if (ValidDirectionsCount)
				{
					SetDirection(Actor, ValidDirections[Random(ValidDirectionsCount)]);
				}
				else
				{
					SetDirection(Actor, GetOppositeDirection(Actor.Direction));
				}
			}
			else
			{
				DirectionSet = false;
				OppositeDirection = GetOppositeDirection(Actor.Direction);
				ShortestDirection = GetShortestDirectionToPlayer(Actor);
				for (int i = 0; i < ValidDirectionsCount; i++)
					if (ShortestDirection == ValidDirections[i])
					{
						PreferredDirection = ShortestDirection;
						DirectionSet = true;
						break;
					}
				if (DirectionSet)
				{
					SetDirection(Actor, PreferredDirection);
				}
				else
				{
					SetDirection(Actor, ValidDirections[Random(ValidDirectionsCount)]);
				}
			}
		}
		else
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				if (Random(255) < 8)
					ActorSpawn(0x11, Actor.TileX, Actor.TileY);
				SetDirection(Actor, GetNearestRegionExit(Actor));
			}
			else
			{
				DirectionSet = false;
				OppositeDirection = GetOppositeDirection(Actor.Direction);
				ShortestDirection = Actor.Direction;
				for (int i = 0; i < ValidDirectionsCount; i++)
					if (ShortestDirection == ValidDirections[i])
					{
						PreferredDirection = ShortestDirection;
						DirectionSet = true;
						break;
					}
				if (DirectionSet)
				{
					if ((PreferredDirection != OppositeDirection) || (Random(255) < 8))
					{
						SetDirection(Actor, PreferredDirection);
					}
				}
				else
				{
					SetDirection(Actor, ValidDirections[Random(ValidDirectionsCount)]);
				}
			}
		}
	}

	void ClumsyDigitalSquatterThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		uint8_t ActorRegion, PlayerRegion, ValidDirectionsCount;
		bool DirectionSet;

		ActorRegion = MazeTiles[Actor.TileX + Actor.TileY * 28];
		PlayerRegion = MazeTiles[Player.TileX + Player.TileY * 28];
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		if (ActorRegion > 4)
		{
			if (Random(255) < 4)
				ActorSpawn(0x11, Actor.TileX, Actor.TileY);
			if (Random(255) < 200)
				DirectionSet = true;
			else
				DirectionSet = false;
			if (DirectionSet)
			{
				if (ValidDirectionsCount)
				{
					SetDirection(Actor, ValidDirections[Random(ValidDirectionsCount)]);
				}
				else
				{
					SetDirection(Actor, GetOppositeDirection(Actor.Direction));
				}
			}
			else
			{
				SetDirection(Actor, GetNearestRegionExit(Actor));
			}
		}
		else
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				if (Random(255) < 8)
					ActorSpawn(0x11, Actor.TileX, Actor.TileY);
				SetDirection(Actor, GetNearestRegionExit(Actor));
			}
			else
			{
				if (Random(255) < 200)
					DirectionSet = false;
				else
					DirectionSet = true;
				if (DirectionSet)
				{
					SetDirection(Actor, ValidDirections[Random(ValidDirectionsCount)]);
				}
			}
		}
	}

	void ScriptKiddieThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection;
		uint32_t DirectionDistances[4], MinDistance;
		uint8_t ValidDirectionsCount;

		OppositeDirection = GetOppositeDirection(Actor.Direction);
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		for (uint32_t i = 0; i < ValidDirectionsCount; i++)
		{
			DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
		}
		if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcDistance(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY);
				if ((DirectionDistances[i] >= 9) && (DirectionDistances[i] <= 25))
				{
					SetDirection(Actor, ValidDirections[i]);
					break;
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
			}
			MinDistance = 0x7FFFFFFF;
			PreferredDirection = Actor.Direction;
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				if ((DirectionDistances[i] < MinDistance) && ((ValidDirections[i] != OppositeDirection) || (Random(255) < 8)))
				{
					PreferredDirection = ValidDirections[i];
					MinDistance = DirectionDistances[i];
				}
			}
			SetDirection(Actor, PreferredDirection);
		}
	}

	void ScriptKiddieChase(TActor& Actor)
	{
		if (!Actor.AttackCounter)
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				if (Actor.Direction == Player.Direction)
				{
					Actor.AttackCounter = 120;
				}
				else
				{
					if (Actor.Direction == GetOppositeDirection(Player.Direction))
					{
						ActorSpawn(0x1C, Actor.TileX, Actor.TileY);
						Actor.AttackCounter = 60;
					}
				}
			}
		}
		MoveActor(Actor);
	}

	void RogueSaboteurThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection;
		uint32_t DirectionDistances[4], MinDistance;
		uint8_t ValidDirectionsCount;

		OppositeDirection = GetOppositeDirection(Actor.Direction);
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		for (uint32_t i = 0; i < ValidDirectionsCount; i++)
		{
			DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
		}
		if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcDistance(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY);
				if ((DirectionDistances[i] >= 36) && (DirectionDistances[i] <= 144))
				{
					SetDirection(Actor, ValidDirections[i]);
					break;
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
			}
			MinDistance = 0x7FFFFFFF;
			PreferredDirection = Actor.Direction;
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				if ((DirectionDistances[i] < MinDistance) && ((ValidDirections[i] != OppositeDirection) || (Random(255) < 64)))
				{
					PreferredDirection = ValidDirections[i];
					MinDistance = DirectionDistances[i];
				}
			}
			SetDirection(Actor, PreferredDirection);
		}
	}

	void RogueSaboteurChase(TActor& Actor)
	{
		if (!Actor.AttackCounter)
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				if (Actor.Direction == Player.Direction)
				{
					Actor.AttackCounter = 240;
				}
				else
				{
					if (Actor.Direction == GetOppositeDirection(Player.Direction))
					{
						ActorSpawn(0x1C, Actor.TileX, Actor.TileY);
						Actor.AttackCounter = 300;
					}
				}
			}
		}
		MoveActor(Actor);
	}

	void CoderRebelThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection;
		uint32_t DirectionDistances[4], MinDistance;
		uint8_t ValidDirectionsCount;

		OppositeDirection = GetOppositeDirection(Actor.Direction);
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		for (uint32_t i = 0; i < ValidDirectionsCount; i++)
		{
			DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
		}
		if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcDistancePlayerOffset(Actor.TileX, Actor.TileY, 2);
				if ((DirectionDistances[i] >= 16) && (DirectionDistances[i] <= 25))
				{
					SetDirection(Actor, ValidDirections[i]);
					break;
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
			}
			MinDistance = 0x7FFFFFFF;
			PreferredDirection = Actor.Direction;
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				if ((DirectionDistances[i] < MinDistance) && ((ValidDirections[i] != OppositeDirection) || (Random(255) < 32)))
				{
					PreferredDirection = ValidDirections[i];
					MinDistance = DirectionDistances[i];
				}
			}
			SetDirection(Actor, PreferredDirection);
		}
	}

	void CoderRebelChase(TActor& Actor)
	{
		if (!Actor.AttackCounter)
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				ActorSpawn(0x1C, Actor.TileX, Actor.TileY);
				Actor.AttackCounter = 60;
			}
		}
		MoveActor(Actor);
	}

	void CoderRookieThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection;
		uint32_t DirectionDistances[4], MinDistance;
		uint8_t ValidDirectionsCount;

		OppositeDirection = GetOppositeDirection(Actor.Direction);
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		for (uint32_t i = 0; i < ValidDirectionsCount; i++)
		{
			DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
		}
		if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcDistancePlayerOffset(Actor.TileX, Actor.TileY, 4);
				if ((DirectionDistances[i] >= 16) && (DirectionDistances[i] <= 25))
				{
					SetDirection(Actor, ValidDirections[i]);
					break;
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
			}
			MinDistance = 0x7FFFFFFF;
			PreferredDirection = Actor.Direction;
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				if ((DirectionDistances[i] < MinDistance) && ((ValidDirections[i] != OppositeDirection) || (Random(255) < 32)))
				{
					PreferredDirection = ValidDirections[i];
					MinDistance = DirectionDistances[i];
				}
			}
			SetDirection(Actor, PreferredDirection);
		}
	}

	void HackerRedThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection;
		uint32_t DirectionDistances[4], MinDistance;
		uint8_t ValidDirectionsCount;

		OppositeDirection = GetOppositeDirection(Actor.Direction);
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		for (uint32_t i = 0; i < ValidDirectionsCount; i++)
		{
			DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
		}
		if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
		{
			if (CheckAttackReady())
			{
				PreferredDirection = Actor.Direction;
				MinDistance = 0x7FFFFFF;
				for (uint32_t i = 0; i < ValidDirectionsCount; i++)
				{
					DirectionDistances[i] = CalcDistance(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY);
					if ((DirectionDistances[i] < MinDistance))
					{
						PreferredDirection = ValidDirections[i];
					}
				}
				SetDirection(Actor, PreferredDirection);
			}
			else
			{
				for (uint32_t i = 0; i < ValidDirectionsCount; i++)
				{
					DirectionDistances[i] = CalcDistancePlayerOffset(Actor.TileX, Actor.TileY, 0);
					if ((DirectionDistances[i] >= 16) && (DirectionDistances[i] <= 25))
					{
						SetDirection(Actor, ValidDirections[i]);
						break;
					}
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
			}
			MinDistance = 0x7FFFFFFF;
			PreferredDirection = Actor.Direction;
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				if ((DirectionDistances[i] < MinDistance) && ((ValidDirections[i] != OppositeDirection) || (Random(255) < 32)))
				{
					PreferredDirection = ValidDirections[i];
					MinDistance = DirectionDistances[i];
				}
			}
			SetDirection(Actor, PreferredDirection);
		}
	}

	void HackerRedChase(TActor& Actor)
	{
		if (!Actor.AttackCounter)
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				ActorSpawn(0x1C, Actor.TileX, Actor.TileY);
				Actor.AttackCounter = 60;
				Actor.ReadyCounter = 60;
			}
		}
		MoveActor(Actor);
	}

	void HackerPinkThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection;
		uint32_t DirectionDistances[4], MinDistance;
		uint8_t ValidDirectionsCount;

		OppositeDirection = GetOppositeDirection(Actor.Direction);
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		for (uint32_t i = 0; i < ValidDirectionsCount; i++)
		{
			DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
		}
		if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
		{
			if (CheckAttackReady())
			{
				PreferredDirection = Actor.Direction;
				MinDistance = 0x7FFFFFF;
				for (uint32_t i = 0; i < ValidDirectionsCount; i++)
				{
					DirectionDistances[i] = CalcDistance(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY);
					if ((DirectionDistances[i] < MinDistance))
					{
						PreferredDirection = ValidDirections[i];
					}
				}
				SetDirection(Actor, PreferredDirection);
			}
			else
			{
				for (uint32_t i = 0; i < ValidDirectionsCount; i++)
				{
					DirectionDistances[i] = CalcDistancePlayerOffset(Actor.TileX, Actor.TileY, 4);
					if ((DirectionDistances[i] >= 16) && (DirectionDistances[i] <= 25))
					{
						SetDirection(Actor, ValidDirections[i]);
						break;
					}
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
			}
			MinDistance = 0x7FFFFFFF;
			PreferredDirection = Actor.Direction;
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				if ((DirectionDistances[i] < MinDistance) && ((ValidDirections[i] != OppositeDirection) || (Random(255) < 32)))
				{
					PreferredDirection = ValidDirections[i];
					MinDistance = DirectionDistances[i];
				}
			}
			SetDirection(Actor, PreferredDirection);
		}
	}

	void HackerPinkChase(TActor& Actor)
	{
		if (!Actor.AttackCounter)
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				ActorSpawn(0x1C, Actor.TileX, Actor.TileY);
				Actor.AttackCounter = 60;
				Actor.ReadyCounter = 60;
			}
		}
		MoveActor(Actor);
	}

	void HackerBlueThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection;
		uint32_t DirectionDistances[4], MinDistance;
		uint8_t ValidDirectionsCount;

		OppositeDirection = GetOppositeDirection(Actor.Direction);
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		for (uint32_t i = 0; i < ValidDirectionsCount; i++)
		{
			DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
		}
		if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
		{
			if (CheckAttackReady())
			{
				PreferredDirection = Actor.Direction;
				MinDistance = 0x7FFFFFF;
				for (uint32_t i = 0; i < ValidDirectionsCount; i++)
				{
					DirectionDistances[i] = CalcDistance(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY);
					if ((DirectionDistances[i] < MinDistance))
					{
						PreferredDirection = ValidDirections[i];
					}
				}
				SetDirection(Actor, PreferredDirection);
			}
			else
			{
				for (uint32_t i = 0; i < ValidDirectionsCount; i++)
				{
					DirectionDistances[i] = CalcDistancePlayerOffset(14, 15, 0);
					if ((DirectionDistances[i] >= 16) && (DirectionDistances[i] <= 25))
					{
						SetDirection(Actor, ValidDirections[i]);
						break;
					}
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
			}
			MinDistance = 0x7FFFFFFF;
			PreferredDirection = Actor.Direction;
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				if ((DirectionDistances[i] < MinDistance) && ((ValidDirections[i] != OppositeDirection) || (Random(255) < 32)))
				{
					PreferredDirection = ValidDirections[i];
					MinDistance = DirectionDistances[i];
				}
			}
			SetDirection(Actor, PreferredDirection);
		}
	}

	void HackerBlueChase(TActor& Actor)
	{
		if (!Actor.AttackCounter)
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				ActorSpawn(0x1C, Actor.TileX, Actor.TileY);
				Actor.AttackCounter = 60;
				Actor.ReadyCounter = 60;
			}
		}
		MoveActor(Actor);
	}

	void HackerOrangeThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection;
		uint32_t DirectionDistances[4], MinDistance;
		uint8_t ValidDirectionsCount;

		OppositeDirection = GetOppositeDirection(Actor.Direction);
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		for (uint32_t i = 0; i < ValidDirectionsCount; i++)
		{
			DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
		}
		if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
		{
			if (CheckAttackReady())
			{
				PreferredDirection = Actor.Direction;
				MinDistance = 0x7FFFFFF;
				for (uint32_t i = 0; i < ValidDirectionsCount; i++)
				{
					DirectionDistances[i] = CalcDistance(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY);
					if ((DirectionDistances[i] < MinDistance))
					{
						PreferredDirection = ValidDirections[i];
					}
				}
				SetDirection(Actor, PreferredDirection);
			}
			else
			{
				for (uint32_t i = 0; i < ValidDirectionsCount; i++)
				{
					DirectionDistances[i] = CalcDistancePlayerOffset(7, 15, 0);
					if ((DirectionDistances[i] >= 16) && (DirectionDistances[i] <= 25))
					{
						SetDirection(Actor, ValidDirections[i]);
						break;
					}
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcDistancePlayerOffset(7, 15, 0);
			}
			MinDistance = 0x7FFFFFFF;
			PreferredDirection = Actor.Direction;
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				if ((DirectionDistances[i] < MinDistance) && ((ValidDirections[i] != OppositeDirection) || (Random(255) < 32)))
				{
					PreferredDirection = ValidDirections[i];
					MinDistance = DirectionDistances[i];
				}
			}
			SetDirection(Actor, PreferredDirection);
		}
	}

	void HackerOrangeChase(TActor& Actor)
	{
		if (!Actor.AttackCounter)
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				ActorSpawn(0x1C, Actor.TileX, Actor.TileY);
				Actor.AttackCounter = 60;
				Actor.ReadyCounter = 60;
			}
		}
		MoveActor(Actor);
	}

	void HackerPurpleThink(TActor& Actor)
	{
		TDirection ValidDirections[4];
		TDirection OppositeDirection, PreferredDirection;
		uint32_t DirectionDistances[4], MinDistance;
		uint8_t ValidDirectionsCount;

		OppositeDirection = GetOppositeDirection(Actor.Direction);
		ValidDirectionsCount = GetActorValidDirections(Actor, ValidDirections);
		for (uint32_t i = 0; i < ValidDirectionsCount; i++)
		{
			DirectionDistances[i] = CalcNextDistanceToPlayer(Actor, ValidDirections[i]);
		}
		if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
		{
			if (CheckAttackReady())
			{
				PreferredDirection = Actor.Direction;
				MinDistance = 0x7FFFFFF;
				for (uint32_t i = 0; i < ValidDirectionsCount; i++)
				{
					DirectionDistances[i] = CalcDistance(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY);
					if ((DirectionDistances[i] < MinDistance))
					{
						PreferredDirection = ValidDirections[i];
					}
				}
				SetDirection(Actor, PreferredDirection);
			}
			else
			{
				for (uint32_t i = 0; i < ValidDirectionsCount; i++)
				{
					DirectionDistances[i] = CalcDistancePlayerOffset(21, 15, 0);
					if ((DirectionDistances[i] >= 16) && (DirectionDistances[i] <= 25))
					{
						SetDirection(Actor, ValidDirections[i]);
						break;
					}
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				DirectionDistances[i] = CalcDistancePlayerOffset(21, 15, 0);
			}
			MinDistance = 0x7FFFFFFF;
			PreferredDirection = Actor.Direction;
			for (uint32_t i = 0; i < ValidDirectionsCount; i++)
			{
				if ((DirectionDistances[i] < MinDistance) && ((ValidDirections[i] != OppositeDirection) || (Random(255) < 32)))
				{
					PreferredDirection = ValidDirections[i];
					MinDistance = DirectionDistances[i];
				}
			}
			SetDirection(Actor, PreferredDirection);
		}
	}

	void HackerPurpleChase(TActor& Actor)
	{
		if (!Actor.AttackCounter)
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				ActorSpawn(0x1C, Actor.TileX, Actor.TileY);
				Actor.AttackCounter = 60;
				Actor.ReadyCounter = 60;
			}
		}
		MoveActor(Actor);
	}

	void CoderRookieChase(TActor& Actor)
	{
		if (!Actor.AttackCounter)
		{
			if (CheckLineOfSight(Actor.TileX, Actor.TileY, Player.TileX, Player.TileY))
			{
				ActorSpawn(0x1C, Actor.TileX, Actor.TileY);
				Actor.AttackCounter = 60;
				Actor.ReadyCounter = 60;
			}
		}
		MoveActor(Actor);
	}

	void HandleActor(TActor& Actor)
	{
		if (Actor.AttackCounter)
			Actor.AttackCounter--;
		if (Actor.ReadyCounter)
			Actor.ReadyCounter--;
		if (Actor.FrightenedCount > 0)
		{
			Actor.FrightenedCount--;
			if (Actor.FrightenedCount == 0)
				Actor.Flags &= ~cfFrightened;
		}
		if (Actor.RegionLockedCount > 0)
		{
			Actor.RegionLockedCount--;
			if (Actor.RegionLockedCount == 0)
				Actor.Flags &= ~cfRegionLocked;
		}
		if (Actor.Flags & cfFrightened)
			Actor.Speed = ActorClasses[Actor.ActorClass].SpeedFright;
		else
			Actor.Speed = ActorClasses[Actor.ActorClass].SpeedNormal;
		Actor.Accumulator += Actor.Speed;
		while (Actor.Accumulator >= 256)
		{
			if (CanActorMove(Actor))
			{
				if (ActorClasses[Actor.ActorClass].ActorChase)
				{
					ActorClasses[Actor.ActorClass].ActorChase(Actor);
				}
				else
				{
					// Usa codice generico
					MoveActor(Actor);
				}
			}
			else
			{
				ChooseRandomDirection(Actor);
			}
			Actor.Accumulator -= 256;
			if (CanActorChangeDir(Actor))
			{
				if (!(Actor.Flags & cfRegionLocked) && (ActorClasses[Actor.ActorClass].ActorThink))
				{
					ActorClasses[Actor.ActorClass].ActorThink(Actor);
				}
				else
				{
					// Usa codice generico
					ChooseRandomDirection(Actor);
				}
			}
		}
	}

	void HandleActors()
	{
		for (uint32_t i = 0; i < MaxActor; i++)
			if ((Actors[i].ActorClass) && (Actors[i].Flags & cfLiving))
				HandleActor(Actors[i]);
	}

	void SpecialPlayerTerminal(TActor& Item)
	{
		Player.Flags |= cfFrightened;
		Player.FrightenedCount = LevelInfo[InspectingDevice].FrightenedCount;
		for (int i = 0; i < MaxActor; i++)
			if ((Actors[i].ActorClass) && (Actors[i].Flags & cfLiving))
			{
				Actors[i].Flags |= cfFrightened;
				Actors[i].FrightenedCount = LevelInfo[InspectingDevice].FrightenedCount;
			}
	}

	void SpecialPlayerSmartCard(TActor& Item)
	{
		Player.Flags |= cfInvincible;
		Player.InvincibleCount = LevelInfo[InspectingDevice].InvincibleCount;
	}

	void SpecialPlayerSSD(TActor& Item)
	{
		RemainingBackups++;
	}

	void SpecialPlayerPGPKey(TActor& Item)
	{
		for (int i = 0; i < MaxActor; i++)
			if ((Actors[i].ActorClass) && (Actors[i].Flags & cfLiving))
			{
				Actors[i].Flags |= cfRegionLocked;
				Actors[i].RegionLockedCount = LevelInfo[InspectingDevice].RegionLockedCount;
			}
	}

	void SpecialPlayerDisturbanceScript(TActor& Item)
	{
		Player.Flags |= cfDisturbed;
		Player.DisturbedCount = 120;
		//Audio::PlayScriptSfx();
		return;
	}

	void PlayerTouchEvent(TActor& Item)
	{
		if (Item.Flags & cfPickup)
		{
			RecoveredBytes += ActorClasses[Item.ActorClass].RecoveringBytes;
			if (Item.Flags & cfMandatory)
				RemainingPickups--;
			if (ActorClasses[Item.ActorClass].SpecialPlayerTouch)
			{
				ActorClasses[Item.ActorClass].SpecialPlayerTouch(Item);
				Audio::PlayPowerSfx();
			}
			else
				Audio::PlayWakaSfx();
			ActorDestroy(Item);
		}
	}

	void SpecialActorTouchItem(TActor & Actor, TActor & Item)
	{
		if (Item.ActorClass == 0x12)
			ActorInitialize(Item, 0x13);
		if (Item.ActorClass == 0x14)
			ActorInitialize(Item, 0x15);
	}

	void ActorTouchEvent(TActor& Actor, TActor& Item)
	{
		if (ActorClasses[Actor.ActorClass].SpecialActorTouch)
			ActorClasses[Actor.ActorClass].SpecialActorTouch(Actor, Item);
	}

	void HandleCollisions()
	{
		for (int i = 0; i < MaxActor; i++)
		{
			if (Actors[i].ActorClass > 0)
			{
				// Collisioni fra giocatore e items
				if ((Actors[i].TileX == Player.TileX) && (Actors[i].TileY == Player.TileY))
				{
					PlayerTouchEvent(Actors[i]);
					continue;
				}
				if (Actors[i].Flags & cfLiving)
				{
					// Collisioni fra giocatore e attori
					if ((Actors[i].TileX == Player.TileX) && (Actors[i].TileY == Player.TileY))
					{
						if (Actors[i].Flags & cfHurtPlayer)
							PlayerHurted = true;
					}
					// Collisioni fra attori e items
					for (int j = 0; j < MaxActor; j++)
					{
						if (Actors[j].ActorClass > 0)
						{
							if ((Actors[i].TileX == Actors[j].TileX) && (Actors[i].TileY == Actors[j].TileY))
							{
								ActorTouchEvent(Actors[i], Actors[j]);
							}
						}
					}
				}
			}
		}
	}

	void PlayGame()
	{
		char Key;
		TUserInput UserInput;

		InspectingDevice = 0;
		RecoveredBytes = 0;
		RemainingBackups = 1;
		do
		{
			if (LevelInfo[InspectingDevice].BeginningTextScreen[0])
			{
				Video::DrawGameTextScreen(LevelInfo[InspectingDevice].BeginningTextScreen);
				Video::RenderFrame();
				Console::DosEmptyReadKbd();
			}
			if (LevelInfo[InspectingDevice].BeginningSplashScreen[0])
			{
				Video::DrawGameSplashScreen(LevelInfo[InspectingDevice].BeginningSplashScreen);
				Video::RenderFrame();
				Console::DosEmptyReadKbd();
			}
			// Inizializza il livello
			GameTicker = 0;
			PlayerHurted = false;
			LevelInitialize();
			Video::DrawMaze();
			UpdatePanel();
			DrawActor(Player);
			Video::RenderFrame();
			Timer::ResetTimer();
			Audio::PlayBeginningMusic();
			Timer::ResetTimer();
			Key = 0;
			do
			{
				// Aggiorna la logica del logico a 60 Hz
				if (Timer::UserTimerExpired(Timer::GetUserClockInterval(60)))
				{
					GameTicker++;
					// Controlla input
					UserInput = InputNone;
					if (Console::DosIsCharAvail() != 0)
					{
						Key = Console::ReadKey();
						switch (Key)
						{
						case Console::KeyUp:
							UserInput = InputUp;
							break;
						case Console::KeyLeft:
							UserInput = InputLeft;
							break;
						case Console::KeyDown:
							UserInput = InputDown;
							break;
						case Console::KeyRight:
							UserInput = InputRight;
							break;
						case 27:
							Audio::DoneSfx();
							break;
						}
					}
					if (CanPlayerChangeDir(UserInput))
					{
						switch (UserInput)
						{
						case InputUp:
							Player.Direction = MoveUp;
							DiagonalDeltaX = Player.TileX * 8 + 4 - Player.PosX;
							DiagonalDeltaY = 0;
							break;
						case InputLeft:
							Player.Direction = MoveLeft;
							DiagonalDeltaX = 0;
							DiagonalDeltaY = Player.TileY * 8 + 4 - Player.PosY;
							break;
						case InputDown:
							Player.Direction = MoveDown;
							DiagonalDeltaX = Player.TileX * 8 + 4 - Player.PosX;
							DiagonalDeltaY = 0;
							break;
						case InputRight:
							Player.Direction = MoveRight;
							DiagonalDeltaX = 0;
							DiagonalDeltaY = Player.TileY * 8 + 4 - Player.PosY;
							break;
						}
					}
					Video::DrawMaze();
					UpdatePanel();
					HandlePlayer();
					HandleActors();
					HandleCollisions();
					DrawActor(Player);
					DrawActors();
					Video::RenderFrame();
					Audio::UpdateSfx();
					if (PlayerHurted)
					{
						RemainingPickups--;
						Audio::PlayDeathSfx();
						for (int i = 0; i <= 20; i++)
						{
							Audio::UpdateSfx();
							Timer::Delay(6);
						}
						ResetActors();
					}
				}
				else
					_mm_pause();
				if (Key == 27)
					break;
			} while ((RemainingPickups > 0) && (RemainingBackups > 0));
			if (Key == 27)
				break;
			Key = 0;
			Timer::ResetTimer();
			Audio::PlayEndingMusic();
			if (RemainingBackups > 0)
			{
				if (LevelInfo[InspectingDevice].EndingTextScreen[0])
				{
					Video::DrawGameTextScreen(LevelInfo[InspectingDevice].EndingTextScreen);
					Video::RenderFrame();
					Console::DosEmptyReadKbd();
				}
				if (LevelInfo[InspectingDevice].EndingSplashScreen[0])
				{
					Video::DrawGameSplashScreen(LevelInfo[InspectingDevice].EndingSplashScreen);
					Video::RenderFrame();
					Console::DosEmptyReadKbd();
				}
				InspectingDevice++;
			}
		} while ((RemainingBackups > 0) && (InspectingDevice < 20));
	}

	void ShowError(uint16_t LineNumber, const char* AString)
	{
		Console::DrawString(0, LineNumber, "RAM-Runner 32 bit - An error has occured:", Console::LightGray);
		Console::DrawString(0, LineNumber + 1, AString, Console::LightGray);
		Console::GotoXY(0, LineNumber + 2);
	}

	void InitMaze()
	{
		uint32_t CurrentCount;
		uint32_t TileIndex;

		Video::CacheMaze(MazeTiles, MaxTileX - MinTileX + 1, MaxTileY - MinTileY + 1);
		for (uint32_t i = 0; i < MazeSize; i++) 
		{
			TileIndex = MazeTiles[i];
			CurrentCount = MazeRegionsCount[TileIndex];
			MazeRegions[TileIndex][CurrentCount] = i;
			MazeRegionsCount[TileIndex]++;
		}
		for (uint32_t i = 0; i < Video::MaxTile; i++)
			MazeRegionsFirst[i] = MazeSize;
		for (uint32_t i = 0; i < MazeSize; i++)
		{
			TileIndex = MazeTiles[i];
			if (i < MazeRegionsFirst[TileIndex])
				MazeRegionsFirst[TileIndex] = i;
		}
	}

	bool Init()
	{
		Console::ClrScr();
		RandSeed = 123456789;
		Console::DrawString(0, 0, "Checking CPU...", Console::LightGray);
		if (!Processor::IsPentiumOrNewer())
		{
			ShowError(1, "The CPU must be at least a Pentium.");
			return false;
		}
		Console::DrawString(20, 0, "DONE", Console::White);
		Console::DrawString(0, 1, "Checking audio and video...", Console::LightGray);
		Audio::InitMusic();
		Audio::InitSfx();
		if (!Video::VbeExists())
		{
			ShowError(2, "Your video card must support 1024 x 768 x 32 bpp resolution, VBE and LFB.");
			return false;
		}
		Console::DrawString(32, 1, "DONE", Console::White);
		Console::DrawString(0, 2, "Allocating memory...", Console::LightGray);
		if (!Video::AllocateFrameBuffer())
		{
			ShowError(3, "Your available RAM must be at least 8 MB.");
			return false;
		}
		Console::DrawString(25, 2, "DONE", Console::White);
		Console::DrawString(0, 3, "Reading asset...", Console::LightGray);
		if (!Video::LoadAssets())
		{
			Video::DeallocateFrameBuffer();
			ShowError(8, "Could not load RR.DAT.");
			return false;
		}
		Console::DrawString(21, 3, "DONE", Console::White);
		Console::DrawString(0, 8, "Caching the maze...", Console::LightGray);
		Console::DrawString(24, 8, "DONE", Console::White);
		Console::DrawString(0, 9, "Installing timer...", Console::LightGray);
		if (!Timer::InstallTimer())
		{
			Video::UnloadAssets();
			Video::DeallocateFrameBuffer();
			ShowError(10, "Error installing the system timer.");
			return false;
		}
		Console::DrawString(24, 9, "DONE", Console::White);
		if (!Video::EnterGraphicMode())
		{
			Timer::UninstallTimer();
			Video::UnloadAssets();
			Video::DeallocateFrameBuffer();
			ShowError(10, "Error setting graphic mode.");
			return false;
		}
		InitMaze();
		return true;
	}

	bool Done()
	{
		Video::ExitGraphicMode();
		Timer::UninstallTimer();
		Audio::FMReset();
		Video::UnloadAssets();
		Video::DeallocateFrameBuffer();
		return true;
	}

	void ShowHelp()
	{
		uint16_t Selection;
		bool Running;
		char Key;
		Selection = 0;
		Running = true;
		do
		{
			if (Console::DosIsCharAvail())
			{
				Key = Console::ReadKey();
				switch (Key)
				{
				case Console::KeyUp:
				case Console::KeyLeft:
					if (Selection > 0)
						Selection--;
					break;
				case Console::KeyDown:
				case Console::KeyRight:
					if (Selection < 6)
						Selection++;
					break;
				case 27:
					Running = false;
					break;
				}
			}
			switch (Selection)
			{
			case 0:
				Video::DrawHelpOnHelp();
				break;
			case 1:
				Video::DrawHelp0();
				break;
			case 2:
				Video::DrawHelp1();
				break;
			case 3:
				Video::DrawHelp2();
				break;
			case 4:
				Video::DrawHelp3();
				break;
			case 5:
				Video::DrawHelp4();
				break;
			case 6:
				Video::DrawHelp5();
				break;
			}
			Video::RenderFrame();
		} while (Running);
	}

	void Main()
	{
		uint16_t Selection;
		bool NeedDrawMenu;
		bool Running;
		char Key;
		Console::FlushKeyboard();
		if (Init())
		{
			Selection = 0;
			Running = true;
			NeedDrawMenu = true;
			do
			{
				if (NeedDrawMenu)
				{
					Video::DrawMenu();
					Video::DrawString(13, 10, "RAM-RUNNER");
					Video::DrawString(16, 12, "NEW GAME");
					Video::DrawString(16, 14, "HELP");
					Video::DrawString(16, 16, "QUIT TO DOS");
					Video::RenderFrame();
					NeedDrawMenu = false;
				}
				Video::DrawString(13, 12, "  ");
				Video::DrawString(13, 14, "  ");
				Video::DrawString(13, 16, "  ");
				Video::DrawString(13, 12 + Selection * 2, "$>");
				Video::RenderFrame();
				if (Console::DosIsCharAvail())
				{
					Key = Console::ReadKey();
					switch (Key)
					{
					case Console::KeyUp:
					case Console::KeyLeft:
						if (Selection > 0)
							Selection--;
						break;
					case Console::KeyDown:
					case Console::KeyRight:
						if (Selection < 2)
							Selection++;
						break;
					case 27:
						Running = false;
						break;
					case 10:
					case 13:
						switch (Selection)
						{
						case 0:
							PlayGame();
							NeedDrawMenu = true;
							break;
						case 1:
							ShowHelp();
							NeedDrawMenu = true;
							break;
						case 2:
							Running = false;
							break;
						}
						break;
					}
				}
				else
					_mm_pause();
			} while (Running);
			Done();
		}
	}

	extern "C" void __cdecl DosMain()
	{
		RamRunner::Main();
	}
}
