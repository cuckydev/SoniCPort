#include "Game.h"
#include "Video.h"

#include "Video.h"

#include "GM_Sega.h"
#include "GM_Title.h"

//Game
GameMode gamemode;

uint16_t demo_length;

//Game entry point
void EntryPoint()
{
	//Initialize game system
	VDPSetupGame();
	
	//Initialize game state
	gamemode = GameMode_Sega;
	
	//Run game loop
	while (gamemode != GameMode_Quit)
	{
		switch (gamemode)
		{
			case GameMode_Sega:
				if (GM_Sega())
					return;
				break;
			case GameMode_Title:
				if (GM_Title())
					return;
				break;
			default:
				gamemode = GameMode_Quit;
				break;
		}
	}
}

//Interrupts
void VBlank()
{
	uint8_t routine = vbla_routine;
	if (vbla_routine != 0x00)
	{
		//Set VDP state
		VDP_SetVScroll(scrposy_dup, bgscrposy_dup);
		
		//Set screen state
		vbla_routine = 0x00;
	}
	
	//Run VBlank routine
	switch (routine)
	{
		case 0x02:
			VDPSetupFrame();
	//Fallthrough
		case 0x14:
			if (demo_length)
				demo_length--;
			break;
		case 0x12:
			VDPSetupFrame();
			break;
	}
}

void HBlank()
{
	
}
