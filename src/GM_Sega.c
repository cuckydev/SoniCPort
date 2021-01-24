#include "GM_Sega.h"

#include "Game.h"
#include "Video.h"
#include "Palette.h"
#include "PaletteCycle.h"
#include "Nemesis.h"

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
void GM_Sega()
{
	//Stop music
	//sfx	bgm_Stop,0,1,1
	
	//Clear the pattern load queue and fade out
	PaletteFadeOut();
	
	//Set VDP state
	VDP_SetPlaneALocation(VRAM_FG);
	VDP_SetPlaneBLocation(VRAM_BG);
	VDP_SetBackgroundColour(0);
	
	wtr_state = 0;
	
	//Clear screen and load SEGA graphics
	ClearScreen();
	
	NemDec(0x0000, art_sega);
	
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
		WaitForVBla();
	}
	while (PCycle_Sega());
	
	//Play "SEGA" sound
	
	vbla_routine = 0x14;
	WaitForVBla();
	
	//Wait a bit before resuming
	demo_length = 30;
	do
	{
		vbla_routine = 0x02;
		WaitForVBla();
		if (!demo_length)
			break;
	} while (!(jpad1_press1 & JPAD_START));
	
	//Start title gamemode
	gamemode = GameMode_Title;
}
