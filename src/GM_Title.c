#include "GM_Title.h"

#include "Game.h"
#include "Video.h"
#include "Palette.h"
#include "PaletteCycle.h"
#include "Level.h"
#include "LevelDraw.h"
#include "LevelScroll.h"
#include "Nemesis.h"

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
void GM_Title()
{
	//Stop music
	//sfx	bgm_Stop,0,1,1
	
	//Clear the pattern load queue and fade out
	PaletteFadeOut();
	
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
	NemDec(0x0000, art_japanese_credits);
	NemDec(0x14C0, art_credits_font);
	
	CopyTilemap(map_japanese_credits, 0xC000 + PLANE_WIDEADD, 40, 24);
	
	//Clear palette
	memset(dry_palette_dup, 0, sizeof(dry_palette_dup));
	PalLoad1(PalId_Sonic);
	
	//Load "SONIC TEAM PRESENTS" object
	objects[2].type = ObjId_Credits;
	
	ExecuteObjects();
	BuildSprites();
	
	//Fade in
	PaletteFadeIn();
	
	//Load title art
	NemDec(0x4000, art_title_fg);
	NemDec(0x6000, art_title_sonic);
	NemDec(0xA200, art_title_tm);
	
	//Reset game state
	last_lamp = 0;
	demo = 0;
	
	//Load GHZ
	level_id = LEVEL_ID(ZoneId_GHZ, 0);
	pcyc_time = 0;
	
	LevelSizeLoad();
	DeformLayers();
	LoadLevelMaps();
	LoadLevelLayout();
	player->pos.l.x.f.u += SCREEN_WIDEADD2; //For widescreen so the title starts scrolling at the correct time
	
	//Fade out
	PaletteFadeOut();
	
	//Draw background
	ClearScreen();
	DrawChunks(bg_scrpos_x.f.u, bg_scrpos_y.f.u, level_layout[0][1], VRAM_BG);
	
	//Load title mappings
	CopyTilemap(&map_title_fg[0x0000], 0xC206 + PLANE_WIDEADD, 34, 22);
	
	//Load GHZ art and title palette
	NemDec(0x0000, art_ghz1);
	PalLoad1(PalId_Title);
	
	//Run title screen for 376 frames
	demo_length = 376;
	
	//Clear objects
	#ifdef SCP_FIX_BUGS
		memset(&objects[2], 0, sizeof(Object));
	#else
		memset(&objects[2], 0, 0x20); //0x20 instead of the object structure size?
		                              //This why the "PRESS START BUTTON" text is missing.
	#endif
	
	//Load title objects
	objects[1].type = ObjId_TitleSonic;
	objects[2].type = ObjId_PSB;
	#ifndef SCP_JP
		objects[3].type = ObjId_PSB;
		objects[3].frame = 3;
	#endif
	objects[4].type = ObjId_PSB;
	objects[4].frame = 2;
	
	ExecuteObjects();
	DeformLayers();
	BuildSprites();
	
	//Fade in
	PaletteFadeIn();
	
	//Loop
	while (1)
	{
		//Render frame
		vbla_routine = 0x04;
		WaitForVBla();
		
		//Handle objects and scrolling
		ExecuteObjects();
		DeformLayers();
		BuildSprites();
		
		//Run palette cycle
		PCycle_Title();
		
		//Move Sonic object (yep, this is how they scroll the camera)
		//...and return to the Sega screen after a minute?
		if ((player->pos.l.x.f.u += 2) >= 0x1C00)
		{
			gamemode = GameMode_Sega;
			return;
		}
		
		//TODO: Check for level select cheat
		
		//Check if start is pressed
		if (jpad1_press1 & JPAD_START)
		{
			;
		}
		
		//Check if the title's over
		if (!demo_length)
		{
			gamemode = GameMode_Demo;
			return;
		}
	}
}
