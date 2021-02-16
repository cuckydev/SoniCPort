#include "Object.h"
#include "Ring.h"

#include "Level.h"
#include "LevelCollision.h"
#include "MathUtil.h"

#include "Game.h"

//Ring loss object
static void Obj_RingLoss_SetupRing(Object *obj, int16_t *xsp, int16_t *ysp, word_u *angle, Object *ring)
{
	//Set object type and routine
	ring->type = ObjId_RingLoss;
	ring->routine += 2;
	
	//Set collision and position
	ring->y_rad = 8;
	ring->x_rad = 8;
	ring->pos.l.x.f.u = obj->pos.l.x.f.u;
	ring->pos.l.y.f.u = obj->pos.l.y.f.u;
	
	//Set object drawing information
	ring->mappings = map_ring;
	ring->tile.w = 0;
	ring->tile.s.palette = 1;
	ring->tile.s.pattern = 0x7B2;
	ring->render.b = 0;
	ring->render.f.align_fg = true;
	ring->priority = 3;
	
	//Set other object state stuff
	ring->col_type = 0x47;
	ring->width_pixels = 8;
	sprite_anim[3].time = -1;
	
	//Handle object angle and velocity
	if (!(angle->f.u & 0x80))
	{
		int16_t sin, cos;
		CalcSine(angle->f.l, &sin, &cos);
		*xsp = sin << angle->f.u;
		*ysp = cos << angle->f.u;
		
		if ((angle->f.l += 0x10) < 0x10)
		{
			if ((angle->v -= 0x80) >= 0xFF80)
				angle->v = 0x288;
		}
	}
	
	ring->xsp = *xsp;
	ring->ysp = *ysp;
	
	*xsp = -*xsp;
	angle->v = -angle->v;
}

void Obj_RingLoss(Object *obj)
{
	switch (obj->routine)
	{
		case 0: //Initialization
		{
			//Get how many rings to drop
			uint16_t drop = rings;
			if (drop >= 32)
				drop = 32; //Cap at 32
			drop--; //dbf
			
			//Drop rings
			int16_t xsp, ysp;
			word_u angle;
			angle.v = 0x0288;
			
			Obj_RingLoss_SetupRing(obj, &xsp, &ysp, &angle, obj);
			drop--;
			do
			{
				Object *ring = FindFreeObj();
				if (ring == NULL)
					break;
				Obj_RingLoss_SetupRing(obj, &xsp, &ysp, &angle, ring);
			} while (drop-- > 0);
			
			//Lose rings
			rings = 0;
			ring_count = 0x80;
			life_count = 0;
			//sfx	sfx_RingLoss,0,0,0	; play ring loss sound //TODO
		}
	//Fallthrough
		case 2: //Moving
			//Use animation frame
			obj->frame = sprite_anim[3].frame;
			
			//Fall
			SpeedToPos(obj);
			obj->ysp += 0x18;
			
			//Do floor collision (every 4 frames)
			if (obj->ysp >= 0 && ((ExecuteObjects_i + (vbla_count & 0xFF)) & 3) == 0)
			{
				int16_t floor_dist = ObjFloorDist(obj, obj->pos.l.x.f.u);
				if (floor_dist < 0)
				{
					//Bounce off floor
					obj->pos.l.y.f.u += floor_dist;
					obj->ysp = -(obj->ysp - (obj->ysp >> 2));
				}
			}
			
			//Delete once below level or animation is done
			if (!sprite_anim[3].time)
			{
				DeleteObject(obj);
				break;
			}
			if ((limit_btm2 + SCREEN_HEIGHT) < obj->pos.l.y.f.u)
			{
				DeleteObject(obj);
				break;
			}
			
			//Draw
			DisplaySprite(obj);
			break;
		case 4: //Collected
			//Change object state
			obj->routine += 2;
			obj->col_type = 0x00;
			obj->priority = 1;
			
			//Collect ring
			CollectRing();
	//Fallthrough
		case 6: //Sparkling
			//Animate and draw
			AnimateSprite(obj, anim_ring);
			DisplaySprite(obj);
			break;
		case 8: //Delete
			DeleteObject(obj);
			break;
	}
}
