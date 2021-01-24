#pragma once

#include <stdint.h>

#include <Backend/Joypad.h>

//Game types
typedef enum
{
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

extern int16_t demo;
extern uint16_t demo_length;
extern uint16_t credits_num;

extern uint8_t credits_cheat;

extern uint8_t jpad_hold2,  jpad_press2;
extern uint8_t jpad1_hold1, jpad1_press1;
extern uint8_t jpad2_hold,  jpad2_press;

//General game functions
void ReadJoypads();

//Entry point
void EntryPoint();

//Interrupt functions
void VBlank();
void HBlank();
