/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Music.cpp
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Music effects
 * ============================================================================
 */
#include "AdLib.h"
#include "Audio.h"
#include "Speaker.h"
#include "Timer.h"

namespace RamRunner::Audio
{
	constexpr uint16_t C3 = 131;
	constexpr uint16_t C3s = 139;
	constexpr uint16_t D3 = 147;
	constexpr uint16_t D3s = 156;
	constexpr uint16_t E3 = 165;
	constexpr uint16_t F3 = 175;
	constexpr uint16_t F3s = 185;
	constexpr uint16_t G3 = 196;
	constexpr uint16_t G3s = 208;
	constexpr uint16_t A3 = 220;
	constexpr uint16_t A3s = 233;
	constexpr uint16_t B3 = 247;
	constexpr uint16_t C4 = 262;
	constexpr uint16_t C4s = 277;
	constexpr uint16_t D4 = 294;
	constexpr uint16_t D4s = 311;
	constexpr uint16_t E4 = 330;
	constexpr uint16_t F4 = 349;
	constexpr uint16_t F4s = 370;
	constexpr uint16_t G4 = 392;
	constexpr uint16_t G4s = 415;
	constexpr uint16_t A4 = 440;
	constexpr uint16_t A4s = 466;
	constexpr uint16_t B4 = 494;
	constexpr uint16_t C5 = 523;
	constexpr uint16_t C5s = 554;
	constexpr uint16_t D5 = 587;
	constexpr uint16_t D5s = 622;
	constexpr uint16_t E5 = 659;
	constexpr uint16_t F5 = 698;
	constexpr uint16_t F5s = 740;
	constexpr uint16_t G5 = 784;
	constexpr uint16_t G5s = 831;
	constexpr uint16_t A5 = 880;
	constexpr uint16_t A5s = 932;
	constexpr uint16_t B5 = 988;
	constexpr uint16_t C6 = 1047;
	constexpr uint16_t C6s = 1109;
	constexpr uint16_t D6 = 1175;
	constexpr uint16_t D6s = 1245;
	constexpr uint16_t E6 = 1319;
	constexpr uint16_t F6 = 1397;
	constexpr uint16_t F6s = 1480;
	constexpr uint16_t G6 = 1568;
	constexpr uint16_t G6s = 1661;
	constexpr uint16_t A6 = 1760;
	constexpr uint16_t A6s = 1865;
	constexpr uint16_t B6 = 1976;
	constexpr TFMInstrument MusicInstrument
	{
		.SoundCharacteristic = 0x4261,
		.Level = 0x0000,
		.AttackDecay = 0xD0E0,
		.SustainRelease = 0xDEFE,
		.WaveSelect = 0x1101,
		.FeedBack = 0x01
	};
	constexpr TFMInstrument SfxInstrument
	{
		.SoundCharacteristic = 0x0121,
		.Level = 0x0000,
		.AttackDecay = 0xF1F1,
		.SustainRelease = 0xF2F2,
		.WaveSelect = 0x0000,
		.FeedBack = 0x00
	};
	TSoundDevice SoundDevice;
	TSfxType CurrentSfx;
	uint16_t SfxStep;
	bool WakaState;

	void InitMusic()
	{
		if (AdlibExists())
		{
			SoundDevice = AdlibCard;
			FMReset();
			FMSetVoice(0, MusicInstrument);
			FMSetVoice(1, MusicInstrument);
		}
		else
			SoundDevice = PCSpeaker;
	}

	void DoneMusic()
	{
		FMReset();
	}

	void PlayNoteSpeaker(uint16_t Frequency, uint16_t InverseDuration)
	{
		SpeakerStartSound(Frequency);
		Timer::Delay(InverseDuration);
		SpeakerStopSound();
		Timer::Delay(InverseDuration * 10);
	}

