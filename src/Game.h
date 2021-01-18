#pragma once

#include <stdint.h>

//Game types
typedef enum
{
	GameMode_Quit,
	GameMode_Sega,
	GameMode_Title,
	GameMode_Demo,
	GameMode_Level,
	GameMode_Special,
	GameMode_Continue,
	GameMode_Ending,
	GameMode_Credits,
} GameMode;

//Game state
extern GameMode gamemode;

extern uint16_t demo_length;

//Game functions
void EntryPoint();

//Interrupt functions
void VBlank();
void HBlank();
