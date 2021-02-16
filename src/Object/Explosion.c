#include "Object.h"

//Explosion assets
static const uint8_t map_explosion[] = {
	#include <Resource/Mappings/Explosion.h>
};

//Explosion object
void Obj_Explosion(Object *obj)
{
	switch (obj->routine)
	{
		case 0: //Spawn an animal
		{
			//Increment routine and create animal object
			obj->routine += 2;
			
			Object *animal = FindFreeObj();
			if (animal != NULL)
			{
				animal->type = ObjId_Animal;
				animal->pos.l.x.f.u = obj->pos.l.x.f.u;
				animal->pos.l.y.f.u = obj->pos.l.y.f.u;
				animal->scratch.u16[0xB] = obj->scratch.u16[0xB];
			}
		}
	//Fallthrough
		case 2: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_explosion;
			obj->tile.w = 0;
			obj->tile.s.pattern = 0x5A0;
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->priority = 1;
			
			//Initialize state
			obj->col_type = 0;
			obj->width_pixels = 12;
			obj->frame_time.b = 7;
			obj->frame = 0;
			
			//sfx	sfx_BreakItem,0,0,0	; play breaking enemy sound //TODO
	//Fallthrough
		case 4: //Animate
			//Run animation and delete once done
			if (--obj->frame_time.b < 0)
			{
				obj->frame_time.b = 7;
				if (++obj->frame == 5)
				{
					ObjectDelete(obj);
					break;
				}
			}
			
			//Draw object
			DisplaySprite(obj);
			break;
	}
}
