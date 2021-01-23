#include "Object.h"

static const uint8_t map_titlesonic[] = {
	#include <Resource/Mappings/TitleSonic.h>
};

void Obj_TitleSonic(Object *obj)
{
	switch (obj->routine)
	{
		case 0:
			//Increment routine and initialize position
			obj->routine += 2;
			
			obj->pos.s.x = 0xF0 + (PLANE_WIDEADD << 3);
			obj->pos.s.y = 0xDE;
			
			//Set object drawing information
			obj->mappings = map_titlesonic;
			obj->tile.w = 0;
			obj->tile.s.palette = 1;
			obj->tile.s.pattern = 0x300;
			obj->priority = 1;
			
			//Initialize state
			obj->frame_time = 29;
			//animate
	//Fallthrough
		case 2:
			//Wait for timer to clear
			if ((--obj->frame_time) >= 0)
				return;
			
			//Increment routine
			obj->routine += 2;
			DisplaySprite(obj);
			break;
	}
}
