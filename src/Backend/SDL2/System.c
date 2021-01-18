#include "SDL.h"

#include "../MegaDrive.h"

#include <stdio.h>

//System interface
int System_Init(const MD_Header *header)
{
	(void)header;
	
	//Initialize SDL2
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("System_Init: %s\n", SDL_GetError());
		return -1;
	}
	
	return 0;
}

void System_Quit()
{
	//Quit SDL2
	SDL_Quit();
}
