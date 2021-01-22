#include "GM_Sega.h"

#include "Game.h"
#include "Video.h"
#include "Palette.h"
#include "PaletteCycle.h"

#include <Backend/VDP.h>

//SEGA art
#ifdef SCP_REV00
	static const uint8_t art_sega[] = {
		#include <Resource/Art/SegaREV00.h>
	};
	static ALIGNED2 const uint8_t map_sega[] = {
		#include <Resource/Tilemap/SegaREV00.h>
	};
#else
	static const uint8_t art_sega[] = {
		#include <Resource/Art/SegaREV01.h>
	};
	static ALIGNED2 const uint8_t map_sega[] = {
		#include <Resource/Tilemap/SegaREV01.h>
	};
#endif

//SEGA gamemode
int GM_Sega()
{
	int result;
	
	//Stop music
	//sfx	bgm_Stop,0,1,1
	
	//Clear the pattern load queue and fade out
	if ((result = PaletteFadeOut()))
		return result;
	
	//Set VDP state
	VDP_SetPlaneALocation(VRAM_FG);
	VDP_SetPlaneBLocation(VRAM_BG);
	VDP_SetBackgroundColour(0);
	
	wtr_state = 0;
	
	//Clear screen and load SEGA graphics
	ClearScreen();
	
	VDP_WriteVRAM(0x0000, art_sega, sizeof(art_sega));
	
	CopyTilemap(&map_sega[0x0000], 0xE510 + PLANE_WIDEADD, 24, 8);
	CopyTilemap(&map_sega[0x0180], 0xC000 + PLANE_WIDEADD, 40, 28);
	#ifdef SCP_JP
		CopyTilemap(&map_sega[0x0A40], 0xC53A + PLANE_WIDEADD, 3, 2); //Hide trademark symbol
	#endif
	
	//Load palette and initialize cycle
	PalLoad2(PalId_SegaBG);
	pcyc_num = -10;
	pcyc_time = 0x0000;
	pcyc_buffer[6] = 0;
	pcyc_buffer[5] = 0;
	
	//Run palette cycle
	do
	{
		vbla_routine = 0x02;
		if ((result = WaitForVBla()))
			return result;
	}
	while (PCycle_Sega());
	
	//Play "SEGA" sound
	
	vbla_routine = 0x14;
	if ((result = WaitForVBla()))
		return result;
	
	//Wait a bit before resuming
	demo_length = 30;
	do
	{
		vbla_routine = 0x02;
		if ((result = WaitForVBla()))
			return result;
		if (!demo_length)
			break;
	} while (1); //TODO: start button isn't pressed
	
	//Start title gamemode
	gamemode = GameMode_Title;
	
	return 0;
}
