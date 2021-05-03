#include "Object.h"

#include "Level.h"
#include "LevelScroll.h"
#include "LevelCollision.h"
#include "Game.h"
#include "MathUtil.h"

//Ring assets
static const uint8_t anim_ring[] = {
	#include "Resource/Animation/Ring.h"
};
static const uint8_t map_ring[] = {
	#include RES_REV(Mappings/Ring)
};

//Ring object
typedef struct
{
	uint8_t subtype;  //0x28
	uint8_t pad[0x9]; //0x29-0x31
	int16_t base_x;   //0x32
	uint8_t index;    //0x34
} Scratch_Ring;

static const int8_t ring_pos[16][2] = {
	{ 0x10, 0x00},
	{ 0x18, 0x00},
	{ 0x20, 0x00},
	{ 0x00, 0x10},
	{ 0x00, 0x18},
	{ 0x00, 0x20},
	{ 0x10, 0x10},
	{ 0x18, 0x18},
	{ 0x20, 0x20},
	{-0x10, 0x10},
	{-0x18, 0x18},
	{-0x20, 0x20},
	{ 0x10, 0x08},
	{ 0x18, 0x10},
	{-0x10, 0x08},
	{-0x18, 0x10},
};

static bool Obj_Ring_ShiftChk(uint8_t *state)
{
	if (*state & 1)
	{
		*state >>= 1;
		return true;
	}
	*state >>= 1;
	return false;
}

static void Obj_Ring_SetupRing(Object *obj, uint8_t index, int16_t x, int16_t y, Object *ring)
{
	Scratch_Ring *scratch = (Scratch_Ring*)&ring->scratch;
	
	//Set type and routine
	ring->type = ObjId_Ring;
	ring->routine += 2;
	
	//Set position
	ring->pos.l.x.f.u = x;
	scratch->base_x = obj->pos.l.x.f.u;
	ring->pos.l.y.f.u = y;
	
	//Set object drawing information
	ring->mappings = map_ring;
	ring->tile = TILE_MAP(0, 1, 0, 0, 0x7B2);
	ring->render.b = 0;
	ring->render.f.align_fg = true;
	ring->priority = 2;
	
	//Set other state stuff
	ring->col_type = 0x47;
	ring->width_pixels = 8;
	ring->respawn_index = obj->respawn_index;
	scratch->index = index;
}

static void ExtraLife()
{
	lives++;
	life_count++;
}

static void CollectRing()
{
	//Increment ring count
	rings++;
	ring_count |= 1;
	
	//Check if we should get an extra life
	//TODO: sound
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
}

void Obj_Ring(Object *obj)
{
	Scratch_Ring *scratch = (Scratch_Ring*)&obj->scratch;
	
	switch (obj->routine)
	{
		case 0: //Initialization
		{
			//Get spawning information
			uint8_t *statep = &objstate[obj->respawn_index];
			uint8_t state = *statep;
			
			uint8_t num = scratch->subtype & 7; //subtype
			if (num == 7)
				num = 6;
			
			int16_t inc_x = ring_pos[scratch->subtype >> 4][0];
			int16_t inc_y = ring_pos[scratch->subtype >> 4][1];
			
			int16_t x = obj->pos.l.x.f.u;
			int16_t y = obj->pos.l.y.f.u;
			
			//Spawn rings
			uint16_t index = 0;
			
			if (!Obj_Ring_ShiftChk(&state))
			{
				*statep &= 0x7F;
				Obj_Ring_SetupRing(obj, index, x, y, obj);
			}
			
			for (
				x += inc_x, y += inc_y, index++;
				num-- > 0;
				x += inc_x, y += inc_y, index++)
			{
				if (!Obj_Ring_ShiftChk(&state))
				{
					*statep &= 0x7F;
					Object *ring = FindFreeObj();
					if (ring != NULL)
						Obj_Ring_SetupRing(obj, index, x, y, ring);
				}
			}
			
			if (*statep & 1)
			{
				ObjectDelete(obj);
				return;
			}
		}
	//Fallthrough
		case 2: //Animate
			//Draw and unload once off-screen
			obj->frame = sprite_anim[1].frame;
			DisplaySprite(obj);
			if (IS_OFFSCREEN(scratch->base_x))
				ObjectDelete(obj);
			break;
		case 4: //Collected
			//Change object state
			obj->routine += 2;
			obj->col_type = 0x00;
			obj->priority = 1;
			
			//Collect ring and mark as collected
			CollectRing();
			objstate[obj->respawn_index] |= (1 << scratch->index);
	//Fallthrough
		case 6: //Sparkling
			//Animate and draw
			AnimateSprite(obj, anim_ring);
			DisplaySprite(obj);
			break;
		case 8: //Delete
			ObjectDelete(obj);
			break;
	}
}

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
	ring->tile = TILE_MAP(0, 1, 0, 0, 0x7B2);
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
			while (drop-- > 0)
			{
				Object *ring = FindFreeObj();
				if (ring == NULL)
					break;
				Obj_RingLoss_SetupRing(obj, &xsp, &ysp, &angle, ring);
			}
			
			//Lose rings
			rings = 0;
			ring_count = 0x80;
			life_num = 0;
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
				ObjectDelete(obj);
				break;
			}
			if ((limit_btm2 + SCREEN_HEIGHT) < obj->pos.l.y.f.u)
			{
				ObjectDelete(obj);
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
			ObjectDelete(obj);
			break;
	}
}
