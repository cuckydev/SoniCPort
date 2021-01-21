#include "GM_Title.h"

#include "Game.h"
#include "Video.h"
#include "Palette.h"
#include "PaletteCycle.h"
#include "Level.h"
#include "LevelDraw.h"
#include "LevelScroll.h"

#include <Backend/VDP.h>

#include <string.h>

//Japanese credits
static const uint8_t art_japanese_credits[] = {
	#include <Resource/Art/JapaneseCredits.h>
};
static ALIGNED2 const uint8_t map_japanese_credits[] = {
	#include <Resource/Tilemap/JapaneseCredits.h>
};

//Credits font
static const uint8_t art_credits_font[] = {
	#include <Resource/Art/CreditsFont.h>
};

//Title
static const uint8_t art_title_fg[] = {
	#include <Resource/Art/TitleFG.h>
};
static const uint8_t art_title_sonic[] = {
	#include <Resource/Art/TitleSonic.h>
};
static const uint8_t art_title_tm[] = {
	#include <Resource/Art/TitleTM.h>
};
static ALIGNED2 const uint8_t map_title_fg[] = {
	#include <Resource/Tilemap/TitleFG.h>
};

//Title gamemode
int GM_Title()
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
	VDP_SetBackgroundColour(0x20);
	
	wtr_state = 0;
	
	//Clear screen
	ClearScreen();
	
	//Clear object memory
	memset(objects, 0, sizeof(objects));
	
	//Load Japanese credits
	VDP_WriteVRAM(0x0000, art_japanese_credits, sizeof(art_japanese_credits));
	VDP_WriteVRAM(0x14C0, art_credits_font, sizeof(art_credits_font));
	
	CopyTilemap(map_japanese_credits, 0xC000 + PLANE_WIDEADD, 40, 24);
	
	//Clear palette
	memset(dry_palette_dup, 0, sizeof(dry_palette_dup));
	PalLoad1(PalId_Sonic);
	
	//Load "SONIC TEAM PRESENTS" object
	//objects[2].type = 0;
	
	//Fade in
	if ((result = PaletteFadeIn()))
		return result;
	
	//Load title art
	VDP_WriteVRAM(0x4000, art_title_fg, sizeof(art_title_fg));
	VDP_WriteVRAM(0x6000, art_title_sonic, sizeof(art_title_sonic));
	VDP_WriteVRAM(0xA200, art_title_tm, sizeof(art_title_tm));
	
	//Reset game state
	
	//Load GHZ
	level_id = LEVEL_ID(ZoneId_GHZ, 0);
	pcyc_time = 0;
	
	LoadLevelMaps();
	LoadLevelLayout();
	DeformLayers();
	
	//Fade out
	if ((result = PaletteFadeOut()))
		return result;
	
	//Draw background
	ClearScreen();
	DrawChunks(bgscrposx, bgscrposy, level_layout[0][1], VRAM_BG);
	
	//Load title mappings
	CopyTilemap(&map_title_fg[0x0000], 0xC206 + PLANE_WIDEADD, 34, 22);
	
	//Load GHZ art and title palette
	VDP_WriteVRAM(0x0000, art_ghz1, art_ghz1_size);
	PalLoad1(PalId_Title);
	
	//Clear objects (Clears 0x20 bytes... weird)
	memset(&objects[2], 0, 0x20);
	
	//Load title objects
	
	//Deform background
	DeformLayers();
	
	//Fade in
	if ((result = PaletteFadeIn()))
		return result;
	
	//Loop
	while (1)
	{
		//Render frame
		vbla_routine = 0x02;
		if ((result = WaitForVBla()))
			return result;
		
		//Handle objects and scrolling
		//ExecuteObjects();
		DeformLayers();
		//BuildSprites();
		
		//Run palette cycle
		PCycle_Title();
		
		//Move Sonic object (yep, this is how they scroll the camera)
		//...and return to the Sega screen after a minute?
		if ((objects[0].pos.l.x.f.u += 2) >= 0x1C00)
		{
			gamemode = GameMode_Sega;
			return 0;
		}
	}
	
	return 0;
}
