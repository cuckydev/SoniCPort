#include "Object.h"

//Chopper assets
static const uint8_t map_chopper[] = {
	#include "Resource/Mappings/Chopper.h"
};
static const uint8_t anim_chopper[] = {
	#include "Resource/Animation/Chopper.h"
};

//Chopper object
typedef struct
{
	uint8_t pad[8]; //0x28-0x2F
	int16_t orig_y; //0x30
} Scratch_Chopper;

void Obj_Chopper(Object *obj)
{
	Scratch_Chopper *scratch = (Scratch_Chopper*)&obj->scratch;
	
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_chopper;
			obj->tile = TILE_MAP(0, 0, 0, 0, 0x47B);
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->priority = 4;
			
			//Initialize object
			obj->col_type = 0x09;
			obj->width_pixels = 16;
			obj->ysp = -0x700;
			scratch->orig_y = obj->pos.l.y.f.u;
	//Fallthrough
		case 2: //Moving
			//Animate and move
			AnimateSprite(obj, anim_chopper);
			SpeedToPos(obj);
			obj->ysp += 0x18;
			
			//Check if we've fallen back to original Y
			if (scratch->orig_y < obj->pos.l.y.f.u)
			{
				obj->pos.l.y.f.u = scratch->orig_y;
				obj->ysp = -0x700;
			}
			
			//Set animation
			obj->anim = 1;
			
			if ((scratch->orig_y - 0xC0) < obj->pos.l.y.f.u)
			{
				obj->anim = 0;
				if (obj->ysp >= 0)
					obj->anim = 2;
			}
			break;
	}
	
	//Draw and unload once off-screen
	RememberState(obj);
}
