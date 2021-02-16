#include "Object.h"

#include "Level.h"
#include "LevelCollision.h"

//Newtron assets
static const uint8_t map_newtron[] = {
	#include <Resource/Mappings/Newtron.h>
};
static const uint8_t anim_newtron[] = {
	#include <Resource/Animation/Newtron.h>
};

//Newtron object
typedef struct
{
	uint8_t subtype;  //0x28
	uint8_t pad[0x9]; //0x29-0x31
	uint8_t fired;    //0x32
} Scratch_Newtron;

void Obj_Newtron(Object *obj)
{
	Scratch_Newtron *scratch = (Scratch_Newtron*)&obj->scratch;
	
	int16_t floor_dist;
	
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_newtron;
			obj->tile.w = 0;
			obj->tile.s.pattern = 0x49B;
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->priority = 4;
			obj->width_pixels = 20;
			obj->y_rad = 16;
			obj->x_rad = 8;
	//Fallthrough
		case 2:
			switch (obj->routine_sec)
			{
				case 0: //Check if near Sonic
				{
					//Get X difference and check if we're close enough to Sonic
					uint16_t x_off;
					if (obj->pos.l.x.f.u < player->pos.l.x.f.u)
					{
						obj->status.o.f.x_flip = true;
						x_off = player->pos.l.x.f.u - obj->pos.l.x.f.u;
					}
					else
					{
						obj->status.o.f.x_flip = false;
						x_off = obj->pos.l.x.f.u - player->pos.l.x.f.u;
					}
					if (x_off >= 0x80)
						break;
					
					//Setup next state depending on if we're green or blue
					obj->routine_sec += 2;
					obj->anim = 1;
					
					if (scratch->subtype)
					{
						obj->tile.w = 0;
						obj->tile.s.palette = 1;
						obj->tile.s.pattern = 0x49B;
						obj->routine_sec = 8;
						obj->anim = 4;
					}
					break;
				}
				case 2: //Flying Newtron
				{
					//Wait until animation has finished
					if (obj->frame < 4)
					{
						if (obj->pos.l.x.f.u < player->pos.l.x.f.u)
							obj->status.o.f.x_flip = true;
						else
							obj->status.o.f.x_flip = false;
						break;
					}
					
					//Change collision type once frame is 1?
					if (obj->frame == 1)
						obj->col_type = 0x0C; //Different collision type than below
					
					//Fall onto ground
					ObjectFall(obj);
					
					floor_dist = ObjFloorDist(obj, obj->pos.l.x.f.u);
					if (floor_dist < 0)
					{
						//Clip and change state
						obj->pos.l.y.f.u += floor_dist;
						obj->ysp = 0;
						obj->routine_sec += 2;
						obj->anim = 2;
						if (obj->tile.s.palette & 1) //Condition never met
							obj->anim++;
						obj->col_type = 0x0D;
						obj->xsp = 0x200;
						if (!obj->status.o.f.x_flip)
							obj->xsp = -obj->xsp;
					}
					break;
				}
				case 4: //Fly along floor
					//Move and check floor
					SpeedToPos(obj);
					
					floor_dist = ObjFloorDist(obj, obj->pos.l.x.f.u);
					if (floor_dist >= -8 && floor_dist < 12)
					{
						obj->pos.l.y.f.u += floor_dist;
						break;
					}
					
					//Didn't find floor, start flying without collision
					obj->routine_sec += 2;
					break;
				case 6: //Fly in mid-air
					SpeedToPos(obj);
					break;
				case 8: //Shooting Newtron
					//Change collision type once frame is 1
					if (obj->frame == 1)
						obj->col_type = 0x0C;
					
					//Check if we should fire
					if (obj->frame == 2 && !scratch->fired)
					{
						//Fire missile
						scratch->fired = true;
						
						Object *msl = FindFreeObj();
						if (msl == NULL)
							break;
						
						msl->type = ObjId_BuzzMissile;
						msl->pos.l.x.f.u = obj->pos.l.x.f.u;
						msl->pos.l.y.f.u = obj->pos.l.y.f.u - 8;
						if (obj->status.o.f.x_flip)
						{
							msl->xsp = 0x200;
							msl->pos.l.x.f.u += 20;
						}
						else
						{
							msl->xsp = -0x200;
							msl->pos.l.x.f.u -= 20;
						}
						msl->status.o.b = obj->status.o.b;
						msl->scratch.u8[0] = 1; //subtype
					}
					break;
			}
			
			//Animate, draw, and unload once off-screen
			AnimateSprite(obj, anim_newtron);
			RememberState(obj);
			break;
		case 4: //Delete
			ObjectDelete(obj);
			break;
	}
}
