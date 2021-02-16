#include "Object.h"

#include "Game.h"
#include "Palette.h"

//Credits assets
static const uint8_t map_credits[] = {
	#include <Resource/Mappings/Credits.h>
};

//Credits object
void Obj_Credits(Object *obj)
{
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine and set position
			obj->routine += 2;
			obj->pos.s.x = 0x120 + SCREEN_WIDEADD2;
			obj->pos.s.y = 0xF0 + SCREEN_TALLADD2;
			
			//Set object drawing information
			obj->mappings = map_credits;
			obj->tile.w = 0;
			obj->tile.s.pattern = 0x5A0;
			obj->frame = credits_num;
			obj->render.b = 0;
			obj->priority = 0;
			
			//Force "SONIC TEAM PRESENTS" text when in title screen
			if (gamemode == GameMode_Title)
			{
				//Display "SONIC TEAM PRESENTS" text
				obj->tile.w = 0;
				obj->tile.s.pattern = 0xA6;
				obj->frame = 10;
				
				//Hidden Japanese credits
				if (credits_cheat && jpad1_hold1 == (JPAD_A | JPAD_C | JPAD_B | JPAD_DOWN))
				{
					dry_palette_dup[2][0] = 0xEEE;
					dry_palette_dup[2][1] = 0x880;
					ObjectDelete(obj);
					break;
				}
			}
			
	//Fallthrough
		case 1: //Drawing
			DisplaySprite(obj);
			break;
	}
}
