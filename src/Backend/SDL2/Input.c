#include "SDL.h"

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
