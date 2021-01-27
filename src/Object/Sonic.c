#include "Sonic.h"

#include "Object.h"
#include "Game.h"
#include "Level.h"

//Sonic mappings
static const uint8_t map_sonic[] = {
	#include <Resource/Mappings/Sonic.h>
};

//Sonic globals
int16_t sonspeed_max, sonspeed_acc, sonspeed_dec;
uint8_t sonframe_num, sonframe_chg;

uint8_t angle_buffer;

//Sonic object
void Obj_Sonic(Object *obj)
{
	//Run debug mode code while in debug mode
	if (debug_use)
	{
		//DebugMode();
		return;
	}
	
	//Run player routine
	switch (obj->routine)
	{
		case 0: //Initialiation
			//Increment routine
			obj->routine += 2;
			
			//Initialize collision size
			obj->y_rad = 19;
			obj->x_rad = 9;
			
			//Set object drawing information
			obj->mappings = map_sonic;
			obj->tile.w = 0;
			obj->tile.s.pattern = 0x780;
			obj->priority = 2;
			obj->width_pixels = 24;
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			
			//Initialize speeds
			sonspeed_max = 0x600;
			sonspeed_acc = 0xC;
			sonspeed_dec = 0x80;
	//Fallthrough
		case 2:
			//Enter debug mode
			if (debug_cheat && (jpad1_press1 & JPAD_B))
			{
				debug_use = true;
				lock_ctrl = false;
				break;
			}
			break;
	}
}
