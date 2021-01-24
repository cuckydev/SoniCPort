#include "Game.h"

#include "Video.h"
#include "Palette.h"
#include "LevelScroll.h"

#include "GM_Sega.h"
#include "GM_Title.h"

//Game
GameMode gamemode;

int16_t demo;
uint16_t demo_length;
uint16_t credits_num;

uint8_t credits_cheat;

uint8_t jpad_hold2,  jpad_press2; //Joypad 2 state
uint8_t jpad1_hold1, jpad1_press1; //Joypad 1 state
uint8_t jpad2_hold,  jpad2_press; //Sonic controls

uint32_t vbla_count;

//General game functions
void ReadJoypads()
{
	uint8_t state;
	
	//Read joypad 1
	state = Joypad_GetState1();
	jpad1_press1 = (state ^ jpad1_hold1) & state;
	jpad1_hold1 = state;
	
	//Read joypad 2
	state = Joypad_GetState2();
	jpad2_press = (state ^ jpad2_hold) & state;
	jpad2_hold = state;
}

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
				GM_Sega();
				break;
			case GameMode_Title:
				GM_Title();
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
	//Read joypad state
	ReadJoypads();
	
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
		VDP_SetVScroll(vid_scrpos_y_dup, vid_bg_scrpos_y_dup);
		
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
	
	//Update music
	
	//Increment VBlank counter
	vbla_count++;
}

void HBlank()
{
	
}