	void PlayNote(uint8_t Channel, uint16_t Frequency)
	{
		uint16_t OutOctave;
		uint16_t OutFrequency;

		OutFrequency = Frequency * 2 / 3;
		OutOctave = 5;
		while (OutFrequency > 2047)
		{
			OutFrequency += 2;
			OutOctave++;
		}
		FMKeyOn(Channel, OutFrequency, OutOctave);
	}

	void PlayNoteDual(uint16_t Freq0, uint16_t Freq1, uint16_t InverseDuration)
	{
		if (SoundDevice == AdlibCard)
		{
			// Riproduce la nota principale sul Canale 0
			PlayNote(0, Freq0);

			// Riproduce il basso sul Canale 1 (se presente)
			if (Freq1 > 0)
				PlayNote(1, Freq1);
			else
				FMKeyOff(1);

			// Attende la durata della nota */
			Timer::Delay(InverseDuration);

			// Stacca le note(Key Off) per dare staccato / ritmo
			FMKeyOff(0);
			FMKeyOff(1);

			// Pausa tra una nota e l'altra per l'articolazione
			Timer::Delay(InverseDuration * 10);
		}
		else
		{
			// Fallback PC Speaker : riproduce solo la melodia principale(Canale 0)
			PlayNoteSpeaker(Freq0, InverseDuration);
		}
	}

	void PlayBeginningMusic()
	{
		// --- BATTUTA 1
		Timer::Delay(1);
		PlayNoteDual(A3, A3 >> 2, 12);
		PlayNoteDual(C4, A3 >> 2, 12);
		PlayNoteDual(E4, A3 >> 2, 12);
		PlayNoteDual(A4, A3 >> 1, 12);
		PlayNoteDual(A3, A3 >> 2, 12);
		PlayNoteDual(C4, A3 >> 2, 12);
		PlayNoteDual(E4, A3 >> 2, 12);
		PlayNoteDual(A4, A3 >> 2, 12);
		PlayNoteDual(A3, A3 >> 2, 12);
		PlayNoteDual(C4, A3 >> 2, 12);
		PlayNoteDual(E4, A3 >> 2, 12);
		PlayNoteDual(A4, A3 >> 1, 12);
		PlayNoteDual(A3, A3 >> 2, 12);
		PlayNoteDual(C4, A3 >> 2, 12);
		PlayNoteDual(E4, A3 >> 2, 12);
		PlayNoteDual(A4, A3 >> 2, 12);

		// --- BATTUTA 2
		PlayNoteDual(F3, F3 >> 2, 12);
		PlayNoteDual(A3, F3 >> 2, 12);
		PlayNoteDual(C4, F3 >> 2, 12);
		PlayNoteDual(F4, F3 >> 1, 12);
		PlayNoteDual(F3, F3 >> 2, 12);
		PlayNoteDual(A3, F3 >> 2, 12);
		PlayNoteDual(C4, F3 >> 2, 12);
		PlayNoteDual(F4, F3 >> 2, 12);
		PlayNoteDual(F3, F3 >> 2, 12);
		PlayNoteDual(A3, F3 >> 2, 12);
		PlayNoteDual(C4, F3 >> 2, 12);
		PlayNoteDual(F4, F3 >> 1, 12);
		PlayNoteDual(F3, F3 >> 2, 12);
		PlayNoteDual(A3, F3 >> 2, 12);
		PlayNoteDual(C4, F3 >> 2, 12);
		PlayNoteDual(F4, F3 >> 2, 12);

		// --- BATTUTA 3
		PlayNoteDual(C3, C3 >> 1, 12);
		PlayNoteDual(E3, C3 >> 1, 12);
		PlayNoteDual(G3, C3 >> 1, 12);
		PlayNoteDual(C4, C3, 12);
		PlayNoteDual(C3, C3 >> 1, 12);
		PlayNoteDual(E3, C3 >> 1, 12);
		PlayNoteDual(G3, C3 >> 1, 12);
		PlayNoteDual(C4, C3 >> 1, 12);
		PlayNoteDual(C3, C3 >> 1, 12);
		PlayNoteDual(E3, C3 >> 1, 12);
		PlayNoteDual(G3, C3 >> 1, 12);
		PlayNoteDual(C4, C3, 12);
		PlayNoteDual(C3, C3 >> 1, 12);
		PlayNoteDual(E3, C3 >> 1, 12);
		PlayNoteDual(G3, C3 >> 1, 12);
		PlayNoteDual(C4, C3 >> 1, 12);

		// --- BATTUTA 4
		PlayNoteDual(G3, G3 >> 2, 12);
		PlayNoteDual(B3, G3 >> 2, 12);
		PlayNoteDual(D4, G3 >> 2, 12);
		PlayNoteDual(G4, G3 >> 1, 12);
		PlayNoteDual(G3, G3 >> 2, 12);
		PlayNoteDual(B3, G3 >> 2, 12);
		PlayNoteDual(D4, G3 >> 2, 12);
		PlayNoteDual(G4, G3 >> 2, 12);
		PlayNoteDual(G3, G3 >> 2, 12);
		PlayNoteDual(B3, G3 >> 2, 12);
		PlayNoteDual(D4, G3 >> 2, 12);
		PlayNoteDual(G4, G3 >> 1, 12);
		PlayNoteDual(G3, G3 >> 2, 12);
		PlayNoteDual(B3, G3 >> 2, 12);
		PlayNoteDual(D4, G3 >> 2, 12);
		PlayNoteDual(G4, G3 >> 2, 12);

		// Spegne tutti i canali alla fine
		if (SoundDevice == AdlibCard)
		{
			FMKeyOff(0);
			FMKeyOff(1);
		}
		else
		{
			SpeakerStopSound();
		}
	}

