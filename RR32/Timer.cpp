/*
 * ============================================================================
 * Project: RAM-Runner (32-bit DOS Engine)
 * File:    Timer.cpp
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Game timer at 60 Hz
 * ============================================================================
 */
#include <intrin.h>
#include "Timer.h"

namespace RamRunner::Timer
{
	constexpr uint32_t Clock8253 = 1193182;
	constexpr uint32_t CountDivider = 19886;
	constexpr uint32_t CountInterval = 65536 / CountDivider; // 65536 div 19886 = 3
	constexpr uint32_t Frequency = Clock8253 / CountDivider; // ~60 Hz
	extern "C" volatile uint32_t UserClockCounter;

	uint16_t GetUserClockInterval(uint16_t Hz)
	{
		return Frequency / Hz;
	}

	void ResetTimer()
	{
		UserClockCounter = 0;
	}

	bool UserTimerExpired(uint32_t Interval)
	{
		if (UserClockCounter >= Interval)
		{
			UserClockCounter -= Interval;
			return true;
		}
		return false;
	}

	void Delay(uint16_t Hz)
	{
		if (!Hz)
			return;
		while (!Timer::UserTimerExpired(Timer::GetUserClockInterval(Hz)))
			_mm_pause();
	}
}
