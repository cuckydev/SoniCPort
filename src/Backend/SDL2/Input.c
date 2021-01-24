#include "SDL.h"

#include <Backend/Joypad.h>

//Backend input interface
int Input_HandleEvents()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT:
				return 1;
			default:
				break;
		}
	}
	return 0;
}

uint8_t Input_GetState1()
{
	//Get keyboard state
	const uint8_t *key_state = SDL_GetKeyboardState(NULL);
	uint8_t start = key_state[SDL_SCANCODE_RETURN] ? JPAD_START : 0;
	uint8_t a     = key_state[SDL_SCANCODE_A]      ? JPAD_A     : 0;
	uint8_t b     = key_state[SDL_SCANCODE_S]      ? JPAD_B     : 0;
	uint8_t c     = key_state[SDL_SCANCODE_D]      ? JPAD_C     : 0;
	uint8_t right = key_state[SDL_SCANCODE_RIGHT]  ? JPAD_RIGHT : 0;
	uint8_t left  = key_state[SDL_SCANCODE_LEFT]   ? JPAD_LEFT  : 0;
	uint8_t down  = key_state[SDL_SCANCODE_DOWN]   ? JPAD_DOWN  : 0;
	uint8_t up    = key_state[SDL_SCANCODE_UP]     ? JPAD_UP    : 0;
	
	//Return as bitfield
	return start | a | c | b | right | left | down | up;
}

uint8_t Input_GetState2()
{
	//No use in Sonic 1
	return 0;
}
