#include "Game.h"

#include "Video.h"
#include "Palette.h"
#include "LevelScroll.h"

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
	while (1)
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
				VDPSetupGame();
				gamemode = GameMode_Sega;
				break;
		}
	}
}

//Interrupts
void WriteVRAMBuffers()
{
	//Read controllers
	
	//Copy palette
	if (wtr_state)
		VDP_WriteCRAM(0, &wet_palette[0][0], 0x40);
	else
		VDP_WriteCRAM(0, &dry_palette[0][0], 0x40);
	
	//Copy buffers
	VDP_WriteVRAM(VRAM_SPRITES, (const uint8_t*)sprite_buffer, sizeof(sprite_buffer));
	VDP_WriteVRAM(VRAM_HSCROLL, (const uint8_t*)hscroll_buffer, sizeof(hscroll_buffer));
}

void VBlank()
{
	uint8_t routine = vbla_routine;
	if (vbla_routine != 0x00)
	{
		//Set VDP state
		VDP_SetVScroll(vid_scrposy_dup, vid_bgscrposy_dup);
		
		//Set screen state
		vbla_routine = 0x00;
	}
	
	//Run VBlank routine
	switch (routine)
	{
		case 0x02:
			WriteVRAMBuffers();
	//Fallthrough
		case 0x14:
			if (demo_length)
				demo_length--;
			break;
		case 0x04:
			WriteVRAMBuffers();
			LoadTilesAsYouMove_BGOnly();
			if (demo_length)
				demo_length--;
			break;
		case 0x12:
			WriteVRAMBuffers();
			break;
	}
}

void HBlank()
{
	
}
