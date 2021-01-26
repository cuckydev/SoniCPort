#include "Game.h"

#include "Video.h"
#include "Palette.h"
#include "LevelScroll.h"
#include "Level.h"
#include "Object/Sonic.h"
#include "PLC.h"

#include "GM_Sega.h"
#include "GM_Title.h"
#include "GM_Level.h"

//Game
uint8_t gamemode; //MSB acts as a title card flag

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
	jpad1_press1 = state & ~jpad1_hold1;
	jpad1_hold1 = state;
	
	//Read joypad 2
	state = Joypad_GetState2();
	jpad2_press = state & ~jpad2_hold;
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
		switch (gamemode & 0x7F)
		{
			case GameMode_Sega:
				GM_Sega();
				break;
			case GameMode_Title:
				GM_Title();
				break;
			case GameMode_Level:
			case GameMode_Demo:
				GM_Level();
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
		case 0x0C:
			//Read joypad state
			ReadJoypads();
			
			//Copy palette
			if (wtr_state)
				VDP_WriteCRAM(0, &wet_palette[0][0], 0x40);
			else
				VDP_WriteCRAM(0, &dry_palette[0][0], 0x40);
			
			//Copy buffers
			VDP_SetHIntPosition(hbla_pos);
			VDP_WriteVRAM(VRAM_SPRITES, (const uint8_t*)sprite_buffer, sizeof(sprite_buffer));
			VDP_WriteVRAM(VRAM_HSCROLL, (const uint8_t*)hscroll_buffer, sizeof(hscroll_buffer));
			
			//Copy duplicate plane positions and flags
			scrpos_x_dup.v     = scrpos_x.v;
			scrpos_y_dup.v     = scrpos_y.v;
			bg_scrpos_x_dup.v  = bg_scrpos_x.v;
			bg_scrpos_y_dup.v  = bg_scrpos_y.v;
			bg2_scrpos_x_dup.v = bg2_scrpos_x.v;
			bg2_scrpos_y_dup.v = bg2_scrpos_y.v;
			bg3_scrpos_x_dup.v = bg3_scrpos_x.v;
			bg3_scrpos_y_dup.v = bg3_scrpos_y.v;
			
			fg_scroll_flags_dup = fg_scroll_flags;
			bg1_scroll_flags_dup = bg1_scroll_flags;
			bg2_scroll_flags_dup = bg2_scroll_flags;
			bg3_scroll_flags_dup = bg3_scroll_flags;
			
			//Update Sonic's art
			if (sonframe_chg)
			{
				//TODO: Write Sonic DPLC
				sonframe_chg = false;
			}
			
			//Update level animations and HUD
			
			//Handle PLC stuff
			RunPLC_VBlank();
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
