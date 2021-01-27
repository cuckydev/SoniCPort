#include "LevelScroll.h"

#include "Video.h"
#include "Level.h"
#include "LevelDraw.h"
#include "Game.h"

//Level scroll state (TODO: clean this UP)
uint8_t nobgscroll, bgscrollvert;

uint16_t fg_scroll_flags,     bg1_scroll_flags,     bg2_scroll_flags,     bg3_scroll_flags;
uint16_t fg_scroll_flags_dup, bg1_scroll_flags_dup, bg2_scroll_flags_dup, bg3_scroll_flags_dup;

dword_s scrpos_x,     scrpos_y,     bg_scrpos_x,     bg_scrpos_y,     bg2_scrpos_x,     bg2_scrpos_y,     bg3_scrpos_x,     bg3_scrpos_y;
dword_s scrpos_x_dup, scrpos_y_dup, bg_scrpos_x_dup, bg_scrpos_y_dup, bg2_scrpos_x_dup, bg2_scrpos_y_dup, bg3_scrpos_x_dup, bg3_scrpos_y_dup;

int16_t scrshift_x, scrshift_y;

uint8_t fg_xblock, bg1_xblock, bg2_xblock, bg3_xblock;
uint8_t fg_yblock, bg1_yblock, bg2_yblock, bg3_yblock;

int16_t look_shift;

static ALIGNED4 uint8_t bgscroll_buffer[0x200];

//Scroll draw functions
void BGScroll_Block1(int32_t x, uint8_t bit)
{
	//Move background
	int32_t prev_x = bg_scrpos_x.v;
	bg_scrpos_x.v += x;
	
	//Handle scrolling flags
	uint8_t no_scroll = (bg_scrpos_x.f.u & 0x10) ^ bg1_xblock;
	if (no_scroll)
		return;
	bg1_xblock ^= 0x10;
	
	if (bg_scrpos_x.v < prev_x)
		bg1_scroll_flags |= bit;
	else
		bg1_scroll_flags |= (bit << 1);
}

void BGScroll_Block2(int32_t x, uint8_t bit)
{
	//Move background
	int32_t prev_x = bg2_scrpos_x.v;
	bg2_scrpos_x.v += x;
	
	//Handle scrolling flags
	uint8_t no_scroll = (bg2_scrpos_x.f.u & 0x10) ^ bg2_xblock;
	if (no_scroll)
		return;
	bg2_xblock ^= 0x10;
	
	if (bg2_scrpos_x.v < prev_x)
		bg2_scroll_flags |= bit;
	else
		bg2_scroll_flags |= (bit << 1);
}

void BGScroll_Block3(int32_t x, uint8_t bit)
{
	//Move background
	int32_t prev_x = bg3_scrpos_x.v;
	bg3_scrpos_x.v += x;
	
	//Handle scrolling flags
	uint8_t no_scroll = (bg3_scrpos_x.f.u & 0x10) ^ bg3_xblock;
	if (no_scroll)
		return;
	bg3_xblock ^= 0x10;
	
	if (bg3_scrpos_x.v < prev_x)
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
	BGScroll_Block3((scrshift_x << 6) + (scrshift_x << 5), SCROLL_FLAG_LEFT2); //Upper mountains
	BGScroll_Block2(scrshift_x << 7, SCROLL_FLAG_LEFT2); //Hills and waterfalls
	
	//Get Y position
	vid_bg_scrpos_y_dup =  0x20 - ((scrpos_y.f.u & 0x7FF) >> 5);
	if (vid_bg_scrpos_y_dup < 0)
		vid_bg_scrpos_y_dup = 0;
	
	//Get foreground position
	if (gamemode == GameMode_Title)
		fg_x = 0;
	else
		fg_x = -scrpos_x.f.u;
	
	//Scroll clouds
	int32_t *scroll = (int32_t*)bgscroll_buffer;
	scroll[0] += 0x10000;
	scroll[1] += 0xC000;
	scroll[2] += 0x8000;
	
	//Scroll cloud layer 1
	bg_x = -(bg3_scrpos_x.f.u + (scroll[0] >> 16));
	for (int i = 0; i < 0x20 - vid_bg_scrpos_y_dup; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll cloud layer 2
	bg_x = -(bg3_scrpos_x.f.u + (scroll[1] >> 16));
	for (int i = 0; i < 0x10; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll cloud layer 3
	bg_x = -(bg3_scrpos_x.f.u + (scroll[2] >> 16));
	for (int i = 0; i < 0x10; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll upper mountains
	bg_x = -bg3_scrpos_x.f.u;
	for (int i = 0; i < 0x30; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll hills and waterfalls
	bg_x = -bg2_scrpos_x.f.u;
	for (int i = 0; i < 0x28; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll water
	int32_t wx = bg2_scrpos_x.v;
	int32_t wi = (((scrpos_x.f.u - bg2_scrpos_x.f.u) << 8) / 0x68) << 8;
	
	for (int i = 0; i < 0x48 + vid_bg_scrpos_y_dup; i++)
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
		bg_scrpos_x.v = 0;
		bg_scrpos_y.v = 0;
		bg2_scrpos_y.v = 0;
		bg3_scrpos_y.v = 0;
		
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
	if (last_lamp)
		return;
	
	//Set background positions
	bg_scrpos_y.f.u = y;
	bg2_scrpos_y.f.u = y;
	bg_scrpos_x.f.u = x;
	bg2_scrpos_x.f.u = x;
	bg3_scrpos_x.f.u = x;
	
	//Run zone's background scroll routine
	bgscroll_routines[level_id >> 8](x, y);
}

//Level scroll functions
void MoveScreenHoriz()
{
	//Get player's position relative to camera
	int16_t tocam_x = objects[0].pos.l.x.f.u - scrpos_x.f.u - (SCREEN_WIDTH / 2 - 16);
	
	//Scroll if we're 16 pixels to the left of the middle of the screen
	if (tocam_x < 0)
	{
		//No scroll limit... oops!
		tocam_x += scrpos_x.f.u;
		if (tocam_x < limit_left2)
			tocam_x = limit_left2;
		
	} //Or to the right of the middle of the screen
	else if ((tocam_x -= 16) >= 0)
	{
		//Limit scroll speed
		if (tocam_x >= 16)
			tocam_x = 16;
		
		//Scroll and limit to end of level
		tocam_x += scrpos_x.f.u;
		if (tocam_x >= limit_right2)
			tocam_x = limit_right2;
	}
	else
	{
		//Clear screen shift value
		scrshift_x = 0;
		return;
	}
	
	//Set camera position
	scrshift_x = (tocam_x - scrpos_x.f.u) << 8;
	scrpos_x.f.u = tocam_x;
}

void ScrollHoriz()
{
	//Move camera
	int16_t prev_x = scrpos_x.f.u;
	MoveScreenHoriz();
	
	//Handle scrolling flags
	uint8_t no_scroll = (scrpos_x.f.u & 0x10) ^ fg_xblock;
	if (no_scroll)
		return;
	fg_xblock ^= 0x10;
	
	if (scrpos_x.f.u < prev_x)
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
	vid_scrpos_y_dup = scrpos_y.f.u;
	vid_bg_scrpos_y_dup = bg_scrpos_y.f.u;
	
	//Run zone's background deformation routine
	deform_routines[level_id >> 8]();
}
