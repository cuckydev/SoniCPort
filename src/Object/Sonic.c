#include "Sonic.h"

#include "Object.h"
#include "Game.h"
#include "Level.h"

#include <string.h>

//Sonic mappings
static const uint8_t map_sonic[] = {
	#include <Resource/Mappings/Sonic.h>
};

//Sonic globals
int16_t sonspeed_max, sonspeed_acc, sonspeed_dec;

uint8_t sonframe_num, sonframe_chg;
uint8_t sgfx_buffer[SONIC_DPLC_SIZE];

uint8_t angle_buffer0;
uint8_t angle_buffer1;

int16_t track_sonic[0x40][2];
word_u track_pos;

//Sonic structures
typedef struct
{
	uint8_t air;                 //0x28
	uint8_t pad0[7];             //0x29 - 0x2F
	uint16_t flash_time;         //0x30
	uint16_t invincibility_time; //0x32
	uint16_t shoes_time;         //0x34
	uint8_t front_angle;         //0x36
	uint8_t back_angle;          //0x37
	uint8_t floor_clip;          //0x38
	uint8_t pad1[3];             //0x39 - 0x3B
	uint8_t jumping;             //0x3C
	uint8_t standing_obj;        //0x3D
	uint16_t control_lock;       //0x3E
} Scratch_Sonic;

//General Sonic state stuff
void Sonic_Display(Object *obj)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	//Handle invulnerability blinking
	uint16_t blink;
	if ((blink = scratch->flash_time))
	{
		scratch->flash_time--;
		if (blink & 4)
			DisplaySprite(obj);
	}
	else
	{
		DisplaySprite(obj);
	}
	
	//Handle invincibility timer
	if (scratch->invincibility_time)
	{
		if (--scratch->invincibility_time == 0)
		{
			//Restore music
			if (!(lock_screen || air < 12))
			{
				//TODO
			}
			
			//Clear flag
			invincibility = false;
		}
	}
	
	//Handle speed shoes timer
	if (scratch->shoes_time)
	{
		if (--scratch->shoes_time == 0)
		{
			//Restore Sonic's speed
			sonspeed_max = 0x600; //BUG: Water isn't checked
			sonspeed_acc = 0xC;
			sonspeed_dec = 0x80;
			
			//Clear flag and restore music
			shoes = false;
			//music	bgm_Slowdown,1,0,0	; run music at normal speed //TODO
		}
	}
}

void Sonic_RecordPosition(Object *obj)
{
	//Track current position
	int16_t *write = &track_sonic[0][0] + (track_pos.v >> 1);
	*write++ = obj->pos.l.x.f.u;
	*write++ = obj->pos.l.y.f.u;
	track_pos.f.l += 4;
}

//Sonic animation
static const uint8_t anim_sonic[] = {
	#include <Resource/Animation/Sonic.h>
};
#define GET_SONIC_ANISCR(x) (anim_sonic + ((anim_sonic[((x) << 1)] << 8) | (anim_sonic[((x) << 1) + 1] << 0)))

static void Sonic_AnimateReadFrame(Object *obj, const uint8_t *anim_script)
{
	//Read current animation command
	uint8_t cmd = anim_script[1 + obj->anim_frame];
	
	if (!(cmd & 0x80))
	{
		Anim_Next:
		//Set animation frame
		obj->frame = cmd;
		obj->anim_frame++;
	}
	else
	{
		if (++cmd == 0) //0xFF
		{
			//Restart animation
			obj->anim_frame = 0;
			cmd = anim_script[1];
			goto Anim_Next;
		}
		if (++cmd == 0) //0xFE
		{
			//Go back (next byte) frames
			obj->anim_frame -= anim_script[2 + obj->anim_frame];
			cmd = anim_script[1 + obj->anim_frame];
			goto Anim_Next;
		}
		if (++cmd == 0) //0xFD
		{
			//Change animation (falls through to the routine increment below)
			obj->anim = anim_script[2 + obj->anim_frame];
		}
	}
}

