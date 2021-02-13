#include "Sonic.h"

#include "Object.h"
#include "Game.h"
#include "Level.h"
#include "LevelScroll.h"
#include "LevelCollision.h"
#include "MathUtil.h"
#include "PLC.h"

#include <string.h>

//Sonic mappings
static const uint8_t map_sonic[] = {
	#include <Resource/Mappings/Sonic.h>
};

//Sonic globals
int16_t sonspeed_max, sonspeed_acc, sonspeed_dec;

uint8_t sonframe_num, sonframe_chg;
uint8_t sgfx_buffer[SONIC_DPLC_SIZE];

int16_t track_sonic[0x40][2];
word_u track_pos;

uint8_t dbg_ang0, dbg_ang1, dbg_ang2, dbg_ang3; //0xFFEC-0xFFEF

//General Sonic state stuff
static void Sonic_Display(Object *obj)
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

static void Sonic_RecordPosition(Object *obj)
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

static void Sonic_Animate(Object *obj)
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
		obj->frame_time.b = 0;
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
		if (--obj->frame_time.b >= 0)
			return;
		obj->frame_time.b = anim_wait;
		
		//Read animation
		Sonic_AnimateReadFrame(obj, anim_script);
	}
	else
	{
		//Wait for current animation frame to end
		if (--obj->frame_time.b >= 0)
			return;
		
		//Special animation (walking, rolling, running, etc.)
		if (++anim_wait == 0)
		{
			//Walking or running
			//Get orienatation
			uint8_t angle = obj->angle;
			uint8_t flip = obj->status.p.f.x_flip;
			if (!flip)
				angle ^= ~0;
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
			uint16_t abs_spd = (obj->inertia < 0) ? -obj->inertia : obj->inertia;
			
			//Get script to use
			anim_script = GET_SONIC_ANISCR(SonAnimId_Run);
			if (abs_spd < 0x600)
			{
				anim_script = GET_SONIC_ANISCR(SonAnimId_Walk);
				angle += (angle >> 1);
			}
			angle <<= 1;
			
			//Get animation delay
			int16_t anim_spd = 0x800 - abs_spd;
			if (anim_spd < 0)
				anim_spd = 0;
			obj->frame_time.b = anim_spd >> 8;
			
			//Read animation
			Sonic_AnimateReadFrame(obj, anim_script);
			obj->frame += angle;
		}
		else if (++anim_wait == 0)
		{
			//Rolling
			//Get absolute speed
			uint16_t abs_spd = (obj->inertia < 0) ? -obj->inertia : obj->inertia;
			
			//Get script to use
			anim_script = GET_SONIC_ANISCR(SonAnimId_Roll2);
			if (abs_spd < 0x600)
				anim_script = GET_SONIC_ANISCR(SonAnimId_Roll);
			
			//Get animation delay
			int16_t anim_spd = 0x400 - abs_spd;
			if (anim_spd < 0)
				anim_spd = 0;
			obj->frame_time.b = anim_spd >> 8;
			
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
			uint16_t abs_spd = (obj->inertia < 0) ? -obj->inertia : obj->inertia;
			
			//Get animation delay
			int16_t anim_spd = 0x800 - abs_spd;
			if (anim_spd < 0)
				anim_spd = 0;
			obj->frame_time.b = anim_spd >> 6;
			
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

static void Sonic_LoadGfx(Object *obj)
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

//Sonic collision functions
static void Sonic_ResetOnFloor(Object *obj)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	//Set state
	obj->status.p.f.pushing = false;
	obj->status.p.f.in_air = false;
	obj->status.p.f.roll_jump = false;
	
	if (obj->status.p.f.in_ball)
	{
		obj->status.p.f.in_ball = false;
		obj->y_rad = SONIC_HEIGHT;
		obj->x_rad = SONIC_WIDTH;
		obj->anim = SonAnimId_Walk;
		obj->pos.l.y.f.u -= SONIC_BALL_SHIFT;
	}
	
	scratch->jumping = false;
	item_bonus = 0;
}

static int16_t Sonic_Angle(Object *obj, int16_t dist0, int16_t dist1)
{
	//Get angle and distance to use (use closest one)
	uint8_t res_angle = angle_buffer1;
	int16_t res_dist = dist1;
	if (dist1 > dist0)
	{
		res_angle = angle_buffer0;
		res_dist = dist0;
	}
	
	//Use adjusted angle if hit angle is odd (special angle, run on all sides)
	if (res_angle & 1)
		obj->angle = (obj->angle + 0x20) & 0xC0;
	else
		obj->angle = res_angle;
	return res_dist;
}

static void Sonic_AnglePos(Object *obj)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	//Don't do floor collision if standing on an object
	if (obj->status.p.f.object_stand)
	{
		angle_buffer0 = 0;
		angle_buffer1 = 0;
		return;
	}
	
	//Set 'no floor' angle
	angle_buffer0 = 3;
	angle_buffer1 = 3;
	
	//Get symmetrical angle
	uint8_t angle = obj->angle;
	if ((angle + 0x20) & 0x80)
	{
		if (angle & 0x80)
			angle--;
		angle += 0x20;
	}
	else
	{
		if (angle & 0x80)
			angle++;
		angle += 0x1F;
	}
	
	int16_t dist0, dist1, dist;
	switch (angle & 0xC0)
	{
		case 0x00:
			dist0 = FindFloor(obj, obj->pos.l.x.f.u + obj->x_rad, obj->pos.l.y.f.u + obj->y_rad, META_SOLID_TOP, 0, 0x10, &angle_buffer0);
			dist1 = FindFloor(obj, obj->pos.l.x.f.u - obj->x_rad, obj->pos.l.y.f.u + obj->y_rad, META_SOLID_TOP, 0, 0x10, &angle_buffer1);
			if ((dist = Sonic_Angle(obj, dist0, dist1)) != 0)
			{
				if (dist < 0)
				{
					if (dist >= -14)
						obj->pos.l.y.f.u += dist;
				}
				else
				{
					if (dist <= 14 || scratch->x38.floor_clip)
					{
						obj->pos.l.y.f.u += dist;
					}
					else
					{
						obj->status.p.f.in_air = true;
						obj->status.p.f.pushing = false;
						obj->prev_anim = 1;
					}
				}
			}
			break;
		case 0x40:
			dist0 = FindWall(obj, (obj->pos.l.x.f.u - obj->y_rad) ^ 0xF, obj->pos.l.y.f.u - obj->x_rad, META_SOLID_TOP, META_X_FLIP, -0x10, &angle_buffer0);
			dist1 = FindWall(obj, (obj->pos.l.x.f.u - obj->y_rad) ^ 0xF, obj->pos.l.y.f.u + obj->x_rad, META_SOLID_TOP, META_X_FLIP, -0x10, &angle_buffer1);
			if ((dist = Sonic_Angle(obj, dist0, dist1)) != 0)
			{
				if (dist < 0)
				{
					if (dist >= -14)
						obj->pos.l.x.f.u -= dist;
				}
				else
				{
					if (dist <= 14 || scratch->x38.floor_clip)
					{
						obj->pos.l.x.f.u -= dist;
					}
					else
					{
						obj->status.p.f.in_air = true;
						obj->status.p.f.pushing = false;
						obj->prev_anim = 1;
					}
				}
			}
			break;
		case 0x80:
			dist0 = FindFloor(obj, obj->pos.l.x.f.u - obj->x_rad, (obj->pos.l.y.f.u - obj->y_rad) ^ 0xF, META_SOLID_TOP, META_Y_FLIP, -0x10, &angle_buffer0);
			dist1 = FindFloor(obj, obj->pos.l.x.f.u + obj->x_rad, (obj->pos.l.y.f.u - obj->y_rad) ^ 0xF, META_SOLID_TOP, META_Y_FLIP, -0x10, &angle_buffer1);
			if ((dist = Sonic_Angle(obj, dist0, dist1)) != 0)
			{
				if (dist < 0)
				{
					if (dist >= -14)
						obj->pos.l.y.f.u -= dist;
				}
				else
				{
					if (dist <= 14 || scratch->x38.floor_clip)
					{
						obj->pos.l.y.f.u -= dist;
					}
					else
					{
						obj->status.p.f.in_air = true;
						obj->status.p.f.pushing = false;
						obj->prev_anim = 1;
					}
				}
			}
			break;
		case 0xC0:
			dist0 = FindWall(obj, obj->pos.l.x.f.u + obj->y_rad, obj->pos.l.y.f.u + obj->x_rad, META_SOLID_TOP, 0, 0x10, &angle_buffer0);
			dist1 = FindWall(obj, obj->pos.l.x.f.u + obj->y_rad, obj->pos.l.y.f.u - obj->x_rad, META_SOLID_TOP, 0, 0x10, &angle_buffer1);
			if ((dist = Sonic_Angle(obj, dist0, dist1)) != 0)
			{
				if (dist < 0)
				{
					if (dist >= -14)
						obj->pos.l.x.f.u += dist;
				}
				else
				{
					if (dist <= 14 || scratch->x38.floor_clip)
					{
						obj->pos.l.x.f.u += dist;
					}
					else if (!scratch->x38.floor_clip)
					{
						obj->status.p.f.in_air = true;
						obj->status.p.f.pushing = false;
						obj->prev_anim = 1;
					}
				}
			}
			break;
	}
}

