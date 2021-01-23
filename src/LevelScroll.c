#include "LevelScroll.h"

#include "Video.h"
#include "Level.h"
#include "LevelDraw.h"
#include "Game.h"

//Level scroll state (TODO: clean this UP)
uint8_t nobgscroll, bgscrollvert;

uint16_t fg_scroll_flags,     bg1_scroll_flags,     bg2_scroll_flags,     bg3_scroll_flags;
uint16_t fg_scroll_flags_dup, bg1_scroll_flags_dup, bg2_scroll_flags_dup, bg3_scroll_flags_dup;

dword_s scrposx,     scrposy,     bgscrposx,     bgscrposy,     bg2scrposx,     bg2scrposy,     bg3scrposx,     bg3scrposy;
dword_s scrposx_dup, scrposy_dup, bgscrposx_dup, bgscrposy_dup, bg2scrposx_dup, bg2scrposy_dup, bg3scrposx_dup, bg3scrposy_dup;

int16_t scrshiftx, scrshifty;

uint8_t fg_xblock, bg1_xblock, bg2_xblock, bg3_xblock;
uint8_t fg_yblock, bg1_yblock, bg2_yblock, bg3_yblock;

int16_t lookshift;

static ALIGNED4 uint8_t bgscroll_buffer[0x200];

//Scroll draw functions
void BGScroll_Block1(int32_t x, uint8_t bit)
{
	//Move background
	int32_t prev_x = bgscrposx.v;
	bgscrposx.v += x;
	
	//Handle scrolling flags
	uint8_t no_scroll = (bgscrposx.f.u & 0x10) ^ bg1_xblock;
	if (no_scroll)
		return;
	bg1_xblock ^= 0x10;
	
	if (bgscrposx.v < prev_x)
		bg1_scroll_flags |= bit;
	else
		bg1_scroll_flags |= (bit << 1);
}

void BGScroll_Block2(int32_t x, uint8_t bit)
{
	//Move background
	int32_t prev_x = bg2scrposx.v;
	bg2scrposx.v += x;
	
	//Handle scrolling flags
	uint8_t no_scroll = (bg2scrposx.f.u & 0x10) ^ bg2_xblock;
	if (no_scroll)
		return;
	bg2_xblock ^= 0x10;
	
	if (bg2scrposx.v < prev_x)
		bg2_scroll_flags |= bit;
	else
		bg2_scroll_flags |= (bit << 1);
}

void BGScroll_Block3(int32_t x, uint8_t bit)
{
	//Move background
	int32_t prev_x = bg3scrposx.v;
	bg3scrposx.v += x;
	
	//Handle scrolling flags
	uint8_t no_scroll = (bg3scrposx.f.u & 0x10) ^ bg3_xblock;
	if (no_scroll)
		return;
	bg3_xblock ^= 0x10;
	
	if (bg3scrposx.v < prev_x)
		bg3_scroll_flags |= bit;
	else
		bg3_scroll_flags |= (bit << 1);
}

