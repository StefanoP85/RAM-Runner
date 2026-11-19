/*
 * ============================================================================
 * Project: RAM-Runner (32-bit DOS Engine)
 * File:    Music.h
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Music and sound effects
 * ============================================================================
 */
#pragma once

namespace RamRunner::Audio
{
	enum TSoundDevice { PCSpeaker, AdlibCard };
	enum TSfxType { SfxNone, SfxWaka, SfxPower, SfxScript, SfxDeath };

	void InitMusic();
	void DoneMusic();
	void PlayBeginningMusic();
	void PlayEndingMusic();

	void InitSfx();
	void DoneSfx();
	void PlayWakaSfx();
	void PlayPowerSfx();
	void PlayScriptSfx();
	void PlayDeathSfx();
	void DoneSfx();
	void UpdateSfx(); // Da chiamare nel loop principale di gioco
}