static void Sonic_Floor(Object *obj)
{
	//Get angle we're moving in
	//There's some weird logging stuff done here
	//Maybe testing if the CalcAngle is yielding desirable results?
	uint8_t angle = CalcAngle(obj->xsp, obj->ysp);
	dbg_ang0 = angle;
	angle -= 0x20;
	dbg_ang1 = angle;
	angle &= 0xC0;
	dbg_ang2 = angle;
	
	int16_t dist0, dist1, clip;
	uint8_t hit_angle;
	switch (angle)
	{
		case 0x00: //Moving down
			//Collide with walls
			if ((dist0 = GetDistance2_Left(obj, obj->pos.l.x.f.u, obj->pos.l.y.f.u, NULL)) < 0)
			{
				obj->pos.l.x.f.u -= dist0;
				obj->xsp = 0;
			}
			if ((dist0 = GetDistance2_Right(obj, obj->pos.l.x.f.u, obj->pos.l.y.f.u, NULL)) < 0)
			{
				obj->pos.l.x.f.u += dist0;
				obj->xsp = 0;
			}
			
			//Collide with floor
			GetDistance_Down(obj, &dist0, &dist1, &hit_angle);
			dbg_ang3 = dist1; //...What?
			
			clip = -((obj->ysp >> 8) + 8);
			if (dist1 < 0 && (dist0 >= clip || dist1 >= clip))
			{
				//Set state
				obj->pos.l.y.f.u += dist1;
				obj->angle = hit_angle;
				Sonic_ResetOnFloor(obj);
				obj->anim = SonAnimId_Walk;
				
				//Get inertia
				if ((hit_angle + 0x20) & 0x40)
				{
					//If floor is greater than 45 degrees, use our full vertical velocity (capped at 0xFC0)
					obj->xsp = 0;
					if (obj->ysp > 0xFC0)
						obj->ysp = 0xFC0;
					obj->inertia = (hit_angle & 0x80) ? -obj->ysp : obj->ysp;
				}
				else if ((hit_angle + 0x10) & 0x20)
				{
					//If floor is greater than 22.5 degrees, use our halved vertical velocity
					obj->ysp /= 2;
					obj->inertia = (hit_angle & 0x80) ? -obj->ysp : obj->ysp;
				}
				else
				{
					//If floor is less than 22.5 degrees, use our horizontal velocity
					obj->ysp = 0;
					obj->inertia = obj->xsp;
				}
			}
			break;
		case 0x40: //Moving left
			//Collide with walls
			if ((dist0 = GetDistance2_Left(obj, obj->pos.l.x.f.u, obj->pos.l.y.f.u, NULL)) < 0)
			{
				obj->pos.l.x.f.u -= dist0;
				obj->xsp = 0;
				obj->inertia = obj->ysp;
				break; //Original returns here?
			}
			
			//Collide with ceiling
			GetDistance_Up(obj, NULL, &dist1, &hit_angle);
			if (dist1 < 0)
			{
				obj->pos.l.y.f.u -= dist1;
				if (obj->ysp < 0)
					obj->ysp = 0;
				break; //Again...
			}
			
			//Collide with floor
			if (obj->ysp >= 0)
			{
				GetDistance_Down(obj, NULL, &dist1, &hit_angle);
				if (dist1 < 0)
				{
					//Set state
					obj->pos.l.y.f.u += dist1;
					obj->angle = hit_angle;
					Sonic_ResetOnFloor(obj);
					obj->anim = SonAnimId_Walk;
					
					//Get inertia
					obj->ysp = 0;
					obj->inertia = obj->xsp;
				}
			}
			break;
		case 0x80: //Moving up
			//Collide with walls
			if ((dist0 = GetDistance2_Left(obj, obj->pos.l.x.f.u, obj->pos.l.y.f.u, NULL)) < 0)
			{
				obj->pos.l.x.f.u -= dist0;
				obj->xsp = 0;
			}
			if ((dist0 = GetDistance2_Right(obj, obj->pos.l.x.f.u, obj->pos.l.y.f.u, NULL)) < 0)
			{
				obj->pos.l.x.f.u += dist0;
				obj->xsp = 0;
			}
			
			//Collide with ceiling
			GetDistance_Up(obj, NULL, &dist1, &hit_angle);
			if (dist1 < 0)
			{
				obj->pos.l.y.f.u -= dist1;
				if (!((hit_angle + 0x20) & 0x40))
				{
					//Kill speed
					if (obj->ysp < 0)
						obj->ysp = 0;
				}
				else
				{
					//Land on ceiling
					obj->angle = hit_angle;
					Sonic_ResetOnFloor(obj);
					obj->inertia = (hit_angle & 0x80) ? -obj->ysp : obj->ysp;
				}
			}
			break;
		case 0xC0: //Moving right
			//Collide with walls
			if ((dist0 = GetDistance2_Right(obj, obj->pos.l.x.f.u, obj->pos.l.y.f.u, NULL)) < 0)
			{
				obj->pos.l.x.f.u += dist0;
				obj->xsp = 0;
			}
			
			//Collide with ceiling
			GetDistance_Up(obj, NULL, &dist1, &hit_angle);
			if (dist1 < 0)
			{
				obj->pos.l.y.f.u -= dist1;
				if (obj->ysp < 0)
					obj->ysp = 0;
				break; //Again...
			}
			
			//Collide with floor
			if (obj->ysp >= 0)
			{
				GetDistance_Down(obj, NULL, &dist1, &hit_angle);
				if (dist1 < 0)
				{
					//Set state
					obj->pos.l.y.f.u += dist1;
					obj->angle = hit_angle;
					Sonic_ResetOnFloor(obj);
					obj->anim = SonAnimId_Walk;
					
					//Get inertia
					obj->ysp = 0;
					obj->inertia = obj->xsp;
				}
			}
			break;
	}
}

