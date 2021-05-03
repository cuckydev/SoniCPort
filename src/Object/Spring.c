#include "Object.h"

#include "Level.h"
#include "LevelScroll.h"
#include "Sonic.h"

//Spring assets
static const uint8_t map_spring[] = {
	#include "Resource/Mappings/Spring.h"
};
static const uint8_t anim_spring[] = {
	#include "Resource/Animation/Spring.h"
};

//Spring object
typedef struct
{
	uint8_t subtype; //0x28
	uint8_t pad[7];  //0x29-0x2F
	int16_t power;   //0x30
} Scratch_Spring;

static const int16_t spring_power[] = {-0x1000, -0xA00};

void Obj_Spring(Object *obj)
{
	Scratch_Spring *scratch = (Scratch_Spring*)&obj->scratch;
	
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_spring;
			obj->tile = TILE_MAP(0, 0, 0, 0, 0x523);
			obj->render.f.align_fg = true;
			obj->width_pixels = 16;
			obj->priority = 4;
			
			//Initialize based off of subtype
			if (scratch->subtype & 0x10)
			{
				//Left/right
				obj->routine = 8;
				obj->anim = 1;
				obj->frame = 3;
				obj->tile = TILE_MAP(0, 0, 0, 0, 0x533);
				obj->width_pixels = 8;
			}
			if (scratch->subtype & 0x20)
			{
				//Down
				obj->routine = 14;
				obj->status.o.f.y_flip = true;
			}
			
			//Check if yellow spring
			if (scratch->subtype & 0x02)
				obj->tile |= TILE_MAP(0, 1, 0, 0, 0);
			scratch->power = spring_power[(scratch->subtype & 0xF) >> 1];
			break;
		case 2: //Up
			//Act as solid and bounce when Sonic hits top
			SolidObject(obj, 27, 8, 16, obj->pos.l.x.f.u, NULL, NULL);
			if (!obj->routine_sec)
				break;
			
			//Increment routine
			obj->routine += 2;
			
			//Launch Sonic
			player->pos.l.y.f.u += 8;
			player->ysp = scratch->power;
			player->status.p.f.in_air = true;
			player->status.p.f.object_stand = false;
			player->anim = SonAnimId_Spring;
			player->routine = 2;
			
			//Reset object state
			obj->status.o.f.player_stand = false;
			obj->routine_sec = 0;
			//sfx	sfx_Spring,0,0,0	; play spring sound //TODO
	//Fallthrough
		case 4: //Up bouncing
			AnimateSprite(obj, anim_spring);
			break;
		case 6: //Up reset
			obj->prev_anim = 1;
			obj->routine -= 4;
			break;
		case 8: //Left/right
			//Act as solid and bounce when Sonic hits side
			SolidObject(obj, 19, 14, 15, obj->pos.l.x.f.u, NULL, NULL);
			if (obj->routine == 2)
				obj->routine = 8;
			if (!obj->status.o.f.player_push)
				break;
			
			//Increment routine
			obj->routine += 2;
			
			//Launch Sonic
			if (obj->status.o.f.x_flip)
			{
				player->xsp = scratch->power;
				player->pos.l.x.f.u += 8;
			}
			else
			{
				player->xsp = -scratch->power;
				player->pos.l.x.f.u -= 8;
			}
			Scratch_Sonic *sscratch = (Scratch_Sonic*)&player->scratch;
			sscratch->control_lock = 15;
			player->inertia = player->xsp;
			player->status.p.f.x_flip ^= 1;
			if (!player->status.p.f.in_ball)
				player->anim = SonAnimId_Walk;
			
			//Reset object state
			obj->status.o.f.player_push = false;
			player->status.p.f.pushing = false;
			//sfx	sfx_Spring,0,0,0	; play spring sound //TODO
			break;
		case 10: //Left/right bouncing
			AnimateSprite(obj, anim_spring);
			break;
		case 12: //Left/right reset
			obj->prev_anim = 2;
			obj->routine -= 4;
			break;
		case 14:; //Down
			//Act as solid and bounce when Sonic hits top
			int16_t y_off;
			SolidObject(obj, 27, 8, 16, obj->pos.l.x.f.u, NULL, &y_off);
			if (obj->routine == 2)
				obj->routine = 14;
			if (!obj->routine_sec || y_off >= 0)
				break;
			
			//Increment routine
			obj->routine += 2;
			
			//Launch Sonic
			player->pos.l.y.f.u -= 8;
			player->ysp = -scratch->power;
			player->status.p.f.in_air = true;
			player->status.p.f.object_stand = false;
			player->anim = SonAnimId_Spring;
			player->routine = 2;
			
			//Reset object state
			obj->status.o.f.player_stand = false;
			obj->routine_sec = 0;
			//sfx	sfx_Spring,0,0,0	; play spring sound //TODO
	//Fallthrough
		case 16: //Down bouncing
			AnimateSprite(obj, anim_spring);
			break;
		case 18: //Down reset
			obj->prev_anim = 1;
			obj->routine -= 4;
			break;
	}
	
	//Draw and delete once off-screen
	DisplaySprite(obj);
	if (IS_OFFSCREEN(obj->pos.l.x.f.u))
		ObjectDelete(obj);
}
