#include "Object.h"

#include "Level.h"
#include "LevelScroll.h"
#include "LevelCollision.h"
#include "Object/Sonic.h"

//Monitor assets
static const uint8_t map_monitor[] = {
	#include "Resource/Mappings/Monitor.h"
};
static const uint8_t anim_monitor[] = {
	#include "Resource/Animation/Monitor.h"
};

//Monitor solid routine
static signed int Mon_SolidSides(Object *obj, uint16_t x_rad, uint16_t y_rad, int16_t *x_off, int16_t *y_off)
{
	//Check if player is in horizontal range
	*x_off = player->pos.l.x.f.u - obj->pos.l.x.f.u + x_rad;
	uint16_t x_dia = x_rad << 1;
	if (*x_off < 0 || *x_off > x_dia)
		return 0;
	
	//Check if player is in vertical range
	y_rad += player->y_rad;
	*y_off = player->pos.l.y.f.u - obj->pos.l.y.f.u + y_rad;
	uint16_t y_dia = y_rad << 1;
	
	if (*y_off < 0 || *y_off >= y_dia)
		return 0;
	
	//Check if player can collide with object
	if ((lock_multi & 0x80) || player->routine >= 6 || debug_use)
		return 0;
	
	//Shift x off when on other side of monitor
	if (x_rad < *x_off)
		*x_off -= x_dia;
	
	//Check if we're touching the top or not
	if (*y_off >= 0x10)
		return 1;
	
	x_rad = obj->width_pixels + 4;
	x_dia = x_rad << 1;
	
	int16_t x_off2 = player->pos.l.x.f.u - obj->pos.l.x.f.u + x_rad;
	if (x_off2 >= 0 && x_off2 < x_dia)
		return -1;
	
	return 1;
}

//Monitor object
void Obj_Monitor(Object *obj)
{
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Initialize collision
			obj->y_rad = 14;
			obj->x_rad = 14;
			
			//Set object drawing information
			obj->mappings = map_monitor;
			obj->tile = TILE_MAP(0, 0, 0, 0, 0x680);
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->priority = 3;
			obj->width_pixels = 15;
			
			//Check if monitor was destroyed
			objstate[obj->respawn_index] &= 0x7F;
			if (objstate[obj->respawn_index] & 1)
			{
				//Destroyed
				obj->routine = 8;
				obj->frame = 11;
				break; //Not drawn
			}
			
			//Set collision type and animation
			obj->col_type = 0x46;
			obj->anim = obj->scratch.u8[0]; //subtype
	//Fallthrough
		case 2: //Solid
			switch (obj->routine_sec)
			{
				case 0: //Solid
				{
					//Check if we're touching the monitor
					int16_t x_off, y_off;
					signed int solid = Mon_SolidSides(obj, 26, 15, &x_off, &y_off);
					
					if (solid && (player->ysp < 0 || player->anim != SonAnimId_Roll))
					{
						if (solid < 0)
						{
							//Stand on monitor
							player->pos.l.y.f.u -= y_off;
							Platform_SetStand(obj);
							obj->routine_sec = 2;
						}
						else
						{
							//Check if player shall be clipped by monitor sides
							if ((x_off > 0 && player->xsp >= 0) || (x_off < 0 && player->xsp < 0))
							{
								player->pos.l.x.f.u -= x_off;
								player->inertia = 0;
								player->xsp = 0;
							}
							
							//Start or stop pushing
							if (!player->status.p.f.in_air)
							{
								player->status.p.f.pushing = true;
								obj->status.o.f.player_push = true;
							}
							else
							{
								player->status.p.f.pushing = false;
								obj->status.o.f.player_push = false;
							}
						}
					}
					else
					{
						//Stop pushing
						if (obj->status.o.f.player_push)
						{
							player->anim = SonAnimId_Run; //Not Walk
							player->status.p.f.pushing = false;
							obj->status.o.f.player_push = false;
						}
					}
					break;
				}
				case 2: //Player standing
				{
					//Handle player standing on the monitor
					uint16_t x_rad = obj->width_pixels + 11;
					ExitPlatform(obj, x_rad, x_rad, NULL);
					if (!player->status.p.f.object_stand) //Checks player?
						obj->routine_sec = 0;
					else
						MvSonicOnPtfm(obj, obj->pos.l.y.f.u - 16, obj->pos.l.x.f.u);
					break;
				}
				default: //Falling
				{
					//Fall to the ground
					ObjectFall(obj);
					
					int16_t floor_dist = ObjFloorDist(obj, obj->pos.l.x.f.u);
					if (floor_dist < 0)
					{
						//Clip out of floor and stop falling
						obj->pos.l.y.f.u += floor_dist;
						obj->ysp = 0;
						obj->routine_sec = 0;
					}
					break;
				}
			}
	//Fallthrough
		case 6: //Animate
			AnimateSprite(obj, anim_monitor);
	//Fallthrough
		case 8: //Broken
			DisplaySprite(obj);
			
			//Delete once off-screen
			if (IS_OFFSCREEN(obj->pos.l.x.f.u))
				ObjectDelete(obj);
			break;
		case 4: //Breaking open
			//Increment routine
			obj->routine += 2;
			obj->col_type = 0x00;
			
			//Create item object
			Object *item = FindFreeObj();
			if (item != NULL)
			{
				item->type = ObjId_MonitorItem;
				item->pos.l.x.f.u = obj->pos.l.x.f.u;
				item->pos.l.y.f.u = obj->pos.l.y.f.u;
				item->anim = obj->anim;
			}
			
			//Create explosion object
			Object *explosion = FindFreeObj();
			if (explosion != NULL)
			{
				explosion->type = ObjId_Explosion;
				explosion->routine += 2;
				explosion->pos.l.x.f.u = obj->pos.l.x.f.u;
				explosion->pos.l.y.f.u = obj->pos.l.y.f.u;
			}
			
			//Mark as broken and draw
			objstate[obj->respawn_index] |= 1;
			
			obj->anim = 9;
			DisplaySprite(obj);
			break;
	}
}