//Object collision
static const uint8_t obj_sizes[][2] = {
	{ 0x0,  0x0},
	{0x14, 0x14},
	{ 0xC, 0x14},
	{0x14,  0xC},
	{ 0x4, 0x10},
	{ 0xC, 0x12},
	{0x10, 0x10},
	{ 0x6,  0x6},
	{0x18,  0xC},
	{ 0xC, 0x10},
	{0x10,  0xC},
	{ 0x8,  0x8},
	{0x14, 0x10},
	{0x14,  0x8},
	{ 0xE,  0xE},
	{0x18, 0x18},
	{0x28, 0x10},
	{0x10, 0x18},
	{ 0x8, 0x10},
	{0x20, 0x70},
	{0x40, 0x20},
	{0x80, 0x20},
	{0x20, 0x20},
	{ 0x8,  0x8},
	{ 0x4,  0x4},
	{0x20,  0x8},
	{ 0xC,  0xC},
	{ 0x8,  0x4},
	{0x18,  0x4},
	{0x28,  0x4},
	{ 0x4,  0x8},
	{ 0x4, 0x18},
	{ 0x4, 0x28},
	{ 0x4, 0x20},
	{0x18, 0x18},
	{ 0xC, 0x18},
	{0x48,  0x8},
};

static int React_ChkHurt(Object *obj, Object *hit)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	//Check for invincibility or invulnerability
	if (invincibility)
		return -1;
	if (scratch->flash_time)
		return -1;
	
	//Hurt Sonic
	return HurtSonic(obj, hit);
}

