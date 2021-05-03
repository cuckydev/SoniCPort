#include "GM_Level.h"

#include "Game.h"
#include "MathUtil.h"
#include "Level.h"
#include "LevelDraw.h"
#include "LevelScroll.h"
#include "LevelCollision.h"
#include "SpecialStage.h"
#include "Object/Sonic.h"
#include "Video.h"
#include "Palette.h"
#include "PaletteCycle.h"
#include "Nemesis.h"
#include "PLC.h"
#include "Demo.h"
#include "HUD.h"

#include <string.h>

//Title card art
static const uint8_t art_titlecard[] = {
	#include "Resource/Art/TitleCard.h"
	,0,
};

//Level gamemode
void GM_Level()
{
	GM_Level_Branch:;
	//Set 'title card' flag
	gamemode |= 0x80;
	
	if (demo >= 0)
		{;} //sfx	bgm_Fade,0,1,1 ; fade out music //TODO
	
	//Clear the pattern load queue and fade out
	ClearPLC();
	PaletteFadeOut();
	
	//Load art if not in credits
	if (demo >= 0)
	{
		//Load title card art
		VDP_SeekVRAM(0xB000);
		NemDec(art_titlecard);
		
		//Load level art and general art
		if (level_header[LEVEL_ZONE(level_id)].plc1 != 0)
			AddPLC(level_header[LEVEL_ZONE(level_id)].plc1);
		AddPLC(PlcId_Main2);
	}
	
	//Clear object memory
	memset(objects, 0, sizeof(objects));
	
	//Clear F628 to F680
	vbla_routine = 0;
	pcyc_num = 0;
	pcyc_time = 0;
	random_seed.v = 0;
	pause = false;
	hbla_pal = 0;
	wtr_pos1 = 0;
	wtr_pos2 = 0;
	wtr_pos3 = 0;
	wtr_routine = 0;
	wtr_state = 0;
	memset(pcyc_buffer, 0, sizeof(pcyc_buffer));
	
	//Clear F700 to F800
	scrpos_x.v = 0;
	scrpos_y.v = 0;
	bg_scrpos_x.v = 0;
	bg_scrpos_y.v = 0;
	bg2_scrpos_x.v = 0;
	bg2_scrpos_y.v = 0;
	bg3_scrpos_x.v = 0;
	bg3_scrpos_y.v = 0;
	
	limit_left1 = 0;
	limit_right1 = 0;
	limit_top1 = 0;
	limit_btm1 = 0;
	limit_left2 = 0;
	limit_right2 = 0;
	limit_top2 = 0;
	limit_btm2 = 0;
	limit_left3 = 0;
	
	scrshift_x = 0;
	scrshift_y = 0;
	
	look_shift = 0;
	dle_routine = 0;
	nobgscroll = false;
	
	fg_xblock = 0;
	fg_yblock = 0;
	bg1_xblock = 0;
	bg1_yblock = 0;
	bg2_xblock = 0;
	bg2_yblock = 0;
	bg3_xblock = 0;
	bg3_yblock = 0;
	
	fg_scroll_flags = 0;
	bg1_scroll_flags = 0;
	bg2_scroll_flags = 0;
	bg3_scroll_flags = 0;
	bgscrollvert = false;
	sonspeed_max = 0;
	sonspeed_acc = 0;
	sonspeed_dec = 0;
	sonframe_num = 0;
	sonframe_chg = 0;
	angle_buffer0 = 0;
	angle_buffer1 = 0;
	
	opl_routine = 0;
	opl_screen = 0;
	opl_ptr0 = NULL;
	opl_ptr4 = NULL;
	opl_ptr8 = NULL;
	opl_ptrC = NULL;
	
	ss_angle = 0;
	ss_rotate = 0;
	btn_pushtime1 = 0;
	btn_pushtime2 = 0;
	pal_chgspeed = 0;
	coll_index = NULL;
	palss_num = 0;
	palss_time = 0;
	
	btn_pushtime1 = 0;
	btn_pushtime2 = 0;
	obj31_ypos = 0;
	boss_status = 0;
	track_pos.v = 0;
	lock_screen = 0;
	memset(level_schunks, 0, sizeof(level_schunks));
	memset(level_anim, 0, sizeof(level_anim));
	gfx_big_ring = 0;
	convey_rev = 0;
	memset(obj63, 0, sizeof(obj63));
	tunnel_mode = 0;
	lock_multi = 0;
	tunnel_allow = 0;
	jump_only = 0;
	obj6B = 0;
	lock_ctrl = false;
	big_ring = 0;
	item_bonus = 0;
	time_bonus = 0;
	ring_bonus = 0;
	endact_bonus = 0;
	sonicend = 0;
	lz_deform = 0;
	memset(f_switch, 0, sizeof(f_switch));
	
	scroll_block1_size = 0;
	scroll_block2_size = 0;
	scroll_block3_size = 0;
	scroll_block4_size = 0;
	
	//FE60 to FF80
	memset(oscillatory.state, 0, sizeof(oscillatory.state));
	memset(sprite_anim, 0, sizeof(sprite_anim));
	sprite_anim_3buf = 0;
	
	limit_top_db = 0;
	limit_btm_db = 0;
	
	scrpos_x_dup.v = 0;
	scrpos_y_dup.v = 0;
	bg_scrpos_x_dup.v = 0;
	bg_scrpos_y_dup.v = 0;
	bg2_scrpos_x_dup.v = 0;
	bg2_scrpos_y_dup.v = 0;
	bg3_scrpos_x_dup.v = 0;
	bg3_scrpos_y_dup.v = 0;
	
	fg_scroll_flags_dup = 0;
	bg1_scroll_flags_dup = 0;
	bg2_scroll_flags_dup = 0;
	bg3_scroll_flags_dup = 0;
	
	//Clear screen
	ClearScreen();
	
	//Initialize VDP state
	VDP_SetPlaneALocation(VRAM_FG);
	VDP_SetPlaneBLocation(VRAM_BG);
	VDP_SetSpriteLocation(VRAM_SPRITES);
	VDP_SetPlaneSize(PLANE_WIDTH, PLANE_HEIGHT);
	VDP_SetBackgroundColour(0x20); //Line 2, entry 0
	
	//Load water
	hbla_pos = (SCREEN_HEIGHT - 1);
	if (LEVEL_ZONE(level_id) == ZoneId_LZ)
	{
		//TODO
	}
	air = 30;
	
	//Load Sonic's palette
	PalLoad2(PalId_Sonic);
	if (LEVEL_ZONE(level_id) == ZoneId_LZ)
		PalLoad3_Water((LEVEL_ACT(level_id) == 3) ? PalId_SonicSBZ : PalId_SonicLZ);
	if (last_lamp)
		{;}//move.b	($FFFFFE53).w,(f_wtr_state).w //TODO
	
	if (demo >= 0)
	{
		//Load music
		//TODO
		
		//Start title card
		objects[2].type = ObjId_TitleCard;
		
		do
		{
			//Run game and load PLCs
			vbla_routine = 0x0C;
			WaitForVBla();
			ExecuteObjects();
			BuildSprites();
			RunPLC();
		} while (objects[4].pos.s.x != objects[4].scratch.u16[4] || plc_buffer[0].art != NULL);
		
		//Initialize HUD
		HUD_Base();
	}
	
	//Load level
	PalLoad1(PalId_Sonic);
	LevelSizeLoad();
	DeformLayers();
	fg_scroll_flags |= SCROLL_FLAG_LEFT; //OK
	LevelDataLoad();
	LoadTilesFromStart();
	FloorLog_Unk();
	ColIndexLoad();
	
	//Create player and HUD objects
	player->type = ObjId_Sonic;
	if (demo >= 0)
		objects[1].type = ObjId_HUD;
	
	//Handle debug mode cheat
	if (debug_cheat && (jpad1_hold1 & JPAD_A))
		debug_mode = 1;
	jpad1_hold2 = 0;
	jpad1_press2 = 0;
	jpad1_hold1 = 0;
	jpad1_press1 = 0;
	
	//Load level objects
	ObjPosLoad();
	ExecuteObjects();
	BuildSprites();
	
	//Initialize game state
	if (!last_lamp)
	{
		rings = 0;
		time.pad = time.min = time.sec = time.frame = 0;
		life_num = 0;
	}
	
	time_over = false;
	shield = false;
	invincibility = false;
	shoes = false;
	debug_use = false;
	restart = false;
	frame_count = 0;
	
	//OscillateNumInit();
	
	score_count = true;
	ring_count = true;
	time_count = true;
	
	//Initialize demo
	btn_pushtime1 = 0;
	
	const uint8_t *demo_data;
	if (demo < 0)
		demo_data = ending_demo_ptr[credits_num - 1];
	else
		demo_data = intro_demo_ptr[LEVEL_ZONE(level_id)];
	btn_pushtime2 = demo_data[1] - 1;
	if (demo < 0)
		demo_length = (credits_num == 4) ? 510 : 540; //Credits length
	else
		demo_length = 1800; //Demo length
	
	//Load level's water palette
	if (LEVEL_ZONE(level_id) == ZoneId_LZ)
		PalLoad4_Water((LEVEL_ACT(level_id) == 3) ? PalId_LZWater : PalId_SBZ3Water);
	
	//Wait for 4 frames
	for (int i = 0; i < 4; i++)
	{
		vbla_routine = 0x08;
		WaitForVBla();
	}
	
	//Fade into level
	PaletteFadeIn_At(0x10, 0x30);
	
	//Tell title card to move away
	objects[2].routine += 2;
	objects[3].routine += 4;
	objects[4].routine += 4;
	objects[5].routine += 4;
	
	//Load missing art in credits demos
	if (demo < 0)
	{
		AddPLC(PlcId_Explode);
		AddPLC(PlcId_GHZAnimals + LEVEL_ZONE(level_id));
	}
	
	//Enter level loop
	gamemode &= 0x7F;
	while (1)
	{
		//Run frame
		vbla_routine = 0x08;
		WaitForVBla();
		frame_count++;
		
		MoveSonicInDemo();
		//LZWaterFeatures();
		
		//Run game
		ExecuteObjects();
		#ifndef SCP_REV00
			//Restart level gamemode if restart flag set
			if (restart)
				goto GM_Level_Branch;
		#endif
		
		//Setup video and load PLCs
		if (debug_use || player->routine < 6)
			DeformLayers();
		BuildSprites();
		ObjPosLoad();
		PaletteCycle();
		RunPLC();
		
		//Other level stuff
		SynchroAnimate();
		
		//Check if level loop should end
		if (gamemode != GameMode_Demo)
		{
			#ifdef SCP_REV00
				//Restart level gamemode if restart flag set
				if (restart)
					goto GM_Level_Branch;
			#endif
			
			//Break if exited the level gamemode
			if (gamemode != GameMode_Level)
				break;
		}
		else
		{
			//Begin to fade if restart flag set or demo ended
			if (restart || !demo_length)
			{
				//Get next game mode
				if (gamemode == GameMode_Demo) //I HATE YOU
					gamemode = (demo < 0) ? GameMode_Credits : GameMode_Sega;
				
				//Prepare fade
				demo_length = 60;
				palette_fade.ind = 0;
				palette_fade.len = 0x40;
				pal_chgspeed = 0;
				
				//Fade loop
				do
				{
					//Run frame
					vbla_routine = 0x08;
					WaitForVBla();
					
					MoveSonicInDemo();
					
					//Run game
					ExecuteObjects();
					BuildSprites();
					ObjPosLoad();
					
					//Fade
					if (--pal_chgspeed < 0)
					{
						pal_chgspeed = 2;
						FadeOut_ToBlack();
					}
				} while (demo_length);
				break;
			}
			else if (gamemode != GameMode_Demo) //Condition never met
			{
				//Go to SEGA game mode
				gamemode = GameMode_Sega;
				break;
			}
		}
	}
}
