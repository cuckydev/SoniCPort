#include "LevelScroll.h"

#include "Video.h"
#include "Level.h"
#include "LevelDraw.h"

//Level deformation state
uint8_t nobgscroll;

uint16_t fg_scroll_flags, bg1_scroll_flags, bg2_scroll_flags, bg3_scroll_flags;

int16_t scrposy, bgscrposy, scrposx, bgscrposx, bg2scrposx, bg3scrposx;
int16_t scrposy_dup, bgscrposy_dup, scrposx_dup, bgscrposx_dup, bg2scrposy, bg3scrposy;

int16_t scrshiftx, scrshifty;

//Level deformation routines


//Level scroll functions
void MoveScreenHoriz()
{
	//Get player's position relative to camera
	int16_t tocam_x = objects[0].pos.l.x.f.p - scrposx - (SCREEN_WIDTH / 2 - 16);
	
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
		if (tocam_x >= limitright2)
			tocam_x = limitright2;
	}
	else
	{
		//Clear screen shift value
		scrshiftx = 0;
		return;
	}
	
	//Set scroll state
	scrshiftx = (tocam_x - scrposx) << 8;
	scrposx = tocam_x;
}

void ScrollHoriz()
{
	//Move camera
	int16_t prev_x = scrposx;
	MoveScreenHoriz();
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
}
