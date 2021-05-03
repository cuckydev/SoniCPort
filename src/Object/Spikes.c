#include "Object.h"

#include "Level.h"
#include "LevelScroll.h"
#include "Object/Sonic.h"

#include "Macros.h"

//Spikes assets
static const uint8_t map_spikes[] = {
	#include "Resource/Mappings/Spikes.h"
};

//Spikes object
typedef struct
{
	uint8_t subtype;   //0x28
	uint8_t pad1[0x7]; //0x29-0x2F
	int16_t orig_x;    //0x30
	int16_t orig_y;    //0x32
	word_u move;       //0x34
	uint16_t dir;      //0x36
	uint16_t timer;    //0x38
} Scratch_Spikes;

static const uint8_t spike_set[][2] = {
	{0, 20},
	{1, 10},
	{2,  4},
	{3, 28},
	{4, 64},
	{5, 16},
};

static void Spike_Hurt(Object *obj)
{
	//Check if player can be hurt
	if (invincibility)
		return;
	if (player->routine >= 4)
		return;
	
	//Hurt player
	player->pos.l.y.v -= player->ysp << 8;
	HurtSonic(player, obj);
}

static void Spike_Wait(Object *obj)
{
	Scratch_Spikes *scratch = (Scratch_Spikes*)&obj->scratch;
	
	//Wait for direction switch
	if (scratch->timer)
	{
		if (--scratch->timer)
			return;
		if (obj->render.f.on_screen)
			{;}//sfx	sfx_SpikesMove,0,0,0	; play "spikes moving" sound TODO
		return;
	}
	
	//Switch direction
	if (scratch->dir)
	{
		//Move to target position
		if ((scratch->move.v -= 0x800) >= 0x8000)
		{
			scratch->move.v = 0x0000;
			scratch->dir = 0;
			scratch->timer = 60;
		}
	}
	else
	{
		//Move to target position
		if ((scratch->move.v += 0x800) >= 0x2000)
		{
			scratch->move.v = 0x2000;
			scratch->dir = 1;
			scratch->timer = 60;
		}
	}
}

void Obj_Spikes(Object *obj)
{
	Scratch_Spikes *scratch = (Scratch_Spikes*)&obj->scratch;
	
	switch (obj->routine)
	{
		case 0: //Initialization
		{
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_spikes;
			obj->tile = TILE_MAP(0, 0, 0, 0, 0x51B);
			obj->render.f.align_fg = true;
			obj->priority = 4;
			
			//Initialize spikes
			const uint8_t *set = spike_set[scratch->subtype >> 4];
			scratch->subtype &= 0xF;
			
			obj->frame = *set++;
			obj->width_pixels = *set++;
			scratch->orig_x = obj->pos.l.x.f.u;
			scratch->orig_y = obj->pos.l.y.f.u;
		}
	//Fallthrough
		case 2: //Solid
		{
			//Spike movement
			switch (scratch->subtype)
			{
				case 0: //Don't move
					break;
				case 1: //Move up and down
					Spike_Wait(obj);
					obj->pos.l.y.f.u = scratch->orig_y + scratch->move.f.u;
					break;
				case 2: //Move left and right
					Spike_Wait(obj);
					obj->pos.l.x.f.u = scratch->orig_x + scratch->move.f.u;
					break;
			}
			
			//Handle spike solidity and damage
			int16_t height = 4;
			if (obj->frame == 5 || (height = 20, obj->frame == 1))
			{
				//Sideways
				signed int solid = SolidObject(obj, 27, height, height + 1, obj->pos.l.x.f.u, NULL, NULL);
				if (!obj->status.o.f.player_stand && solid == 1)
					Spike_Hurt(obj);
			}
			else
			{
				//Up/down
				signed int solid = SolidObject(obj, obj->width_pixels + 11, 16, 17, obj->pos.l.x.f.u, NULL, NULL);
				if (obj->status.o.f.player_stand || solid < 0)
					Spike_Hurt(obj);
			}
			
			//Draw and unload once offscreen
			DisplaySprite(obj);
			if (IS_OFFSCREEN(scratch->orig_x))
				ObjectDelete(obj);
			break;
		}
	}
}
