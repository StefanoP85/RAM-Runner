/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Audio.cpp
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: AdLib and Sound Blaster handling
 * ============================================================================
 */
#include "AdLib.h"

namespace RamRunner::Audio
{
	// Offset dei registri AdLib per gli operatori della portante (Carrier) e modulatore (Modulator)
	const uint8_t ModOffset[9] = { 0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12 };
	const uint8_t CarOffset[9] = { 0x03, 0x04, 0x05, 0x0B, 0x0C, 0x0D, 0x13, 0x14, 0x15 };

	// Reset completo di tutti i registri OPL2
	void FMReset()
	{
		for (int i = 1; i <= 0xF5; i++)
			WriteAdLib(i, 0);

		WriteAdLib(0x01, 0x20); // Abilita Waveform Select
	}

	// Rilevazione HW AdLib standard tramite Timer OPL2
	bool AdlibExists()
	{
		uint8_t Status1, Status2;

		// Reset dei Timer
		WriteAdLib(0x04, 0x60);
		WriteAdLib(0x04, 0x80);
		Status1 = ReadAdLib();

		// Avvia Timer 1 con valore $FF
		WriteAdLib(0x02, 0xFF);
		WriteAdLib(0x04, 0x21);

		// Attesa calibrata indipendentemente dai cicli CPU?
		for (int i = 1; i <= 2000; i++)
			Status2 = ReadAdLib();
		Status2 = ReadAdLib();

		// Reset finale
		WriteAdLib(0x04, 0x60);
		WriteAdLib(0x04, 0x80);

		// Verifica dei flag del timer
		return ((Status1 & 0xE0) == 0x00) && ((Status2 & 0xC0) == 0xC0);
	}

	uint8_t Hi(uint16_t Value)
	{
		return (Value & 0xFF00) >> 8;
	}

	uint8_t Lo(uint16_t Value)
	{
		return Value & 0x00FF;
	}

	// Carica lo strumento FM sul canale specificato(0..8)
	void FMSetVoice(uint8_t Channel, const TFMInstrument& Inst)
	{
		uint8_t ModOp, CarOp;

		if (Channel > 8)
			return;

		ModOp = ModOffset[Channel];
		CarOp = CarOffset[Channel];

		// Carica i valori ADSR, inviluppo e forma d'onda dal record TFMInstrument
		WriteAdLib(0x20 + ModOp, Hi(Inst.SoundCharacteristic));
		WriteAdLib(0x20 + CarOp, Lo(Inst.SoundCharacteristic));

		WriteAdLib(0x40 + ModOp, Hi(Inst.Level));
		WriteAdLib(0x40 + CarOp, Lo(Inst.Level));

		WriteAdLib(0x60 + ModOp, Hi(Inst.AttackDecay));
		WriteAdLib(0x60 + CarOp, Lo(Inst.AttackDecay));

		WriteAdLib(0x80 + ModOp, Hi(Inst.SustainRelease));
		WriteAdLib(0x80 + CarOp, Lo(Inst.SustainRelease));

		WriteAdLib(0xE0 + ModOp, Hi(Inst.WaveSelect));
		WriteAdLib(0xE0 + CarOp, Lo(Inst.WaveSelect));

		WriteAdLib(0xC0 + Channel, Inst.FeedBack);
	}

	// Attiva la nota su uno specifico canale AdLib
	void FMKeyOn(uint8_t Channel, uint16_t FFreq, uint16_t FOctave)
	{
		uint16_t FNum;
		uint8_t RegB0;

		if (Channel > 8)
			return;

		// FFreq deve rimanere nel range 0..1023 per l'OPL2
		FNum = FFreq & 0x03FF;

		// Registro A0 : Byte Basso della frequenza
		WriteAdLib(0xA0 + Channel, Lo(FNum));

		// Registro B0 : Bit 5 = KeyOn, Bit 2..4 = Ottava(0..7), Bit 0..1 = Byte Alto Frequenza
		RegB0 = 0x20 | ((FOctave & 0x07) << 2) | (Hi(FNum) & 0x03);
		WriteAdLib(0xB0 + Channel, RegB0);
	}

	// Spegne la nota sul canale
	void FMKeyOff(uint8_t Channel)
	{
		if (Channel > 8)
			return;

		WriteAdLib(0xB0 + Channel, 0);
	}
}
