#include "Object.h"

//GHZ bridge assets
static const uint8_t map_ghz_bridge[] = {
	#include <Resource/Mappings/GHZBridge.h>
};

//GHZ bridge object
typedef struct
{
	uint8_t subtype;    //0x28
	uint8_t pad0[0x13]; //0x29-0x3B
	int16_t x3C;        //0x3C
	uint8_t x3E;        //0x3E
} Scratch_GHZBridge;

void Obj_GHZBridge(Object *obj)
{
	Scratch_GHZBridge *scratch = (Scratch_GHZBridge*)&obj->scratch;
	
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_ghz_bridge;
			obj->tile.w = 0;
			obj->tile.s.palette = 2;
			obj->tile.s.pattern = 0x38E;
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			
			break;
	}
	DisplaySprite(obj);
}