//Level deformation routines
void Deform_GHZ()
{
	//TODO: port the REV00 routine
	
	int16_t fg_x, bg_x;
	int16_t *bufp = &hscroll_buffer[0][0];
	
	//Scroll background layers
	BGScroll_Block3((scrshiftx << 6) + (scrshiftx << 5), SCROLL_FLAG_LEFT2); //Upper mountains
	BGScroll_Block2(scrshiftx << 7, SCROLL_FLAG_LEFT2); //Hills and waterfalls
	
	//Get Y position
	vid_bgscrposy_dup =  0x20 - ((scrposy.f.u & 0x7FF) >> 5);
	if (vid_bgscrposy_dup < 0)
		vid_bgscrposy_dup = 0;
	
	//Get foreground position
	if (gamemode == GameMode_Title)
		fg_x = 0;
	else
		fg_x = -scrposx.f.u;
	
	//Scroll clouds
	int32_t *scroll = (int32_t*)bgscroll_buffer;
	scroll[0] += 0x10000;
	scroll[1] += 0xC000;
	scroll[2] += 0x8000;
	
	//Scroll cloud layer 1
	bg_x = -(bg3scrposx.f.u + (scroll[0] >> 16));
	for (int i = 0; i < 0x20 - vid_bgscrposy_dup; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll cloud layer 2
	bg_x = -(bg3scrposx.f.u + (scroll[1] >> 16));
	for (int i = 0; i < 0x10; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll cloud layer 3
	bg_x = -(bg3scrposx.f.u + (scroll[2] >> 16));
	for (int i = 0; i < 0x10; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll upper mountains
	bg_x = -bg3scrposx.f.u;
	for (int i = 0; i < 0x30; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll hills and waterfalls
	bg_x = -bg2scrposx.f.u;
	for (int i = 0; i < 0x28; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll water
	int32_t wx = bg2scrposx.v;
	int32_t wi = (((scrposx.f.u - bg2scrposx.f.u) << 8) / 0x68) << 8;
	
	for (int i = 0; i < 0x48 + vid_bgscrposy_dup; i++)
	{
		*bufp++ = fg_x; *bufp++ = -(wx >> 16);
		wx += wi;
	}
}

static void (*deform_routines[])() = {
	Deform_GHZ,
};

//Background scroll speed routines
void BgScroll_GHZ(int16_t x, int16_t y)
{
	(void)x;
	(void)y;
	#ifdef SCP_REV00
		Deform_GHZ();
	#else
		//Reset background positions
		bgscrposx.v = 0;
		bgscrposy.v = 0;
		bg2scrposy.v = 0;
		bg3scrposy.v = 0;
		
		//Reset cloud scrolling
		int32_t *scroll = (int32_t*)bgscroll_buffer;
		scroll[0] = 0;
		scroll[1] = 0;
		scroll[2] = 0;
	#endif
}

static void (*bgscroll_routines[])(int16_t, int16_t) = {
	BgScroll_GHZ,
};

void BgScrollSpeed(int16_t x, int16_t y)
{
	//Don't run if spawning from a checkpoint
	if (lastlamp)
		return;
	
	//Set background positions
	bgscrposy.f.u = y;
	bg2scrposy.f.u = y;
	bgscrposx.f.u = x;
	bg2scrposx.f.u = x;
	bg3scrposx.f.u = x;
	
	//Run zone's background scroll routine
	bgscroll_routines[level_id >> 8](x, y);
}

//Level scroll functions
void MoveScreenHoriz()
{
	//Get player's position relative to camera
	int16_t tocam_x = objects[0].pos.l.x.f.u - scrposx.f.u - (SCREEN_WIDTH / 2 - 16);
	
	//Scroll if we're 16 pixels to the left of the middle of the screen
	if (tocam_x < 0)
	{
		//No scroll limit... oops!
		tocam_x += scrposx.f.u;
		if (tocam_x < limitleft2)
			tocam_x = limitleft2;
		
	} //Or to the right of the middle of the screen
	else if ((tocam_x -= 16) >= 0)
	{
		//Limit scroll speed
		if (tocam_x >= 16)
			tocam_x = 16;
		
		//Scroll and limit to end of level
		tocam_x += scrposx.f.u;
		if (tocam_x >= limitright2)
			tocam_x = limitright2;
	}
	else
	{
		//Clear screen shift value
		scrshiftx = 0;
		return;
	}
	
	//Set camera position
	scrshiftx = (tocam_x - scrposx.f.u) << 8;
	scrposx.f.u = tocam_x;
}

void ScrollHoriz()
{
	//Move camera
	int16_t prev_x = scrposx.f.u;
	MoveScreenHoriz();
	
	//Handle scrolling flags
	uint8_t no_scroll = (scrposx.f.u & 0x10) ^ fg_xblock;
	if (no_scroll)
		return;
	fg_xblock ^= 0x10;
	
	if (scrposx.f.u < prev_x)
		fg_scroll_flags |= SCROLL_FLAG_LEFT;
	else
		fg_scroll_flags |= SCROLL_FLAG_RIGHT;
}

void ScrollVertical()
{
	
}

void DeformLayers()
{
	//Check if we're allowed to scroll
	if (nobgscroll)
		return;
	
	//Clear previous flags
	fg_scroll_flags = 0;
	bg1_scroll_flags = 0;
	bg2_scroll_flags = 0;
	bg3_scroll_flags = 0;
	
	//Scroll camera
	ScrollHoriz();
	ScrollVertical();
	
	//Copy screen Y position
	vid_scrposy_dup = scrposy.f.u;
	vid_bgscrposy_dup = bgscrposy.f.u;
	
	//Run zone's background deformation routine
	deform_routines[level_id >> 8]();
}

void LoadTilesAsYouMove_BGOnly()
{
	DrawBGScrollBlock1(bgscrposx.f.u,  bgscrposy.f.u,  &bg1_scroll_flags, level_layout[0][1], VRAM_BG);
	DrawBGScrollBlock2(bg2scrposx.f.u, bg2scrposy.f.u, &bg2_scroll_flags, level_layout[0][1], VRAM_BG);
	//No scroll block 3, even in REV01... odd
}
