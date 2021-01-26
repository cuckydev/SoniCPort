#include "GM_Level.h"

#include "Game.h"
#include "MathUtil.h"
#include "Level.h"
#include "LevelDraw.h"
#include "LevelScroll.h"
#include "SpecialStage.h"
#include "Object/Sonic.h"
#include "Video.h"
#include "Palette.h"
#include "PaletteCycle.h"
#include "Nemesis.h"
#include "PLC.h"

#include <string.h>

//Title card art
static const uint8_t art_titlecard[] = {
	#include <Resource/Art/TitleCard.h>
};

//Level gamemode
void GM_Level()
{
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
		NemDec(0xB000, art_titlecard);
		
		//Load level art and general art
		if (level_header[LEVEL_ZONE(level_id)].plc1 != PlcId_Main)
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
	pause = 0;
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
	nobgscroll = 0;
	
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
	bgscrollvert = 0;
	sonspeed_max = 0;
	sonspeed_acc = 0;
	sonspeed_dec = 0;
	sonframe_num = 0;
	sonframe_chg = 0;
	angle_buffer = 0;
	
	opl_routine = 0;
	opl_screen = 0;
	memset(opl_data, 0, sizeof(opl_data));
	
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
	lock_ctrl = 0;
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
	VDP_SetBackgroundColour(0x30); //Line 3, entry 0
	
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
		
		while (1)
		{
			//Run game and load PLCs
			vbla_routine = 0x0C;
			WaitForVBla();
			ExecuteObjects();
			BuildSprites();
			RunPLC();
			
			//Break if title card is over and PLCs have loaded
			if (objects[4].pos.s.x != objects[4].scratch.u16[1])
				continue;
			if (plc_buffer[0].art != NULL)
				continue;
			break;
		}
		
		//Load level
		PalLoad1(PalId_Sonic);
		LevelSizeLoad();
		DeformLayers();
		fg_scroll_flags |= SCROLL_FLAG_LEFT;
		LevelDataLoad();
		
		PaletteFadeIn_At(0x10, 0x30);
	}
}
