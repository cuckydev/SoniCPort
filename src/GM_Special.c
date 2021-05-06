#include "GM_Special.h"

#include "Game.h"
#include "SpecialStage.h"
#include "Level.h"
#include "LevelScroll.h"
#include "LevelDraw.h"
#include "LevelCollision.h"
#include "Video.h"
#include "Palette.h"
#include "PaletteCycle.h"
#include "PLC.h"
#include "Nemesis.h"
#include "Demo.h"
#include "Object/Sonic.h"

#include <string.h>

//Special stage gamemode
void GM_Special()
{
	//Fade out
	//sfx	sfx_EnterSS,0,1,0 ; play special stage entry sound TODO
	PaletteWhiteOut();
	
	//Reset screen
	ClearScreen();
	VDP_SeekVRAM(0x5000);
	VDP_FillVRAM(0, 0x7000);
	
	//Load special assets
	QuickPLC(PlcId_SpecialStage);
	
	//Clear object memory
	memset(objects, 0, sizeof(objects));
	
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
	
	ss_angle.v = 0;
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
	
	//FE60 to FF00
	memset(oscillatory.state, 0, sizeof(oscillatory.state));
	memset(sprite_anim, 0, sizeof(sprite_anim));
	sprite_anim_3buf = 0;
	
	limit_top_db = 0;
	limit_btm_db = 0;
	
	//Clear Nemesis buffer
	memset(nemesis_buffer, 0, sizeof(nemesis_buffer));
	
	//Clear other memory
	wtr_state = 0;
	restart = false;
	
	//Load special stage palette and layout
	PalLoad1(PalId_Special);
	SS_Load();
	
	//Initialize special stage
	scrpos_x.v = 0;
	scrpos_y.v = 0;
	
	player->type = ObjId_SpecialSonic;
	PCycle_SS();
	
	ss_angle.v = 0;
	ss_rotate = 0x0040;
	//music	bgm_SS,0,1,0	; play special stage BG	music TODO
	
	//TODO: load demos
	
	rings = 0;
	life_num = 0;
	debug_use = false;
	demo_length = 1800;
	
	//Handle debug mode cheat
	if (debug_cheat && (jpad1_hold1 & JPAD_A))
		debug_mode = true;
	
	//Fade in
	PaletteWhiteIn();
	
	//Start special stage loop
	while (1)
	{
		//Run frame
		vbla_routine = 0x0A;
		WaitForVBla();
		
		MoveSonicInDemo();
		jpad1_hold2  = jpad1_hold1;
		jpad1_press2 = jpad1_press1;
		
		//Run and draw stage
		if (jpad1_hold1 & JPAD_LEFT)
			scrpos_x.f.u -= 4;
		if (jpad1_hold1 & JPAD_UP)
			scrpos_y.f.u -= 4;
		if (jpad1_hold1 & JPAD_RIGHT)
			scrpos_x.f.u += 4;
		if (jpad1_hold1 & JPAD_DOWN)
			scrpos_y.f.u += 4;
		if (jpad1_hold1 & JPAD_A)
			ss_angle.f.u--;
		if (jpad1_hold1 & JPAD_B)
			ss_angle.f.u++;
		
		ExecuteObjects();
		uint8_t sprite_i;
		BuildSprites(&sprite_i);
		SS_ShowLayout(sprite_i);
	}
}
