#include "Object.h"

#include "Level.h"

//Buzz Bomber assets
static const uint8_t map_buzz_bomber[] = {
	#include <Resource/Mappings/BuzzBomber.h>
};
static const uint8_t map_buzz_missile[] = {
	#include <Resource/Mappings/BuzzMissile.h>
};
static const uint8_t map_buzz_explode[] = {
	#include <Resource/Mappings/BuzzExplode.h>
};

static const uint8_t anim_buzz_bomber[] = {
	#include <Resource/Animation/BuzzBomber.h>
};
static const uint8_t anim_buzz_missile[] = {
	#include <Resource/Animation/BuzzMissile.h>
};

//Buzz Bomber's explosion
void Obj_BuzzExplode(Object *obj)
{
	
}

//Buzz Bomber's missile
typedef struct
{
	uint8_t subtype; //0x28
	uint8_t pad0[9]; //0x29 - 0x31
	int16_t time_delay; //0x32
	uint8_t pad1[0xC - sizeof(Object*)]; //This will break in 20 years when 128-bit processors are mainstream
	Object *parent; //0x3C assuming 32-bit address
} Scratch_BuzzMissile;

void Obj_BuzzMissile(Object *obj)
{
	Scratch_BuzzMissile *scratch = (Scratch_BuzzMissile*)&obj->scratch;
	
	switch (obj->routine)
	{
		case 0: //Initialization
			//Wait for timer to expire
			if (--scratch->time_delay < 0)
				break;
			
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_buzz_missile;
			obj->tile.w = 0;
			obj->tile.s.palette = 1;
			obj->tile.s.pattern = 0x444;
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->priority = 3;
			obj->width_pixels = 8;
			obj->status.o.f.flag2 = false;
			obj->status.o.f.player_stand = false;
			obj->status.o.f.flag4 = false;
			obj->status.o.f.player_push = false;
			obj->status.o.f.flag6 = false;
			obj->status.o.f.flag7 = false;
			
			//Check if we were created by a Newtron
			if (scratch->subtype)
			{
				obj->routine = 8;
				obj->col_type = 0x87;
			}
			
	//Fallthrough
		case 2: //Charging
			//Delete object if parent Buzz Bomber has exploded
			if (scratch->parent->type == ObjId_Explosion)
				DeleteObject(obj);
			
			//Animate and draw
			AnimateSprite(obj, anim_buzz_missile);
			DisplaySprite(obj);
			break;
		case 4: //Fired by Buzz Bomber
			//Check if we've 'hit Sonic' (disabled)
			if (!obj->status.o.f.flag7)
			{
				//Use fired collision and animation
				obj->col_type = 0x87;
				obj->anim = 1;
				
				//Move and animate
				SpeedToPos(obj);
				AnimateSprite(obj, anim_buzz_missile);
				DisplaySprite(obj);
				
				//Delete if fallen below stage
				if (obj->pos.l.y.f.u >= limit_btm2 + SCREEN_HEIGHT)
					DeleteObject(obj);
			}
			else
			{
				//Explode
				obj->type = ObjId_BuzzExplode;
				obj->routine = 0;
				Obj_BuzzExplode(obj);
			}
			break;
		case 6: //Delete
			DeleteObject(obj);
			break;
		case 8: //Fired by Newtron
			//Delete once off-screen
			if (!obj->render.f.on_screen)
			{
				DeleteObject(obj);
				break;
			}
			
			//Move and animate
			SpeedToPos(obj);
			AnimateSprite(obj, anim_buzz_missile);
			DisplaySprite(obj);
			break;
	}
}

//Buzz Bomber object
typedef struct
{
	uint8_t pad0[10];    //0x28-0x31
	int16_t time_delay;  //0x32
	int16_t buzz_status; //0x34
} Scratch_BuzzBomber;

void Obj_BuzzBomber(Object *obj)
{
	Scratch_BuzzBomber *scratch = (Scratch_BuzzBomber*)&obj->scratch;
	
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set object drawing information
			obj->mappings = map_buzz_bomber;
			obj->tile.w = 0;
			obj->tile.s.pattern = 0x444;
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->priority = 3;
			obj->col_type = 0x08;
			obj->width_pixels = 24;
			
	//Fallthrough
		case 2: //Moving
			switch (obj->routine_sec)
			{
				case 0: //Moving
					//Wait for timer to expire
					if (--scratch->time_delay >= 0)
						break;
					
					//Start firing missile if near Sonic
					if (!(scratch->buzz_status & 2))
					{
						//Not near Sonic
						obj->routine_sec += 2;
						scratch->time_delay = 127; //It's a word, Yuji!
						obj->xsp = 0x400;
						obj->anim = 1;
						if (!obj->status.o.f.x_flip)
							obj->xsp = -obj->xsp;
					}
					else
					{
						//Near Sonic
						//Create missile object
						Object *missile = FindFreeObj();
						if (missile == NULL)
							break;
						Scratch_BuzzMissile *mscratch = (Scratch_BuzzMissile*)&missile->scratch;
						
						missile->type = ObjId_BuzzMissile;
						missile->pos.l.x.f.u = obj->pos.l.x.f.u;
						missile->pos.l.y.f.u = obj->pos.l.y.f.u + 28;
						missile->ysp = 0x200;
						missile->xsp = 0x200;
						if (!obj->status.o.f.x_flip)
						{
							missile->pos.l.x.f.u -= 24;
							missile->xsp = -missile->xsp;
						}
						else
						{
							missile->pos.l.x.f.u += 24;
						}
						missile->status.o.b = obj->status.o.b;
						mscratch->time_delay = 14;
						mscratch->parent = obj;
						
						//Set our state
						scratch->buzz_status = 1;
						scratch->time_delay = 59;
						obj->anim = 2;
					}
					break;
				case 2: //Check near Sonic
					//Wait for timer to expire
					if (--scratch->time_delay >= 0)
					{
						//Move and check if we're near Sonic
						SpeedToPos(obj);
						
						if (scratch->buzz_status)
							break;
						
						//Get X difference and check if we're close enough to Sonic
						uint16_t x_off =
							(obj->pos.l.x.f.u < player->pos.l.x.f.u) ?
							(player->pos.l.x.f.u - obj->pos.l.x.f.u) : 
							(obj->pos.l.x.f.u - player->pos.l.x.f.u);
						
						if (x_off >= 0x60 || !obj->render.f.on_screen)
							break;
						
						//Set that we're near Sonic
						scratch->buzz_status = 2;
						scratch->time_delay = 29;
					}
					else
					{
						//Change direction
						scratch->buzz_status = 0;
						obj->status.o.f.x_flip ^= 1;
						scratch->time_delay = 59;
					}
					
					//Stop
					obj->routine_sec -= 2;
					obj->xsp = 0;
					obj->anim = 0;
					break;
			}
			
			//Animate, draw, and unload when off-screen
			AnimateSprite(obj, anim_buzz_bomber);
			RememberState(obj);
			break;
		case 4: //Delete
			DeleteObject(obj);
			break;
	}
}