void Sonic_Animate(Object *obj)
{
	//Get animation script to use
	const uint8_t *anim_script = anim_sonic;
	
	//Check if animation changed
	uint8_t anim = obj->anim;
	if (anim != obj->prev_anim)
	{
		//Reset animation state
		obj->prev_anim = anim;
		obj->anim_frame = 0;
		obj->frame_time = 0;
	}
	
	//Get animation script to use
	anim <<= 1;
	anim_script += (anim_script[anim] << 8) | (anim_script[anim + 1] << 0);
	
	int8_t anim_wait = anim_script[0];
	if (anim_wait >= 0)
	{
		//Regular animation
		//Set render flip
		obj->render.f.x_flip = obj->status.p.f.x_flip;
		obj->render.f.y_flip = false;
		
		//Wait for current animation frame to end
		if (--obj->frame_time >= 0)
			return;
		obj->frame_time = anim_wait;
		
		//Read animation
		Sonic_AnimateReadFrame(obj, anim_script);
	}
	else
	{
		//Special animation (walking, rolling, running, etc.)
		if (++anim_wait == 0)
		{
			//Walking or running
			//Wait for current animation frame to end
			if (--obj->frame_time >= 0)
				return;
			
			//Get orienatation
			uint8_t angle = obj->angle;
			uint8_t flip = obj->status.p.f.x_flip;
			if (flip)
				angle = -angle;
			if ((angle += 0x10) & 0x80)
				flip ^= 3;
			
			//Set render flip
			obj->render.f.x_flip = (flip & 1) != 0;
			obj->render.f.y_flip = (flip & 2) != 0;
			
			//Branch to pushing if the pushing flag is set.. but not in the animation
			if (obj->status.p.f.pushing)
				goto Anim_Pushing;
			
			//Get rotation
			angle = (angle >> 4) & 6;
			
			//Get absolute speed
			int16_t abs_spd = obj->inertia;
			if (abs_spd < 0)
				abs_spd = -abs_spd;
			
			//Get script to use
			anim_script = GET_SONIC_ANISCR(SonAnimId_Run);
			if (abs_spd < 0x600)
			{
				anim_script = GET_SONIC_ANISCR(SonAnimId_Walk);
				angle += (angle >> 1);
			}
			
			//Get animation delay
			int16_t anim_spd = 0x800 - abs_spd;
			if (anim_spd < 0)
				anim_spd = 0;
			obj->frame_time = anim_spd >> 8;
			
			//Read animation
			Sonic_AnimateReadFrame(obj, anim_script);
			obj->frame += angle;
		}
		else if (++anim_wait == 0)
		{
			//Rolling
			//Get absolute speed
			int16_t abs_spd = obj->inertia;
			if (abs_spd < 0)
				abs_spd = -abs_spd;
			
			//Get script to use
			anim_script = GET_SONIC_ANISCR(SonAnimId_Roll2);
			if (abs_spd < 0x600)
				anim_script = GET_SONIC_ANISCR(SonAnimId_Roll);
			
			//Get animation delay
			int16_t anim_spd = 0x400 - abs_spd;
			if (anim_spd < 0)
				anim_spd = 0;
			obj->frame_time = anim_spd >> 8;
			
			//Set render flip
			obj->render.f.x_flip = obj->status.p.f.x_flip;
			obj->render.f.y_flip = false;
			
			//Read animation
			Sonic_AnimateReadFrame(obj, anim_script);
		}
		else
		{
			Anim_Pushing:;
			//Pushing
			//Get absolute speed
			int16_t abs_spd = obj->inertia;
			if (abs_spd < 0)
				abs_spd = -abs_spd;
			
			//Get animation delay
			int16_t anim_spd = 0x800 - abs_spd;
			if (anim_spd < 0)
				anim_spd = 0;
			obj->frame_time = anim_spd >> 6;
			
			//Get script to use
			anim_script = GET_SONIC_ANISCR(SonAnimId_Push);
			
			//Set render flip
			obj->render.f.x_flip = obj->status.p.f.x_flip;
			obj->render.f.y_flip = false;
			
			//Read animation
			Sonic_AnimateReadFrame(obj, anim_script);
		}
	}
}

//Sonic DPLCs
static const uint8_t art_sonic[] = {
	#include <Resource/Art/Sonic.h>
};

static const uint8_t dplc_sonic[] = {
	#include <Resource/Mappings/SonicDPLC.h>
};

void Sonic_LoadGfx(Object *obj)
{
	//Check if we're loading a new frame
	uint8_t frame = obj->frame;
	if (frame == sonframe_num)
		return;
	sonframe_num = frame;
	
	//Get DPLC script
	const uint8_t *dplc_script = dplc_sonic;
	frame <<= 1;
	dplc_script += (dplc_script[frame] << 8) | (dplc_script[frame + 1] << 0);
	
	//Read number of entries
	int8_t entries = (*dplc_script++) - 1;
	if (entries < 0)
		return;
	
	//Start reading data
	uint8_t *top = sgfx_buffer;
	sonframe_chg = true;
	
	do
	{
		//Read entry
		uint16_t tile = *dplc_script++;
		uint8_t tiles = tile >> 4;
		tile = ((tile << 8) | (*dplc_script++)) << 5;
		
		const uint8_t *fromp = art_sonic + tile;
		do
		{
			memcpy(top, fromp, 0x20);
			fromp += 0x20;
			top += 0x20;
		} while (tiles-- > 0);
	} while (entries-- > 0);
}

//Sonic object
void Obj_Sonic(Object *obj)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	//Run debug mode code while in debug mode
	if (debug_use)
	{
		//DebugMode();
		return;
	}
	
	//Run player routine
	switch (obj->routine)
	{
		case 0: //Initialiation
			//Increment routine
			obj->routine += 2;
			
			//Initialize collision size
			obj->y_rad = 19;
			obj->x_rad = 9;
			
			//Set object drawing information
			obj->mappings = map_sonic;
			obj->tile.w = 0;
			obj->tile.s.pattern = 0x780;
			obj->priority = 2;
			obj->width_pixels = 24;
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			
			//Initialize speeds
			sonspeed_max = 0x600;
			sonspeed_acc = 0xC;
			sonspeed_dec = 0x80;
	//Fallthrough
		case 2:
			//Enter debug mode
			if (debug_cheat && (jpad1_press1 & JPAD_B))
			{
				debug_use = true;
				lock_ctrl = false;
				break;
			}
			
			//Copy player controls if not locked
			if (!lock_ctrl)
			{
				jpad1_hold2  = jpad1_hold1;
				jpad1_press2 = jpad1_press1;
			}
			
			//Run player routine
			if (!(lock_multi & 1))
			{
				switch ((obj->status.p.f.in_ball << 2) | (obj->status.p.f.in_air << 1))
				{
					case 0: //Not in ball, not in air
						break;
					case 2: //Not in ball, in air
						break;
					case 4: //In ball, not in air
						break;
					case 6: //In ball, in air
						break;
				}
			}
			
			//Handle general player state stuff
			Sonic_Display(obj);
			Sonic_RecordPosition(obj);
			//Sonic_Water(obj); //TODO
			
			//Copy angle buffers
			scratch->front_angle = angle_buffer0;
			scratch->back_angle  = angle_buffer1;
			
			//Animate
			if (tunnel_mode)
			{
				if (!obj->anim)
					obj->prev_anim = obj->anim;
			}
			Sonic_Animate(obj);
			
			//Handle DPLCs
			Sonic_LoadGfx(obj);
			break;
	}
}