	void PlayEndingMusic()
	{
		// --- BATTUTA 1
		Timer::Delay(1);
		PlayNoteDual(D3, D3 >> 1, 12);
		PlayNoteDual(F3, D3 >> 1, 12);
		PlayNoteDual(A3, D3 >> 1, 12);
		PlayNoteDual(D4, D3, 12);
		PlayNoteDual(D3, D3 >> 1, 12);
		PlayNoteDual(F3, D3 >> 1, 12);
		PlayNoteDual(A3, D3 >> 1, 12);
		PlayNoteDual(D4, D3 >> 1, 12);
		PlayNoteDual(D3, D3 >> 1, 12);
		PlayNoteDual(F3, D3 >> 1, 12);
		PlayNoteDual(A3, D3 >> 1, 12);
		PlayNoteDual(D4, D3, 12);
		PlayNoteDual(D3, D3 >> 1, 12);
		PlayNoteDual(F3, D3 >> 1, 12);
		PlayNoteDual(A3, D3 >> 1, 12);
		PlayNoteDual(D4, D3 >> 1, 12);

		// --- BATTUTA 2
		PlayNoteDual(E3, E3 >> 2, 12);
		PlayNoteDual(G3s, E3 >> 2, 12);
		PlayNoteDual(B3, E3 >> 2, 12);
		PlayNoteDual(E4, E3 >> 1, 12);
		PlayNoteDual(E3, E3 >> 2, 12);
		PlayNoteDual(G3s, E3 >> 2, 12);
		PlayNoteDual(B3, E3 >> 2, 12);
		PlayNoteDual(E4, E3 >> 2, 12);
		PlayNoteDual(E3, E3 >> 2, 12);
		PlayNoteDual(G3s, E3 >> 2, 12);
		PlayNoteDual(B3, E3 >> 2, 12);
		PlayNoteDual(E4, E3 >> 2, 12);
		PlayNoteDual(E3, E3 >> 2, 12);
		PlayNoteDual(G3s, E3 >> 2, 12);
		PlayNoteDual(B3, E3 >> 2, 12);
		PlayNoteDual(E4, E3 >> 1, 12);
		PlayNoteDual(E3, E3 >> 2, 12);
		PlayNoteDual(G3s, E3 >> 2, 12);
		PlayNoteDual(B3, E3 >> 2, 12);
		PlayNoteDual(E4, E3 >> 2, 12);

		// Spegne tutti i canali alla fine
		if (SoundDevice == AdlibCard)
		{
			FMKeyOff(0);
			FMKeyOff(1);
		}
		else
		{
			SpeakerStopSound();
		}
	}