static int React_Enemy(Object *obj, Object *hit)
{
	//Check if we can hurt the enemy
	if (!(invincibility || obj->anim == SonAnimId_Roll))
		return React_ChkHurt(obj, hit);
	
	//Check if enemy is a boss
	if (hit->col_property)
	{
		//Hit boss
		obj->xsp = -obj->xsp >> 1;
		obj->ysp = -obj->ysp >> 1;
		hit->col_type = 0;
		if (!(--hit->col_property))
			hit->status.o.f.flag7 = true;
		return 0; //d0 not set
	}
	else
	{
		//Mark enemy touch flag
		hit->status.o.f.flag7 = true;
		
		//Increment bonus
		uint8_t bonus = item_bonus;
		item_bonus += 2;
		
		//Handle score bonus
		if (bonus >= 6)
			bonus = bonus;
		hit->scratch.u16[0xB] = bonus;
		
		static const uint16_t points[] = {10, 20, 50, 100};
		uint16_t point_bonus = points[bonus >> 1];
		if (item_bonus >= 32) //16 enemies destroyed
		{
			point_bonus = 1000;
			hit->scratch.u16[0xB] = 10;
		}
		
		AddPoints(point_bonus);
		
		//Explode object and bounce
		hit->type = ObjId_Explosion;
		hit->routine = 0;
		
		if (obj->ysp >= 0)
		{
			if (obj->pos.l.y.f.u < hit->pos.l.y.f.u)
				obj->ysp = -obj->ysp;
			else
				obj->ysp -= 0x100;
			return (uint16_t)obj->pos.l.y.f.u; //d0 is set to obj->pos.l.y.f.u
		}
		else
		{
			obj->ysp += 0x100;
			return 0; //d0 not set
		}
	}
}

static int ReactToItem(Object *obj)
{
	//Get collision area
	int16_t width, height;
	int16_t x = obj->pos.l.x.f.u - 8;
	int16_t y = obj->pos.l.y.f.u - (height = (uint8_t)(obj->y_rad - 3));
	
	if (obj->frame == 0x39) //Ducking
	{
		//Smaller hitbox when ducking
		y += 12;
		height = 10;
	}
	width = 16;
	height <<= 1;
	
	//Iterate through level objects
	Object *hit = level_objects;
	for (int i = 0; i < LEVEL_OBJECTS; i++, hit++)
	{
		//Check if object is collidable
		if (!(hit->render.f.on_screen && hit->col_type))
			continue;
		
		//Get object's size
		const uint8_t *sizep = obj_sizes[hit->col_type & 0x3F];
		uint8_t hit_width = *sizep++;
		uint8_t hit_height = *sizep++;
		
		//Check if we're touching (TODO: may be inaccurate)
		int16_t x_diff = x - (hit->pos.l.x.f.u - hit_width);
		int16_t y_diff = y - (hit->pos.l.y.f.u - hit_height);
		
		if (x_diff >= -width && x_diff <= hit_width * 2 && y_diff >= -height && y_diff <= hit_height * 2)
		{
			//Made contact
			switch (hit->col_type & 0xC0)
			{
				case 0x00: //Enemy
					return React_Enemy(obj, hit);
				case 0xC0: //Special
					break;
				case 0x80: //Hurt
					return React_ChkHurt(obj, hit);
				case 0x40: //Other
					break;
			}
		}
	}
	return 0;
}

//Sonic functions
int HurtSonic(Object *obj, Object *src)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	//Lose rings and shield
	if (!shield)
	{
		if (rings)
		{
			//Spawn lost rings object
			Object *rings = FindFreeObj();
			if (rings != NULL)
			{
				rings->type = ObjId_RingLoss;
				rings->pos.l.x.f.u = obj->pos.l.x.f.u;
				rings->pos.l.y.f.u = obj->pos.l.y.f.u;
			}
		}
		else
		{
			//Die
			return KillSonic(obj, src);
		}
	}
	shield = 0;
	
	//Set Sonic state
	obj->routine = 4;
	Sonic_ResetOnFloor(obj);
	obj->status.p.f.in_air = true;
	
	obj->xsp = obj->status.p.f.underwater ? -0x100 : -0x200;
	obj->ysp = obj->status.p.f.underwater ? -0x200 : -0x400;
	if (obj->pos.l.x.f.u >= src->pos.l.x.f.u)
		obj->xsp = -obj->xsp;
	obj->inertia = 0;
	
	obj->anim = SonAnimId_Hurt;
	scratch->flash_time = 120;
	
	//move.w	#sfx_Death,d0	; load normal damage sound
	//cmpi.b	#id_Spikes,(a2)	; was damage caused by spikes?
	//bne.s	@sound		; if not, branch
	//cmpi.b	#id_Harpoon,(a2) ; was damage caused by LZ harpoon?
	//bne.s	@sound		; if not, branch
	//move.w	#sfx_HitSpikes,d0 ; load spikes damage sound
	//
	//@sound:
	//jsr	(PlaySound_Special).l //TODO
	return -1;
}

int KillSonic(Object *obj, Object *src)
{
	(void)src;
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	//Check if we can be killed
	if (debug_use)
		return -1;
	
	//Set state
	invincibility = false;
	obj->routine = 6;
	Sonic_ResetOnFloor(obj);
	obj->status.p.f.in_air = true;
	obj->ysp = -0x700;
	obj->xsp = 0;
	obj->inertia = 0;
	scratch->x38.death_y = obj->pos.l.y.f.u;
	obj->anim = SonAnimId_Death;
	obj->tile.s.priority = true;
	
	//Play death sound
	//move.w	#sfx_Death,d0	; play normal death sound
	//cmpi.b	#id_Spikes,(a2)	; check	if you were killed by spikes
	//bne.s	@sound
	//move.w	#sfx_HitSpikes,d0 ; play spikes death sound
	//
	//@sound:
	//jsr	(PlaySound_Special).l
	return -1;
}

