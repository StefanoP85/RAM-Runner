/*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    Console.h
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Keyboard and text mode screen handling
 * ============================================================================
 */
#pragma once
#include "Proc.h"

namespace RamRunner::Console
{
	constexpr uint8_t KeyUp = 72;
	constexpr uint8_t KeyDown = 80;
	constexpr uint8_t KeyLeft = 75;
	constexpr uint8_t KeyRight = 77;
	constexpr uint8_t Black = 0;
	constexpr uint8_t Blue = 1;
	constexpr uint8_t Green = 2;
	constexpr uint8_t Cyan = 3;
	constexpr uint8_t Red = 4;
	constexpr uint8_t Magenta = 5;
	constexpr uint8_t Brown = 6;
	constexpr uint8_t LightGray = 7;
	constexpr uint8_t DarkGray = 8;
	constexpr uint8_t LightBlue = 9;
	constexpr uint8_t LightGreen = 10;
	constexpr uint8_t LightCyan = 11;
	constexpr uint8_t LightRed = 12;
	constexpr uint8_t LightMagenta = 13;
	constexpr uint8_t Yellow = 14;
	constexpr uint8_t White = 15;
	constexpr uint8_t Blink = 128;
	uint8_t DosEmptyReadKbd();
	uint8_t DosIsCharAvail();
	void GotoXY(uint8_t X, uint8_t Y);
	void ClrScr();
	void TextMode80x25();
	uint8_t ReadKey();
	void FlushKeyboard();
	void DrawChar(uint16_t X, uint16_t Y, char Code, char Attribute);
	void DrawString(uint16_t X, uint16_t Y, const char* AString, uint8_t Attribute);
}
