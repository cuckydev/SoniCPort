#include "Object.h"

#include "Level.h"
#include "LevelScroll.h"
#include "Sonic.h"

#include "Macros.h"

//GHZ edge assets
static const uint8_t map_ghz_edge[] = {
	#include "Resource/Mappings/GHZEdge.h"
};

//GHZ edge object
static int Obj44_SolidWall2(Object *obj, uint16_t x_rad, uint16_t y_rad, int16_t *x_off, int16_t *y_off)
{
	//Check if we're touching horizontally
	*x_off = player->pos.l.x.f.u - obj->pos.l.x.f.u + x_rad;
	uint16_t x_dia = x_rad << 1;
	
	if (*x_off < 0 || *x_off > x_dia)
		return 0;
	
	//Check if we're touching vertically
	y_rad += player->y_rad;
	
	*y_off = player->pos.l.y.f.u - obj->pos.l.y.f.u + 4 + y_rad;
	uint16_t y_dia = y_rad << 1;
	
	if (*y_off < 0 || *y_off >= y_dia)
		return 0;
	
	//Check if we can touch
	if ((lock_multi & 0x80) || player->routine >= 6 || debug_use)
		return 0;
	
	//Get X clip
	uint16_t x_clip = *x_off;
	if (x_rad < *x_off)
	{
		*x_off -= x_dia;
		x_clip = -*x_off;
	}
	
	//Get Y clip
	uint16_t y_clip = *y_off;
	if (y_rad < *y_off)
	{
		*y_off -= y_dia;
		y_clip = -*y_off;
	}
	
	if (x_clip <= y_clip)
		return 1;
	else
		return -1;
}

static void Obj44_SolidWall(Object *obj, uint16_t x_rad, uint16_t y_rad)
{
	int16_t x_off, y_off;
	
	int res = Obj44_SolidWall2(obj, x_rad, y_rad, &x_off, &y_off);
	if (res > 0)
	{
		//Hit horizontal
		if (x_off > 0)
		{
			if (player->xsp >= 0)
			{
				player->pos.l.x.f.u -= x_off;
				player->inertia = 0;
				player->xsp = 0;
			}
		}
		else if (x_off < 0)
		{
			if (player->xsp < 0)
			{
				player->pos.l.x.f.u -= x_off;
				player->inertia = 0;
				player->xsp = 0;
			}
		}
		
		//Update push flags
		if (!player->status.p.f.in_air)
		{
			player->status.p.f.pushing = true;
			obj->status.o.f.player_push = true;
		}
		else
		{
			obj->status.o.f.player_push = false;
			player->status.p.f.pushing = false;
		}
		return;
	}
	else if (res < 0)
	{
		//Hit vertical
		if (player->ysp >= 0 || y_off >= 0)
			return;
		player->pos.l.y.f.u -= y_off;
		player->ysp = 0;
	}
	
	//Clear pushing state
	if (obj->status.o.f.player_push)
	{
		player->anim = SonAnimId_Run; //Not Walk
		obj->status.o.f.player_push = false;
		player->status.p.f.pushing = false;
	}
}

void Obj_GHZEdge(Object *obj)
{
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_ghz_edge;
			obj->tile = TILE_MAP(0, 2, 0, 0, 0x34C);
			obj->render.f.align_fg = true;
			obj->width_pixels = 8;
			obj->priority = 6;
			obj->frame = obj->scratch.u8[0]; //subtype
			
			//Check if object should be solid
			if (!(obj->frame & 0x10))
			{
				//Solid
				Obj44_SolidWall(obj, 0x13, 0x28);
			}
			else
			{
				//Non-solid
				obj->frame &= ~0x10;
				obj->routine += 2;
			}
			break;
		case 2: //Solid and draw
			Obj44_SolidWall(obj, 0x13, 0x28);
			break;
		case 4: //Draw
			break;
		default:
			return;
	}
	
	DisplaySprite(obj);
	if (IS_OFFSCREEN(obj->pos.l.x.f.u))
		ObjectDelete(obj);
}