//Sonic movement functions
static bool Sonic_Jump(Object *obj)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	//Don't jump if ABC isn't pressed
	if (!(jpad1_press2 & (JPAD_A | JPAD_C | JPAD_B)))
		return false;
	
	//Check if we have enough room to jump
	int16_t dist;
	GetDistanceBelowAngle(obj, obj->angle + 0x80, NULL, &dist, NULL);
	if (dist < 6)
		return false;
	
	//Get jump speed
	int16_t speed = 0x680;
	if (obj->status.p.f.underwater)
		speed = 0x380;
	
	int16_t sin, cos;
	CalcSine(obj->angle - 0x40, &sin, &cos);
	obj->xsp += (cos * speed) >> 8;
	obj->ysp += (sin * speed) >> 8;
	
	//Set state
	obj->status.p.f.in_air = true;
	obj->status.p.f.pushing = false;
	scratch->jumping = true;
	scratch->x38.floor_clip = 0;
	
	//sfx	sfx_Jump,0,0,0	; play jumping sound //TODO
	
	obj->y_rad = SONIC_HEIGHT; //No idea why this is here
	obj->x_rad = SONIC_WIDTH;
	
	if (!obj->status.p.f.in_ball)
	{
		obj->y_rad = SONIC_BALL_HEIGHT;
		obj->x_rad = SONIC_BALL_WIDTH;
		obj->anim = SonAnimId_Roll;
		obj->status.p.f.in_ball = true;
		obj->pos.l.y.f.u += SONIC_BALL_SHIFT;
	}
	else
	{
		obj->status.p.f.roll_jump = true;
	}
	return true;
}

static void Sonic_SlopeResist(Object *obj)
{
	if (((obj->angle + 0x60) & 0xFF) >= 0xC0)
		return;
	
	int16_t force = (GetSin(obj->angle) * 0x20) >> 8;
	if (obj->inertia != 0)
		obj->inertia += force;
}

static void Sonic_MoveLeft(Object *obj)
{
	int16_t inertia = obj->inertia;
	if (inertia <= 0)
	{
		//Turn around
		if (!obj->status.p.f.x_flip)
		{
			obj->status.p.f.x_flip = true;
			obj->status.p.f.pushing = false;
			obj->prev_anim = 1; //Reset animation
		}
		
		//Accelerate
		if ((inertia -= sonspeed_acc) <= -sonspeed_max)
			inertia = -sonspeed_max;
		
		//Set speed and animation
		obj->inertia = inertia;
		obj->anim = SonAnimId_Walk;
	}
	else
	{
		//Decelerate
		if ((inertia -= sonspeed_dec) < 0)
			inertia = -0x80;
		obj->inertia = inertia;
		
		//Skid
		if (((obj->angle + 0x20) & 0xC0) == 0x00 && inertia >= 0x400)
		{
			obj->anim = SonAnimId_Stop;
			obj->status.p.f.x_flip = false;
			//sfx	sfx_Skid,0,0,0	; play stopping sound //TODO
		}
	}
}

static void Sonic_MoveRight(Object *obj)
{
	int16_t inertia = obj->inertia;
	if (inertia >= 0)
	{
		//Turn around
		if (obj->status.p.f.x_flip)
		{
			obj->status.p.f.x_flip = false;
			obj->status.p.f.pushing = false;
			obj->prev_anim = 1; //Reset animation
		}
		
		//Accelerate
		if ((inertia += sonspeed_acc) >= sonspeed_max)
			inertia = sonspeed_max;
		
		//Set speed and animation
		obj->inertia = inertia;
		obj->anim = SonAnimId_Walk;
	}
	else
	{
		//Decelerate
		if ((inertia += sonspeed_dec) >= 0)
			inertia = 0x80;
		obj->inertia = inertia;
		
		//Skid
		if (((obj->angle + 0x20) & 0xC0) == 0x00 && inertia <= -0x400)
		{
			obj->anim = SonAnimId_Stop;
			obj->status.p.f.x_flip = true;
			//sfx	sfx_Skid,0,0,0	; play stopping sound //TODO
		}
	}
}

