/*
 * ============================================================================
 * Project: RAM-Runner (32-bit DOS Engine)
 * File:    Audio.h
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: AdLib and Sound Blaster handling
 * ============================================================================
 */
#pragma once
#include "Proc.h"

namespace RamRunner::Audio
{
	struct TFMInstrument
	{
		uint16_t SoundCharacteristic;
		uint16_t Level;
		uint16_t AttackDecay;
		uint16_t SustainRelease;
		uint16_t WaveSelect;
		uint8_t FeedBack;
	};

	extern "C"
	{
		uint8_t ReadAdLib();
		void WriteAdLib(uint8_t Reg, uint8_t Value);
	}
	bool AdlibExists();
	void FMReset();
	void FMSetVoice(uint8_t Channel, const TFMInstrument& Inst);
	void FMKeyOn(uint8_t Channel, uint16_t FFreq, uint16_t FOctave);
	void FMKeyOff(uint8_t Channel);
}
