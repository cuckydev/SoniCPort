#include "Object.h"

#include "LevelScroll.h"

#include <Macros.h>

//GHZ rock assets
static const uint8_t map_ghz_rock[] = {
	#include <Resource/Mappings/GHZRock.h>
};

//GHZ rock object
void Obj_GHZRock(Object *obj)
{
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_ghz_rock;
			obj->tile = TILE_MAP(0, 3, 0, 0, 0x3D0);
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->width_pixels = 19;
			obj->priority = 4;
	//Fallthrough
		case 2: //Solid
			//Act as solid object and draw
			SolidObject(obj, 27, 16, 16, obj->pos.l.x.f.u);
			DisplaySprite(obj);
			
			//Delete once off-screen
			if (IS_OFFSCREEN(obj->pos.l.x.f.u))
				ObjectDelete(obj);
			break;
	}
}