static void Sonic_Move(Object *obj)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	if (!jump_only)
	{
		if (!scratch->control_lock)
		{
			//Move left and right according to held direction
			if (jpad1_hold2 & JPAD_LEFT)
				Sonic_MoveLeft(obj);
			if (jpad1_hold2 & JPAD_RIGHT)
				Sonic_MoveRight(obj);
			
			//Do idle or balance animation
			if (((obj->angle + 0x20) & 0xC0) == 0x00 && !obj->inertia)
			{
				//Do idle animation
				obj->status.p.f.pushing = false;
				obj->anim = SonAnimId_Wait;
				
				if (obj->status.p.f.object_stand)
				{
					//Balance on an object
					Object *stand = &objects[scratch->standing_obj];
					if (!stand->status.o.f.flag7)
					{
						int16_t left_dist = stand->width_pixels;
						int16_t right_dist = left_dist + left_dist - 4;
						left_dist += obj->pos.l.x.f.u - stand->pos.l.x.f.u;
						
						if (left_dist < 4)
							obj->status.p.f.x_flip = true;
						else if (left_dist >= right_dist)
							obj->status.p.f.x_flip = false;
						obj->anim = SonAnimId_Balance;
						goto Sonic_ResetScr;
					}
				}
				else
				{
					//Balance on level
					if (ObjFloorDist(obj, obj->pos.l.x.f.u) >= 12)
					{
						if (scratch->front_angle == 3)
						{
							obj->status.p.f.x_flip = false;
							obj->anim = SonAnimId_Balance;
							goto Sonic_ResetScr;
						}
						if (scratch->back_angle == 3)
						{
							obj->status.p.f.x_flip = true;
							obj->anim = SonAnimId_Balance;
							goto Sonic_ResetScr;
						}
					}
				}
				
				//Handle looking up and down
				if (jpad1_hold2 & JPAD_UP)
				{
					obj->anim = SonAnimId_LookUp;
					if (look_shift != (200 + SCREEN_TALLADD2))
						look_shift += 2;
					goto DoFriction;
				}
				if (jpad1_hold2 & JPAD_DOWN)
				{
					obj->anim = SonAnimId_Duck;
					if (look_shift != (8 + SCREEN_TALLADD2))
						look_shift -= 2;
					goto DoFriction;
				}
			}
		}
		
		//Reset camera to neutral position
		Sonic_ResetScr:;
		if (look_shift < (96 + SCREEN_TALLADD2))
			look_shift += 2;
		else if (look_shift > (96 + SCREEN_TALLADD2))
			look_shift -= 2;
		
		//Friction
		DoFriction:;
		if (!(jpad1_hold2 & (JPAD_LEFT | JPAD_RIGHT)))
		{
			if (obj->inertia > 0)
			{
				if ((obj->inertia -= sonspeed_acc) < 0)
					obj->inertia = 0;
			}
			else
			{
				if ((obj->inertia += sonspeed_acc) >= 0)
					obj->inertia = 0;
			}
		}
	}
	
	//Calculate global speed from inertia
	int16_t sin, cos;
	CalcSine(obj->angle, &sin, &cos);
	obj->xsp = (cos * obj->inertia) >> 8;
	obj->ysp = (sin * obj->inertia) >> 8;
	
	//Handle wall collision
	if (((obj->angle + 0x40) & 0x80) || !obj->inertia)
		return;
	
	uint8_t add_angle = (obj->inertia < 0) ? 0x40 : -0x40;
	uint8_t angle = obj->angle + add_angle;
	int16_t dist = GetDistanceBelowAngle2(obj, angle, NULL);
	
	if (dist < 0)
	{
		dist <<= 8;
		switch ((angle + 0x20) & 0xC0)
		{
			case 0x00:
				obj->ysp += dist;
				break;
			case 0x40:
				obj->xsp -= dist;
				obj->status.p.f.pushing = true;
				obj->inertia = 0;
				break;
			case 0x80:
				obj->ysp -= dist;
				break;
			case 0xC0:
				obj->xsp += dist;
				obj->status.p.f.pushing = true;
				obj->inertia = 0;
				break;
		}
	}
}

static void Sonic_ChkRoll(Object *obj)
{
	//Enter roll state
	if (obj->status.p.f.in_ball)
		return;
	obj->status.p.f.in_ball = true;
	obj->y_rad = SONIC_BALL_HEIGHT;
	obj->x_rad = SONIC_BALL_WIDTH;
	obj->anim = SonAnimId_Roll;
	obj->pos.l.y.f.u += SONIC_BALL_SHIFT;
	//sfx	sfx_Roll,0,0,0	; play rolling sound //TODO
	
	//Set speed (S-tubes)
	if (!obj->inertia)
		obj->inertia = 0x200;
}

static void Sonic_Roll(Object *obj)
{
	//Check if we can and are trying to roll
	if (jump_only || ((obj->inertia < 0) ? -obj->inertia : obj->inertia) < 0x80)
		return;
	if ((jpad1_hold2 & (JPAD_LEFT | JPAD_RIGHT)) || !(jpad1_hold2 & JPAD_DOWN))
		return;
	Sonic_ChkRoll(obj);
}

static void Sonic_LevelBound(Object *obj)
{
	//Get next X position
	//This is unsigned, but it shouldn't be
	uint16_t x = (obj->pos.l.x.v + (obj->xsp << 8)) >> 16;
	
	//Prevent us from going off the left or right boundaries
	int16_t bound;
	if (x < (bound = limit_left2 + 16) || x > (bound = limit_right2 + (lock_screen ? 290 : 360) + SCREEN_WIDEADD))
	{
		obj->pos.l.x.f.u = bound;
		obj->pos.l.x.f.l = 0;
		obj->xsp = 0;
		obj->inertia = 0;
	} 
	
	//Fall off the bottom boundary
	if (obj->pos.l.y.f.u >= limit_btm2 + SCREEN_HEIGHT)
	{
		if (level_id == LEVEL_ID(ZoneId_SBZ, 1) && obj->pos.l.x.f.u >= 0x2000)
		{
			//Go to SBZ3 if falling off at the end of SBZ2
			last_lamp = 0;
			restart = true;
			level_id = LEVEL_ID(ZoneId_LZ, 3);
		}
		else
		{
			//Kill Sonic
			KillSonic(obj, obj); //The second argument is undefined in the original
		}
	}
}

static void Sonic_SlopeRepel(Object *obj)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	//Check if we can fall off a slope
	if (scratch->x38.floor_clip)
		return;
	
	if (!scratch->control_lock)
	{
		//Check if the slope is steep enough
		if ((obj->angle + 0x20) & 0xC0)
		{
			//Fall off if we're going too slow
			if (((obj->inertia < 0) ? -obj->inertia : obj->inertia) < 0x280)
			{
				obj->inertia = 0;
				obj->status.p.f.in_air = true;
				scratch->control_lock = 30;
			}
		}
	}
	else
	{
		//Decrement control lock
		scratch->control_lock--;
	}
}

static void Sonic_RollRepel(Object *obj)
{
	if (((obj->angle + 0x60) & 0xFF) >= 0xC0)
		return;
	
	int16_t force = (GetSin(obj->angle) * 0x50) >> 8;
	if (obj->inertia > 0)
	{
		if (force < 0)
			force >>= 2;
		obj->inertia += force;
	}
	else if (obj->inertia < 0)
	{
		if (force >= 0)
			force >>= 2;
		obj->inertia += force;
	}
}

