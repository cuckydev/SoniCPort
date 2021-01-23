#include "Object.h"

static const uint8_t anim_psb[] = {
	#include <Resource/Animation/PSB.h>
};

static const uint8_t map_psb[] = {
	#include <Resource/Mappings/PSB.h>
};

void Obj_PSB(Object *obj)
{
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine and set position
			obj->routine += 2;
			obj->pos.s.x = 0xD0 + (PLANE_WIDEADD << 2);
			obj->pos.s.y = 0x130;
			
			//Set object drawing information
			obj->mappings = map_psb;
			obj->tile.w = 0;
			obj->tile.s.pattern = 0x200;
			
			//Handle different frames
			if (obj->frame >= 2)
			{
				obj->routine += 2;
				if (obj->frame == 3)
				{
					//Trademark
					obj->tile.w = 0;
					obj->tile.s.palette = 1;
					obj->tile.s.pattern = 0x510;
					obj->pos.s.x = 0x170 + (PLANE_WIDEADD << 2);
					obj->pos.s.y = 0xF8;
				}
				break;
			}
	//Fallthrough
		case 2: //Press Start Button
			AnimateSprite(obj, anim_psb);
			break;
		case 4: //TM or Sonic mask
			break;
	}
	
	DisplaySprite(obj);
}
