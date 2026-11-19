/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    RR.h
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Main source for the game logic
 * ============================================================================
 */
#pragma once
#include "Proc.h"

namespace RamRunner
{
	constexpr uint32_t MinTileX = 0;
	constexpr uint32_t MaxTileX = 27;
	constexpr uint32_t MinTileY = 0;
	constexpr uint32_t MaxTileY = 29;
	constexpr uint32_t MazeSize = (MaxTileY - MinTileY + 1) * (MaxTileX - MinTileX + 1);
	constexpr uint32_t MaxLevelInfo = 20;
	constexpr uint32_t MaxActor = 256;
	struct TLevelInfo
	{
		uint32_t LevelNumber;
		char BeginningSplashScreen[24];
		char BeginningTextScreen[24];
		char EndingSplashScreen[24];
		char EndingTextScreen[24];
		bool SpawnHigh;
		bool SpawnMedium;
		bool SpawnLow;
		uint32_t NumFileFragments;
		uint32_t NumFiles;
		uint32_t NumFloppys;
		uint32_t NumCloudFiles;
		uint32_t NumJunkFragments;
		uint32_t NumLockedFiles;
		uint32_t NumLockedCloudFiles;
		uint32_t NumEncryptedUSB;
		uint32_t NumTerminalShells;
		uint32_t NumSmartCards;
		uint32_t NumSSDs;
		uint32_t NumPGPKeys;
		uint32_t SpeedPlayer;
		uint32_t SpeedHigh;
		uint32_t SpeedMedium;
		uint32_t SpeedLow;
		uint32_t SpeedPlayerFright;
		uint32_t SpeedHighFright;
		uint32_t SpeedMediumFright;
		uint32_t SpeedLowFright;
		uint32_t FrightenedCount;
		uint32_t RegionLockedCount;
		uint32_t InvincibleCount;
	};
	using PLevelInfo = TLevelInfo*;
	using TMazeTiles = uint8_t[MazeSize];
	enum TUserInput : uint32_t
	{
		InputNone,
		InputUp,
		InputLeft,
		InputDown,
		InputRight
	};
	enum TDirection : uint32_t
	{
		MoveUp = 0,
		MoveLeft = 1,
		MoveDown = 2,
		MoveRight = 3
	};
	enum TActorClassFlags : uint32_t
	{
		cfNone = 0,
		cfPickup = 1 << 0,
		cfMandatory = 1 << 1,
		cfMandatoryPickup = cfPickup | cfMandatory,
		cfLiving = 1 << 2,
		cfHurtPlayer = 1 << 3,
		cfLivingDanger = cfLiving | cfHurtPlayer,
		cfFrightened = 1 << 4,
		cfRegionLocked = 1 << 5,
		cfInvincible = 1 << 6,
		cfDisturbed = 1 << 7,
	};
	enum TActorClassTier
	{
		ctNone = 0,
		ctLow = 1,
		ctMedium = 2,
		ctHigh = 3
	};
	struct TActor
	{
		uint32_t ActorClass;
		uint32_t Flags;
		int32_t PosX, PosY; // Posizione in pixel
		int32_t TileX, TileY; // Tile occupata
		TDirection Direction; // Direzione di movimento
		int32_t Speed, Accumulator; // Gestione della velocita'
		int32_t FrightenedCount;
		int32_t InvincibleCount;
		int32_t RegionLockedCount;
		int32_t DisturbedCount;
		int32_t ChangeDirCounter; // Gestione del timeout di cambio direzione
		int32_t AttackCounter; // Gestione del timeout di attacco
		int32_t ReadyCounter; // Gestione del timeout di attacco organizzato
	};
	using TSpecialPlayerTouch = void(*)(TActor& Item);
	using TSpecialActorTouch = void(*)(TActor& Actor, TActor& Item);
	using TActorThink = void(*)(TActor& Item);
	using TActorChase = void(*)(TActor& Item);
	struct TActorClass
	{
		TActorClassFlags Flags;
		TActorClassTier Tier;
		uint32_t BaseSprite; // Primo sprite
		uint32_t NumFrames; // Numero di frame di animazione
		bool Directional; // Se true, allora ha 1 sprite per direzione
		uint32_t RecoveringBytes; // Punteggio da aggiungere
		uint32_t SpeedNormal;
		uint32_t SpeedFright;
		TSpecialPlayerTouch SpecialPlayerTouch;
		TSpecialActorTouch SpecialActorTouch;
		TActorThink ActorThink;
		TActorChase ActorChase;
	};

	void SpecialPlayerTerminal(TActor& Item);
	void SpecialPlayerSmartCard(TActor& Item);
	void SpecialPlayerSSD(TActor& Item);
	void SpecialPlayerPGPKey(TActor& Item);
	void SpecialPlayerDisturbanceScript(TActor& Item);
	void PlayerTouchEvent(TActor& Item);
	void SpecialActorTouchItem(TActor& Actor, TActor& Item);
	void LoudBasementStreamerThink(TActor& Actor);
	void CasualCoffeShopDevThink(TActor& Actor);
	void SnakeOilSalesmanThink(TActor& Actor);
	void ClumsyDigitalSquatterThink(TActor& Actor);
	void ScriptKiddieThink(TActor& Actor);
	void ScriptKiddieChase(TActor& Actor);
	void RogueSaboteurThink(TActor& Actor);
	void RogueSaboteurChase(TActor& Actor);
	void CoderRebelThink(TActor& Actor);
	void CoderRebelChase(TActor& Actor);
	void CoderRookieThink(TActor& Actor);
	void CoderRookieChase(TActor& Actor);
	void HackerRedThink(TActor& Actor);
	void HackerRedChase(TActor& Actor);
	void HackerPinkThink(TActor& Actor);
	void HackerPinkChase(TActor& Actor);
	void HackerBlueThink(TActor& Actor);
	void HackerBlueChase(TActor& Actor);
	void HackerOrangeThink(TActor& Actor);
	void HackerOrangeChase(TActor& Actor);
	void HackerPurpleThink(TActor& Actor);
	void HackerPurpleChase(TActor& Actor);

	extern "C" void __cdecl DosMain();
}