static void Sonic_RollLeft(Object *obj)
{
	int16_t inertia = obj->inertia;
	if (inertia <= 0)
	{
		//Set animation
		obj->status.p.f.x_flip = true;
		obj->anim = SonAnimId_Roll;
	}
	else
	{
		//Decelerate
		if ((inertia -= sonspeed_dec >> 2) < 0)
			inertia = -0x80;
		obj->inertia = inertia;
	}
}

static void Sonic_RollRight(Object *obj)
{
	int16_t inertia = obj->inertia;
	if (inertia >= 0)
	{
		//Set animation
		obj->status.p.f.x_flip = false;
		obj->anim = SonAnimId_Roll;
	}
	else
	{
		//Decelerate
		if ((inertia += sonspeed_dec >> 2) >= 0)
			inertia = 0x80;
		obj->inertia = inertia;
	}
}

static void Sonic_RollSpeed(Object *obj)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	if (!jump_only)
	{
		if (!scratch->control_lock)
		{
			//Move left and right according to held direction
			if (jpad1_hold2 & JPAD_LEFT)
				Sonic_RollLeft(obj);
			if (jpad1_hold2 & JPAD_RIGHT)
				Sonic_RollRight(obj);
		}
		
		//Friction
		if (obj->inertia > 0)
		{
			if ((obj->inertia -= (sonspeed_acc >> 1)) < 0)
				obj->inertia = 0;
		}
		else
		{
			if ((obj->inertia += (sonspeed_acc >> 1)) >= 0)
				obj->inertia = 0;
		}
		
		//Uncurl when we've come to a stop
		if (obj->inertia == 0)
		{
			obj->status.p.f.in_ball = false;
			obj->y_rad = SONIC_HEIGHT;
			obj->x_rad = SONIC_WIDTH;
			obj->anim = SonAnimId_Wait;
			obj->pos.l.y.f.u -= SONIC_BALL_SHIFT;
		}
	}
	
	//Calculate global speed from inertia
	int16_t sin, cos;
	CalcSine(obj->angle, &sin, &cos);
	obj->ysp = (sin * obj->inertia) >> 8;
	cos = (cos * obj->inertia) >> 8;
	if (cos > 0x1000) //Global X speed is limited to 0x1000 both ways.
		cos = 0x1000; //This causes the speed to desync from inertia
	if (cos < -0x1000) //at high speeds.
		cos = -0x1000; //Is this here because of the broken camera?
	obj->xsp = cos;
	
	//Handle wall collision
	if (((obj->angle + 0x40) & 0x80) || !obj->inertia)
		return;
	
	uint8_t add_angle = (obj->inertia < 0) ? 0x40 : -0x40;
	uint8_t angle = obj->angle + add_angle;
	int16_t dist = GetDistanceBelowAngle2(obj, angle, NULL);
	
	if (dist < 0)
	{
		dist <<= 8;
		switch ((angle + 0x20) & 0xC0)
		{
			case 0x00:
				obj->ysp += dist;
				break;
			case 0x40:
				obj->xsp -= dist;
				obj->status.p.f.pushing = true;
				obj->inertia = 0;
				break;
			case 0x80:
				obj->ysp -= dist;
				break;
			case 0xC0:
				obj->xsp += dist;
				obj->status.p.f.pushing = true;
				obj->inertia = 0;
				break;
		}
	}
}

static void Sonic_JumpHeight(Object *obj)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	if (scratch->jumping)
	{
		//Get minimum jump speed and apply if ABC isn't held
		int16_t spd = obj->status.p.f.underwater ? -0x200 : -0x400;
		if (obj->ysp < spd && !(jpad1_hold2 & (JPAD_A | JPAD_C | JPAD_B)))
			obj->ysp = spd;
	}
	else
	{
		//Cap upwards speed
		if (obj->ysp < -0xFC0)
			obj->ysp = -0xFC0;
	}
}

static void Sonic_JumpDirection(Object *obj)
{
	//Handle acceleration
	if (!obj->status.p.f.roll_jump)
	{
		int16_t xsp = obj->xsp;
		
		//Accelerate left
		if (jpad1_hold2 & JPAD_LEFT)
		{
			obj->status.p.f.x_flip = true;
			if ((xsp -= (sonspeed_acc << 1)) <= -sonspeed_max)
				xsp = -sonspeed_max;
		}
		
		//Accelerate right
		if (jpad1_hold2 & JPAD_RIGHT)
		{
			obj->status.p.f.x_flip = false;
			if ((xsp += (sonspeed_acc << 1)) >= sonspeed_max)
				xsp = sonspeed_max;
		}
		
		//Apply acceleration
		obj->xsp = xsp;
	}
	
	//Reset screen shift
	if (look_shift < (96 + SCREEN_TALLADD2))
		look_shift += 2;
	else if (look_shift > (96 + SCREEN_TALLADD2))
		look_shift -= 2;
	
	//Handle air drag
	if ((uint16_t)obj->ysp >= (uint16_t)-0x400)
	{
		int16_t xsp = obj->xsp;
		int16_t drag = xsp >> 5;
		if (drag > 0) //Do I really have to say anything?
		{
			if ((xsp -= drag) < 0)
				xsp = 0;
			obj->xsp = xsp;
		}
		else if (drag < 0)
		{
			if ((xsp -= drag) >= 0)
				xsp = 0;
			obj->xsp = xsp;
		}
	}
}

