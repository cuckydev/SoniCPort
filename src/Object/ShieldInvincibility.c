#include "Object.h"

#include "Level.h"
#include "Object/Sonic.h"

//Shield and invincibility assets
static const uint8_t map_shieldinvincibility[] = {
	#include "Resource/Mappings/ShieldInvincibility.h"
};
static const uint8_t anim_shieldinvincibility[] = {
	#include "Resource/Animation/ShieldInvincibility.h"
};

//Shield and invincibility object
typedef struct
{
	uint8_t pad[0x8]; //0x28-0x2F
	uint8_t trail;    //0x30
} Scratch_Invincibility;

void Obj_ShieldInvincibility(Object *obj)
{
	Scratch_Invincibility *scratch = (Scratch_Invincibility*)&obj->scratch;
	
	switch (obj->routine)
	{
		case 0: //Initialiation
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_shieldinvincibility;
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->priority = 1;
			obj->width_pixels = 16;
			
			//Check if invincibility or shield
			if (!obj->anim)
			{
				//Shield
				obj->tile = TILE_MAP(0, 0, 0, 0, 0x541);
			}
			else
			{
				//Invincibility
				obj->routine += 2;
				obj->tile = TILE_MAP(0, 0, 0, 0, 0x55C);
			}
			break;
		case 2: //Shield
			//Check if shield should exist
			if (invincibility)
				break;
			if (!shield)
			{
				ObjectDelete(obj);
				break;
			}
			
			//Update shield position
			obj->pos.l.x.f.u = player->pos.l.x.f.u;
			obj->pos.l.y.f.u = player->pos.l.y.f.u;
			obj->status.b = player->status.b;
			
			//Animate and draw shield
			AnimateSprite(obj, anim_shieldinvincibility);
			DisplaySprite(obj);
			break;
		case 4: //Invincibility
			//Check if invincibility should exist
			if (!invincibility)
			{
				ObjectDelete(obj);
				break;
			}
			
			//Get trail position
			uint8_t track_ind = track_pos.f.l;
			uint8_t track_off = (obj->anim - 1) << 3;
			track_off = (track_off << 1) + track_off + 4;
			track_ind -= track_off + scratch->trail;
			if ((scratch->trail += 4) >= 0x18)
				scratch->trail = 0;
			
			//Set object position
			int16_t *pos = track_sonic[track_ind >> 2];
			obj->pos.l.x.f.u = *pos++;
			obj->pos.l.y.f.u = *pos++;
			obj->status.b = player->status.b;
			
			//Animate and draw object
			AnimateSprite(obj, anim_shieldinvincibility);
			DisplaySprite(obj);
			break;
	}
}
