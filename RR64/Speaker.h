/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Speaker.h
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: PC Speaker management
 * ============================================================================
 */
#pragma once
#include "Proc.h"

namespace RamRunner::Audio
{
	extern "C"
	{
		void SpeakerStartSound(uint16_t Frequency);
		void SpeakerStopSound();
	}
}