	void InitSfx()
	{
		FMSetVoice(8, SfxInstrument); // Usa il canale 8 dedicato agli Sfx
	}

	void DoneSfx()
	{
		FMKeyOff(8);
		CurrentSfx = SfxNone;
		SfxStep = 0;
	}

	void PlayWakaSfx()
	{
		// Gli effetti importanti fermano o impediscono il Waka
		if ((CurrentSfx == SfxDeath) || (CurrentSfx == SfxPower) || (CurrentSfx == SfxScript))
			return;

		CurrentSfx = SfxWaka;
		SfxStep = 0;
		WakaState = !WakaState;
	}

	void PlayPowerSfx()
	{
		if (CurrentSfx == SfxDeath)
			return;

		CurrentSfx = SfxPower;
		SfxStep = 0;
	}

	void PlayScriptSfx()
	{
		if (CurrentSfx == SfxDeath)
			return;

		CurrentSfx = SfxScript;
		SfxStep = 0;
	}

	void PlayDeathSfx()
	{
		CurrentSfx = SfxDeath;
		SfxStep = 0;
	}

	void UpdateSfx()
	{
		uint16_t Freq;
		if (CurrentSfx == SfxNone)
			return;

		switch (CurrentSfx)
		{
		case SfxWaka:
			switch (SfxStep)
			{
			case 0:
				if (WakaState)
					Freq = 280;
				else
					Freq = 420;
				FMKeyOn(8, Freq, 4);
				break;
			case 1:
				if (WakaState)
					Freq = 350;
				else
					Freq = 490;
				FMKeyOn(8, Freq, 4);
				break;
			case 2:
			case 3:
				break; // Pausa breve
			case 4:
				DoneSfx();
				break;
			}
			SfxStep++;
			break;

		case SfxPower:
			// Effetto arpeggio allegro di 4 note ascendenti
			switch (SfxStep)
			{
			case 0:
				FMKeyOn(8, C5, 4);
				break;
			case 1:
				FMKeyOn(8, E5, 4);
				break;
			case 2:
				FMKeyOn(8, G5, 4);
				break;
			case 3:
				FMKeyOn(8, C6, 5);
				break;
			case 4:
			case 5:
				break; // Risonanza finale
			case 6:
				DoneSfx();
			}
			SfxStep++;
			break;

		case SfxScript:
			// Effetto glissando rapido verso l'alto + trillo */
			switch (SfxStep)
			{
			case 0:
				FMKeyOn(8, 300, 4);
				break;
			case 1:
				FMKeyOn(8, 500, 4);
				break;
			case 2:
				FMKeyOn(8, 800, 5);
				break;
			case 3:
				FMKeyOn(8, 1200, 5);
				break;
			case 4:
				FMKeyOn(8, 1600, 5);
				break;
			case 5:
				FMKeyOn(8, 1400, 5);
				break;
			case 6:
				FMKeyOn(8, 1800, 5);
				break;
			case 7:
			case 8:
				break;
			case 9:
				DoneSfx();
				break;
			}
			SfxStep++;
			break;

		case SfxDeath:
			if (SfxStep < 20)
			{
				Freq = 1200 - (SfxStep * 55);
				FMKeyOn(8, Freq, 4);
			}
			else
			{
				if (SfxStep == 20)
					FMKeyOn(8, 150, 3);
				else
					DoneSfx();
			}
			SfxStep++;
			break;
		}
	}
}