static void Sonic_JumpAngle(Object *obj)
{
	//Reset angle towards 0
	uint8_t angle = obj->angle;
	if (angle == 0)
		return;
	
	if (!(angle & 0x80))
	{
		if ((angle -= 2) & 0x80)
			angle = 0;
	}
	else
	{
		if (!((angle += 2) & 0x80))
			angle = 0;
	}
	
	obj->angle = angle;
}

static void Sonic_Loops(Object *obj)
{
	//Make sure we're in SLZ or GHZ
	if (LEVEL_ZONE(level_id) != ZoneId_SLZ && LEVEL_ZONE(level_id) != ZoneId_GHZ)
		return;
	
	//Get chunk we're on
	int16_t cx = (obj->pos.l.x.f.u >> 8) & 0x3F;
	int16_t cy = (obj->pos.l.y.f.u >> 8) & 0x7;
	uint8_t chunk = level_layout[cy][0][cx];
	
	//Handle S-tubes
	if (chunk == level_schunks[1][0] || chunk == level_schunks[1][1])
	{
		Sonic_ChkRoll(obj);
		return;
	}
	
	//Handle loops
	if (chunk == level_schunks[0][0])
	{
		CheckLoop:;
		//Return to high plane if to the left
		if ((uint8_t)obj->pos.l.x.f.u < 0x2C)
		{
			obj->render.f.player_loop = false;
			return;
		}
		
		//Go to low plane if to the right
		if ((uint8_t)obj->pos.l.x.f.u >= 0xE0)
		{
			obj->render.f.player_loop = true;
			return;
		}
		
		//Check our angle
		if (!obj->render.f.player_loop)
		{
			if (obj->angle && obj->angle < 0x80)
				obj->render.f.player_loop = true;
		}
		else
		{
			if (obj->angle > 0x80)
				obj->render.f.player_loop = false;
		}
	}
	else if (chunk == level_schunks[0][1])
	{
		//Return to high plane if in mid-air
		if (!obj->status.p.f.in_air)
			goto CheckLoop;
		obj->render.f.player_loop = false;
	}
	else
	{
		//Return to high plane
		obj->render.f.player_loop = false;
	}
}

//Other functions
static void GameOver(Object *obj)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&obj->scratch;
	
	//Have we fallen below the screen?
	if ((limit_btm2 + 256 + SCREEN_TALLADD) >= obj->pos.l.y.f.u)
		return;
	
	//Enter respawn state
	obj->ysp = -0x38; //???
	obj->routine += 2;
	time_count = false;
	
	//Check if we've game over'ed
	if (--lives == 0)
	{
		//Set death timer
		scratch->death_timer = 0;
		
		//Load 'GAME OVER' objects
		objects[2].type = ObjId_GameOverCard; //GAME
		objects[3].type = ObjId_GameOverCard; //OVER
		objects[3].frame = 1;
		
		time_over = false;
		//music	bgm_GameOver,0,0,0	; play game over music //TODO
		AddPLC(PlcId_GameOver);
	}
	else
	{
		//Set death timer
		scratch->death_timer = 60;
		
		//Check if we've time over'ed
		if (time_over)
		{
			//Set death timer
			scratch->death_timer = 0;
			
			//Load 'TIME OVER' objects
			objects[2].type = ObjId_GameOverCard; //TIME
			objects[3].type = ObjId_GameOverCard; //OVER
			objects[2].frame = 2;
			objects[3].frame = 3;
			
			//music	bgm_GameOver,0,0,0	; play game over music //TODO
			AddPLC(PlcId_GameOver);
		}
	}
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
			obj->y_rad = SONIC_HEIGHT;
			obj->x_rad = SONIC_WIDTH;
			
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
		case 2: //Regular movement
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
						if (Sonic_Jump(obj))
							break;
						Sonic_SlopeResist(obj);
						Sonic_Move(obj);
						Sonic_Roll(obj);
						Sonic_LevelBound(obj);
						SpeedToPos(obj);
						Sonic_AnglePos(obj);
						Sonic_SlopeRepel(obj);
						break;
					case 2: //Not in ball, in air
						Sonic_JumpHeight(obj);
						Sonic_JumpDirection(obj);
						Sonic_LevelBound(obj);
						ObjectFall(obj);
						if (obj->status.p.f.underwater)
							obj->ysp -= 0x28;
						Sonic_JumpAngle(obj);
						Sonic_Floor(obj);
						break;
					case 4: //In ball, not in air
						if (Sonic_Jump(obj))
							break;
						Sonic_RollRepel(obj);
						Sonic_RollSpeed(obj);
						Sonic_LevelBound(obj);
						SpeedToPos(obj);
						Sonic_AnglePos(obj);
						Sonic_SlopeRepel(obj);
						break;
					case 6: //In ball, in air
						Sonic_JumpHeight(obj);
						Sonic_JumpDirection(obj);
						Sonic_LevelBound(obj);
						ObjectFall(obj);
						if (obj->status.p.f.underwater)
							obj->ysp -= 0x28;
						Sonic_JumpAngle(obj);
						Sonic_Floor(obj);
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
			
			//Handle object and loop interaction
			if (!(lock_multi & 0x80))
				ReactToItem(obj);
			Sonic_Loops(obj);
			
			//Handle DPLCs
			Sonic_LoadGfx(obj);
			break;
		case 4: //Hurt
			break;
		case 6: //Dead
			//Check for respawning and fall
			GameOver(obj);
			ObjectFall(obj);
			
			//Handle general player state stuff
			Sonic_RecordPosition(obj);
			
			//Animate
			Sonic_Animate(obj);
			
			//Handle DPLCs and draw sprite
			Sonic_LoadGfx(obj);
			DisplaySprite(obj);
			break;
		case 8: //Dead, respawning
			//Handle death timer
			if (scratch->death_timer && --scratch->death_timer == 0)
				restart = true;
			break;
	}
}