//Monitor item object
static void ExtraLife()
{
	lives++;
	life_count++;
	//music	bgm_ExtraLife,1,0,0	; play extra life music TODO
}

void Obj_MonitorItem(Object *obj)
{
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_monitor;
			obj->tile = TILE_MAP(0, 0, 0, 0, 0x680);
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->render.f.raw_mappings = true;
			obj->priority = 3;
			obj->width_pixels = 8;
			
			//Initialize object state
			obj->ysp = -0x300;
			obj->frame = obj->anim + 2;
			
			const uint8_t *mapping_ind = (const uint8_t*)map_monitor + (obj->frame << 1);
			obj->mappings = map_monitor + ((mapping_ind[0] << 8) | (mapping_ind[1] << 0)) + 1;
	//Fallthrough
		case 2: //Move
			if (obj->ysp < 0)
			{
				//Move
				SpeedToPos(obj);
				obj->ysp += 0x18;
			}
			else
			{
				//Increment routine
				obj->routine += 2;
				obj->frame_time.w = 29;
				
				//Give item
				Scratch_Sonic *scratch = (Scratch_Sonic*)&player->scratch;
				
				switch (obj->anim) //I'm not doing an else if chain LOL
				{
					case 1: //Eggman
						break; //Nothing
					case 2: //Sonic
						ExtraLife();
						break;
					case 3: //Shoes
						shoes = true;
						scratch->shoes_time = 1200;
						sonspeed_max = 0xC00;
						sonspeed_acc = 0x18;
						sonspeed_dec = 0x80;
						//music	bgm_Speedup,1,0,0		; Speed	up the music TODO
						break;
					case 4: //Shield
						shield = true;
						objects[6].type = ObjId_ShieldInvincibility; //TODO
						//music	sfx_Shield,1,0,0	; play shield sound TODO
						break;
					case 5: //Invincibility
						invincibility = true;
						scratch->invincibility_time = 1200;
						objects[8].type = ObjId_ShieldInvincibility; //TODO
						objects[8].anim = 1;
						objects[9].type = ObjId_ShieldInvincibility;
						objects[9].anim = 2;
						objects[10].type = ObjId_ShieldInvincibility;
						objects[10].anim = 3;
						objects[11].type = ObjId_ShieldInvincibility;
						objects[11].anim = 4;
						if (!lock_screen)
							{;}//music	bgm_Invincible,1,0,0 ; play invincibility music TODO
						break;
					case 6: //Rings
						//Give 10 rings
						rings += 10;
						ring_count |= 1;
						
						//Give extra life
						if (rings >= 100 && !(life_num & 1))
						{
							life_num |= 1;
							ExtraLife();
						}
						else if (rings >= 200 && !(life_num & 2))
						{
							life_num |= 2;
							ExtraLife();
						}
						else
						{
							//music	sfx_Ring,1,0,0	; play ring sound TODO
						}
						break;
					case 7: //S
						break; //Nothing
				}
			}
			break;
		case 4: //Delete
			if (--obj->frame_time.w < 0)
				ObjectDelete(obj);
			break;
	}
	
	//Draw object
	DisplaySprite(obj);
}
