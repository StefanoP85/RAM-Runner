/*
 * ============================================================================
 * Project: RAM-Runner (32-bit DOS Engine)
 * File:    Timer.h
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Game timer at 60 Hz
 * ============================================================================
 */
#pragma once
#include "Proc.h"

namespace RamRunner::Timer
{
	extern "C"
	{
		bool InstallTimer();
		void UninstallTimer();
	}
	uint16_t GetUserClockInterval(uint16_t Hz);
	void ResetTimer();
	void StartTimer();
	void StopTimer();
	bool UserTimerExpired(uint32_t Interval);
	void Delay(uint16_t InverseDuration);
}
