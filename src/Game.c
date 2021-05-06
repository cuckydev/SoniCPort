#include "Game.h"

#include "Video.h"
#include "Palette.h"
#include "PaletteCycle.h"
#include "Level.h"
#include "LevelDraw.h"
#include "LevelScroll.h"
#include "Object/Sonic.h"
#include "PLC.h"
#include "HUD.h"

#include "GM_Sega.h"
#include "GM_Title.h"
#include "GM_Level.h"
#include "GM_Special.h"
#ifdef SCP_SPLASH
	#include "GM_SSRG.h"
#endif

//Game
ALIGNED4 uint8_t buffer0000[0xA400];

uint8_t gamemode; //MSB acts as a title card flag

int16_t demo;
uint16_t demo_length;
uint16_t credits_num;

uint8_t credits_cheat;

uint8_t debug_cheat, debug_mode;

uint8_t jpad2_hold,  jpad2_press; //Joypad 2 state
uint8_t jpad1_hold1, jpad1_press1; //Joypad 1 state
uint8_t jpad1_hold2, jpad1_press2; //Sonic controls

uint32_t vbla_count;

//Global assets
const uint8_t art_text[] = {
	#include "Resource/Art/Text.h"
};

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
			case GameMode_Special:
				GM_Special();
				break;
		#ifdef SCP_SPLASH
			case GameMode_SSRG:
				GM_SSRG();
				break;
		#endif
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
	VDP_SeekCRAM(0);
	if (wtr_state)
		VDP_WriteCRAM(&wet_palette[0][0], 0x40);
	else
		VDP_WriteCRAM(&dry_palette[0][0], 0x40);
	
	//Copy buffers
	VDP_SeekVRAM(VRAM_SPRITES);
	VDP_WriteVRAM((const uint8_t*)sprite_buffer, sizeof(sprite_buffer));
	VDP_SeekVRAM(VRAM_HSCROLL);
	VDP_WriteVRAM((const uint8_t*)hscroll_buffer, sizeof(hscroll_buffer));
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
			ProcessDPLC();
			if (demo_length)
				demo_length--;
			break;
		case 0x08:
			//Read joypad state
			ReadJoypads();
			
			//Copy palette
			VDP_SeekCRAM(0);
			if (wtr_state)
				VDP_WriteCRAM(&wet_palette[0][0], 0x40);
			else
				VDP_WriteCRAM(&dry_palette[0][0], 0x40);
			
			//Copy buffers
			VDP_SetHIntPosition(hbla_pos);
			VDP_SeekVRAM(VRAM_SPRITES);
			VDP_WriteVRAM((const uint8_t*)sprite_buffer, sizeof(sprite_buffer));
			VDP_SeekVRAM(VRAM_HSCROLL);
			VDP_WriteVRAM((const uint8_t*)hscroll_buffer, sizeof(hscroll_buffer));
			
			//Update Sonic's art
			if (sonframe_chg)
			{
				VDP_SeekVRAM(0xF000);
				VDP_WriteVRAM(sgfx_buffer, SONIC_DPLC_SIZE);
				sonframe_chg = false;
			}
			
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
			
			if (hbla_pos >= 96) //Uh?
			{
				//Scroll camera
				LoadTilesAsYouMove();
				
				//Update level animations and HUD
				AnimateLevelGfx();
				HUD_Update();
				
				//Process PLCs
				ProcessDPLC2();
				
				//Decrement demo timer
				if (demo_length)
					demo_length--;
			}
			break;
		case 0x0A:
			//Read joypad state
			ReadJoypads();
			
			//Copy palette
			VDP_SeekCRAM(0);
			VDP_WriteCRAM(&dry_palette[0][0], 0x40);
			
			//Copy buffers
			VDP_SetHIntPosition(hbla_pos);
			VDP_SeekVRAM(VRAM_SPRITES);
			VDP_WriteVRAM((const uint8_t*)sprite_buffer, sizeof(sprite_buffer));
			VDP_SeekVRAM(VRAM_HSCROLL);
			VDP_WriteVRAM((const uint8_t*)hscroll_buffer, sizeof(hscroll_buffer));
			
			//Run palette cycle
			PCycle_SS();
			
			//Update Sonic's art
			if (sonframe_chg)
			{
				VDP_SeekVRAM(0xF000);
				VDP_WriteVRAM(sgfx_buffer, SONIC_DPLC_SIZE);
				sonframe_chg = false;
			}
			
			//Decrement demo timer
			if (demo_length)
				demo_length--;
			break;
		case 0x0C:
			//Read joypad state
			ReadJoypads();
			
			//Copy palette
			VDP_SeekCRAM(0);
			if (wtr_state)
				VDP_WriteCRAM(&wet_palette[0][0], 0x40);
			else
				VDP_WriteCRAM(&dry_palette[0][0], 0x40);
			
			//Copy buffers
			VDP_SetHIntPosition(hbla_pos);
			VDP_SeekVRAM(VRAM_SPRITES);
			VDP_WriteVRAM((const uint8_t*)sprite_buffer, sizeof(sprite_buffer));
			VDP_SeekVRAM(VRAM_HSCROLL);
			VDP_WriteVRAM((const uint8_t*)hscroll_buffer, sizeof(hscroll_buffer));
			
			//Update Sonic's art
			if (sonframe_chg)
			{
				VDP_SeekVRAM(0xF000);
				VDP_WriteVRAM(sgfx_buffer, SONIC_DPLC_SIZE);
				sonframe_chg = false;
			}
			
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
			
			//Scroll camera
			LoadTilesAsYouMove();
			
			//Update level animations and HUD
			AnimateLevelGfx();
			HUD_Update();
			
			//Process PLCs
			ProcessDPLC();
			break;
		case 0x12:
			WriteVRAMBuffers();
			ProcessDPLC();
			break;
	}
	
	//Update music
	
	//Increment VBlank counter
	vbla_count++;
}

void HBlank()
{
	
}
