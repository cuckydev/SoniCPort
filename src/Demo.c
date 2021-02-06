#include "Demo.h"

#include "Game.h"
#include "Level.h"

#include <stddef.h>

//Demo state
uint16_t btn_pushtime1;
uint8_t btn_pushtime2;

//Demos
static const uint8_t demo_intro_ghz[] = {
	#include <Resource/Demo/IntroGHZ.h>
};
static const uint8_t demo_intro_mz[] = {
	#include <Resource/Demo/IntroMZ.h>
};
static const uint8_t demo_intro_syz[] = {
	#include <Resource/Demo/IntroSYZ.h>
};
static const uint8_t demo_intro_ss[] = {
	#include <Resource/Demo/IntroSS.h>
};

static const uint8_t demo_ending_ghz1[] = {
	#include <Resource/Demo/EndingGHZ1.h>
};
static const uint8_t demo_ending_mz[] = {
	#include <Resource/Demo/EndingMZ.h>
};
static const uint8_t demo_ending_syz[] = {
	#include <Resource/Demo/EndingSYZ.h>
};
static const uint8_t demo_ending_lz[] = {
	#include <Resource/Demo/EndingLZ.h>
};
static const uint8_t demo_ending_slz[] = {
	#include <Resource/Demo/EndingSLZ.h>
};
static const uint8_t demo_ending_sbz1[] = {
	#include <Resource/Demo/EndingSBZ1.h>
};
static const uint8_t demo_ending_sbz2[] = {
	#include <Resource/Demo/EndingSBZ2.h>
};
static const uint8_t demo_ending_ghz2[] = {
	#include <Resource/Demo/EndingGHZ2.h>
};

const uint8_t *intro_demo_ptr[] = {
	/* ZoneId_GHZ  */ demo_intro_ghz,
	/* ZoneId_LZ   */ demo_intro_ghz,
	/* ZoneId_MZ   */ demo_intro_mz,
	/* ZoneId_SLZ  */ demo_intro_mz,
	/* ZoneId_SYZ  */ demo_intro_syz,
	/* ZoneId_SBZ  */ demo_intro_syz,
	/* ZoneId_EndZ */ demo_intro_ss,
	                  demo_intro_ss,
};

const uint8_t *ending_demo_ptr[] = {
	demo_ending_ghz1,
	demo_ending_mz,
	demo_ending_syz,
	demo_ending_lz,
	demo_ending_slz,
	demo_ending_sbz1,
	demo_ending_sbz2,
	demo_ending_ghz2,
};

//Demo playback
void MoveSonicInDemo()
{
	if (!demo)
		return;
	
	//Return to title screen if start is pressed
	if (demo >= 0 && (jpad1_hold1 & JPAD_START))
		gamemode = GameMode_Title;
	
	//Get demo data
	const uint8_t *demo_data;
	if (demo < 0)
		demo_data = ending_demo_ptr[credits_num - 1];
	else
		demo_data = intro_demo_ptr[(gamemode == GameMode_Special) ? 6 : LEVEL_ZONE(level_id)];
	
	//Offset demo address
	demo_data += btn_pushtime1;
	
	//Apply input onto joypad state
	uint8_t d0 = demo_data[0];
	uint8_t d1 = d0;
	#ifdef SCP_REV00
		uint8_t d2 = jpad1_hold1;
	#else
		uint8_t d2 = 0; //Fix the infamous demo playback bug
	#endif
	d0 ^= d2;
	jpad1_hold1 = d1;
	d0 &= d1;
	jpad1_press1 = d0;
	
	//Handle demo timer
	if (--btn_pushtime2 == 0xFF)
	{
		btn_pushtime2 = demo_data[3];
		btn_pushtime1 += 2;
	}
}
