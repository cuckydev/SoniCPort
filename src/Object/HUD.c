#include "Object.h"

#include "Level.h"

//HUD assets
static const uint8_t map_hud[] = {
	#include "Resource/Mappings/HUD.h"
};

//HUD object
void Obj_HUD(Object *obj)
{
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set screen position
			obj->pos.s.x = 0x90;
			obj->pos.s.y = 0x108 + SCREEN_TALLADD2;
			
			//Initialize object drawing information
			obj->mappings = map_hud;
			obj->tile = TILE_MAP(0, 0, 0, 0, 0x6CA);
			obj->render.b = 0;
			obj->priority = 0;
	//Fallthrough
		case 2:
			#ifndef SCP_FIX_BUGS
				if (rings)
				{
					//Don't flash RINGS or TIME(?) and draw
					obj->frame = 0;
					DisplaySprite(obj);
				}
				else
				{
					//Determine frame and draw
					uint8_t frame = 0;
					if (!(frame_count & 8))
					{
						frame += 1; //Flash RINGS
						if (time.min == 9)
							frame += 2; //Flash TIME
					}
					obj->frame = frame;
					DisplaySprite(obj);
				}
			#else
				//Determine frame and draw
				uint8_t frame = 0;
				if (!(frame_count & 8))
				{
					if (!rings)
						frame += 1; //Flash RINGS
					if (time.min == 9)
						frame += 2; //Flash TIME
				}
				obj->frame = frame;
				DisplaySprite(obj);
			#endif
			break;
	}
}
