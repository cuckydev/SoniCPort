#include "Object.h"

#include "Level.h"
#include "LevelCollision.h"

//Crabmeat assets
static const uint8_t map_crabmeat[] = {
	#include <Resource/Mappings/Crabmeat.h>
};
static const uint8_t anim_crabmeat[] = {
	#include <Resource/Animation/Crabmeat.h>
};

//Crabmeat object
typedef struct
{
	uint8_t pad0[8];    //0x28-0x2F
	int16_t time_delay; //0x30
	uint8_t crab_mode;  //0x32
} Scratch_Crabmeat;

static uint8_t Obj_Crabmeat_SetAni(Object *obj)
{
	if (obj->angle & 0x80)
	{
		if (obj->angle <= 0xFA)
			return (obj->status.o.f.x_flip ? 2 : 1);
	}
	else
	{
		if (obj->angle >= 0x06)
			return (obj->status.o.f.x_flip ? 1 : 2);
	}
	return 0;
}

void Obj_Crabmeat(Object *obj)
{
	Scratch_Crabmeat *scratch = (Scratch_Crabmeat*)&obj->scratch;
	
	int16_t floor_dist;
	
	switch (obj->routine)
	{
		case 0: //Initialization
			//Initialize collision
			obj->y_rad = 16;
			obj->x_rad = 8;
			
			//Initialize object drawing information
			obj->mappings = map_crabmeat;
			obj->tile = TILE_MAP(0, 0, 0, 0, 0x400);
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->priority = 3;
			
			//Initialize more stuff
			obj->col_type = 0x06;
			obj->width_pixels = 21;
			
			//Fall before enabling
			ObjectFall(obj);
			if ((floor_dist = ObjFloorDist(obj, obj->pos.l.x.f.u)) >= 0)
				break;
			
			//Clip and increment routine
			obj->pos.l.y.f.u += floor_dist;
			obj->ysp = 0;
			obj->angle = angle_buffer0;
			obj->routine += 2;
	//Fallthrough
		case 2: //Moving
			switch (obj->routine_sec)
			{
				case 0: //Wait to fire
					//Wait for timer to expire
					if (--scratch->time_delay >= 0)
						break;
					
					//Check if we should fire
					if (!obj->render.f.on_screen || ((scratch->crab_mode ^= 2) & 2))
					{
						//Turn around
						obj->routine_sec += 2;
						scratch->time_delay = 127;
						obj->xsp = 0x80;
						obj->anim = 3 + Obj_Crabmeat_SetAni(obj);
						if ((obj->status.o.f.x_flip ^= 1))
							obj->xsp = -obj->xsp;
					}
					else
					{
						//Fire
						scratch->time_delay = 59;
						obj->anim = 6;
						
						//Create projectiles
						Object *proj;
						
						proj = FindFreeObj();
						if (proj != NULL)
						{
							//Left
							proj->type = ObjId_Crabmeat;
							proj->routine = 6;
							proj->pos.l.x.f.u = obj->pos.l.x.f.u - 16;
							proj->pos.l.y.f.u = obj->pos.l.y.f.u;
							proj->xsp = -0x100;
						}
						
						proj = FindFreeObj();
						if (proj != NULL)
						{
							//Right
							proj->type = ObjId_Crabmeat;
							proj->routine = 6;
							proj->pos.l.x.f.u = obj->pos.l.x.f.u + 16;
							proj->pos.l.y.f.u = obj->pos.l.y.f.u;
							proj->xsp = 0x100;
						}
					}
					break;
				case 2: //Walking
					//Wait for timer to expire
					if (--scratch->time_delay >= 0)
					{
						//Move then perform collision
						SpeedToPos(obj);
						
						if (!((scratch->crab_mode ^= 1) & 1))
						{
							//Check if we're too close to edge
							floor_dist = ObjFloorDist(obj, obj->pos.l.x.f.u + (obj->status.o.f.x_flip ? -16 : 16));
							if (floor_dist >= -8 && floor_dist < 12)
								break;
						}
						else
						{
							//Clip out of floor
							floor_dist = ObjFloorDist(obj, obj->pos.l.x.f.u);
							obj->pos.l.y.f.u += floor_dist;
							obj->angle = angle_buffer0;
							obj->anim = 3 + Obj_Crabmeat_SetAni(obj);
							break;
						}
					}
					
					//Stop for a moment
					obj->routine_sec -= 2;
					scratch->time_delay = 59;
					obj->xsp = 0;
					obj->anim = Obj_Crabmeat_SetAni(obj);
					break;
			}
			
			//Animate, draw, and unload when off-screen
			AnimateSprite(obj, anim_crabmeat);
			RememberState(obj);
			break;
		case 4: //Delete
			ObjectDelete(obj);
			break;
		case 6: //Projectile initialization
			//Increment routine
			obj->routine += 2;
			
			//Initialize object drawing information
			obj->mappings = map_crabmeat;
			obj->tile = TILE_MAP(0, 0, 0, 0, 0x400);
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->priority = 3;
			
			//Initialize more stuff
			obj->col_type = 0x87;
			obj->width_pixels = 8;
			obj->ysp = -0x400;
			obj->anim = 7;
	//Fallthrough
		case 8: //Projectile move
			//Move and animate
			ObjectFall(obj);
			AnimateSprite(obj, anim_crabmeat);
			DisplaySprite(obj);
			
			//Delete if fallen below stage
			if (obj->pos.l.y.f.u >= limit_btm2 + SCREEN_HEIGHT)
				ObjectDelete(obj);
			break;
	}
}
