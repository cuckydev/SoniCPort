#include "Joypad.h"

//Backend input interface
uint8_t Input_GetState1();
uint8_t Input_GetState2();

//Joypad information
uint8_t Joypad_GetState1()
{
	return Input_GetState1();
}

uint8_t Joypad_GetState2()
{
	return Input_GetState2();
}
