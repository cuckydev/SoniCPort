#include "LevelScroll.h"

#include "Video.h"
#include "Level.h"
#include "LevelDraw.h"
#include "Game.h"

//Level scroll state (TODO: clean this UP)
uint8_t nobgscroll, bgscrollvert;

uint16_t fg_scroll_flags, bg1_scroll_flags, bg2_scroll_flags, bg3_scroll_flags;

int32_t scrposx, scrposy, bgscrposy;
int32_t scrposx_dup, scrposy_dup, bgscrposy_dup;
int32_t bgscrposx, bg2scrposx, bg3scrposx;
int32_t bgscrposx_dup, bg2scrposx_dup, bg3scrposx_dup;
int32_t bgscrposy_dup, bg2scrposy_dup, bg3scrposy_dup;

int16_t scrshiftx, scrshifty;

uint8_t fg_xblock, bg1_xblock, bg2_xblock, bg3_xblock;
uint8_t fg_yblock, bg1_yblock, bg2_yblock, bg3_yblock;

static ALIGNED4 uint8_t bgscroll_buffer[0x200];

//Scroll draw functions
void BGScroll_Block1(int32_t x, uint8_t bit)
{
	//Move background
	int32_t prev_x = bgscrposx;
	bgscrposx += x;
	
	//Handle scrolling flags
	uint8_t no_scroll = ((bgscrposx >> 16) & 0x10) ^ bg1_xblock;
	if (no_scroll)
		return;
	
	if (bgscrposx < prev_x)
		fg_scroll_flags |= (1 << bit);
	else
		fg_scroll_flags |= (1 << (bit + 1));
}

void BGScroll_Block2(int32_t x, uint8_t bit)
{
	//Move background
	int32_t prev_x = bg2scrposx;
	bg2scrposx += x;
	
	//Handle scrolling flags
	uint8_t no_scroll = ((bg2scrposx >> 16) & 0x10) ^ bg2_xblock;
	if (no_scroll)
		return;
	
	if (bg2scrposx < prev_x)
		fg_scroll_flags |= (1 << bit);
	else
		fg_scroll_flags |= (1 << (bit + 1));
}

void BGScroll_Block3(int32_t x, uint8_t bit)
{
	//Move background
	int32_t prev_x = bg3scrposx;
	bg3scrposx += x;
	
	//Handle scrolling flags
	uint8_t no_scroll = ((bg3scrposx >> 16) & 0x10) ^ bg3_xblock;
	if (no_scroll)
		return;
	
	if (bg3scrposx < prev_x)
		fg_scroll_flags |= (1 << bit);
	else
		fg_scroll_flags |= (1 << (bit + 1));
}

//Level deformation routines
void Deform_GHZ()
{
	int16_t fg_x, bg_x;
	int16_t *bufp = &hscroll_buffer[0][0];
	
	//Scroll background layers
	BGScroll_Block3((scrshiftx << 6) + (scrshiftx << 5), 0); //Upper mountains
	BGScroll_Block2(scrshiftx << 7, 0); //Hills and waterfalls
	
	//Get Y position
	bgscrposy_dup =  0x20 - ((scrposy & 0x7FF) >> 5);
	if (bgscrposy_dup < 0)
		bgscrposy_dup = 0;
	
	//Get foreground position
	if (gamemode == GameMode_Title)
		fg_x = 0;
	else
		fg_x = -scrposx;
	
	//Scroll clouds
	int32_t *scroll = (int32_t*)bgscroll_buffer;
	scroll[0] += 0x10000;
	scroll[1] += 0xC000;
	scroll[2] += 0x8000;
	
	//Scroll cloud layer 1
	bg_x = -((bg3scrposx + scroll[0]) >> 16);
	for (int i = 0; i < 0x20 - bgscrposy_dup; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll cloud layer 2
	bg_x = -((bg3scrposx + scroll[1]) >> 16);
	for (int i = 0; i < 0x10; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll cloud layer 3
	bg_x = -((bg3scrposx + scroll[2]) >> 16);
	for (int i = 0; i < 0x10; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll upper mountains
	bg_x = -(bg3scrposx >> 16);
	for (int i = 0; i < 0x30; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll hills and waterfalls
	bg_x = -(bg2scrposx >> 16);
	for (int i = 0; i < 0x28; i++)
	{ *bufp++ = fg_x; *bufp++ = bg_x; }
	
	//Scroll water
	int32_t wx = bg2scrposx;
	int32_t wi = (((scrposx - (bg2scrposx >> 16)) << 8) / 0x68) << 8;
	
	for (int i = 0; i < 0x48 + bgscrposy_dup; i++)
	{
		*bufp++ = fg_x; *bufp++ = -(wx >> 16);
		wx += wi;
	}
}

static void (*deform_routines[])() = {
	Deform_GHZ,
};

//Level scroll functions
void MoveScreenHoriz()
{
	//Get player's position relative to camera
	int16_t tocam_x = objects[0].pos.l.x.f.u - scrposx - (SCREEN_WIDTH / 2 - 16);
	
	//Scroll if we're 16 pixels to the left of the middle of the screen
	if (tocam_x < 0)
	{
		//No scroll limit... oops!
		tocam_x += scrposx;
		if (tocam_x < limitleft2)
			tocam_x = limitleft2;
		
	} //Or to the right of the middle of the screen
	else if ((tocam_x -= 16) >= 0)
	{
		//Limit scroll speed
		if (tocam_x >= 16)
			tocam_x = 16;
		
		//Scroll and limit to end of level
		tocam_x += scrposx;
		//if (tocam_x >= limitright2)
		//	tocam_x = limitright2;
	}
	else
	{
		//Clear screen shift value
		scrshiftx = 0;
		return;
	}
	
	//Set camera position
	scrshiftx = (tocam_x - scrposx) << 8;
	scrposx = tocam_x;
}

void ScrollHoriz()
{
	//Move camera
	int16_t prev_x = scrposx;
	MoveScreenHoriz();
	
	//Handle scrolling flags
	uint8_t no_scroll = (scrposx & 0x10) ^ fg_xblock;
	if (no_scroll)
		return;
	fg_xblock ^= 0x10;
	
	if (scrposx < prev_x)
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
	scrposy_dup = scrposy;
	bgscrposy_dup = bgscrposy;
	
	//Deform background
	deform_routines[level_id >> 8]();
}
