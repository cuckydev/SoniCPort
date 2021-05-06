#include "Object.h"
#include "Sonic.h"

#include "Game.h"
#include "Level.h"
#include "LevelScroll.h"
#include "SpecialStage.h"
#include "MathUtil.h"

//Special Stage Sonic scratch memory
typedef struct
{
	uint8_t pad0[8];   //0x28-0x2F
	uint8_t hit_block; //0x30
	uint8_t pad1;      //0x31
	uint8_t *hit_addr; //0x32
} Scratch_SpecialSonic;

//Special Stage Sonic functions
static void SpecialSonic_FixCamera(Object *obj)
{
	if ((uint16_t)obj->pos.l.x.f.u >= (SCREEN_WIDTH >> 1))
		scrpos_x.f.u = obj->pos.l.x.f.u - (SCREEN_WIDTH >> 1);
	if ((uint16_t)obj->pos.l.x.f.u >= (SCREEN_HEIGHT >> 1))
		scrpos_y.f.u = obj->pos.l.y.f.u - (SCREEN_HEIGHT >> 1);
}

static void SpecialSonic_Display(Object *obj)
{
	//Move object and camera
	SpeedToPos(obj);
	SpecialSonic_FixCamera(obj);
	
	//Rotate stage
	ss_angle.v += ss_rotate;
	
	//Animate object
	Sonic_Animate(obj);
}

//Special Stage Sonic collision
static void SpecialSonic_ChkTile(Object *obj, uint8_t block, uint8_t *addr, int *result)
{
	Scratch_SpecialSonic *scratch = (Scratch_SpecialSonic*)&obj->scratch;
	
	//Check if block can be touched
	if (block == 0x00 || block == 0x28)
		return;
	if (block >= 0x3A && block < 0x4B)
		return;
	
	//Set that block was touched
	scratch->hit_block = block;
	scratch->hit_addr = addr; //This is after the address is incremented?
	*result = -1;
}

static int SpecialSonic_ChkPos(Object *obj, int32_t x, int32_t y)
{
	//Get stage layout position
	const uint8_t *layout = ss_layout;
	layout += (((uint16_t)(y >> 16) + 0x44) / 24) * SS_DIM;
	layout += (((uint16_t)(x >> 16) + 0x14) / 24);
	
	//Return if we hit a tile
	int result = 0;
	SpecialSonic_ChkTile(obj, *layout++, layout, &result);
	SpecialSonic_ChkTile(obj, *layout++, layout, &result);
	layout += SS_DIM - 2;
	SpecialSonic_ChkTile(obj, *layout++, layout, &result);
	SpecialSonic_ChkTile(obj, *layout++, layout, &result);
	return result;
}

//Special Stage Sonic jump
static void SpecialSonic_Jump(Object *obj)
{
	//Don't jump if ABC isn't pressed
	if (!(jpad1_press2 & (JPAD_A | JPAD_C | JPAD_B)))
		return;
	
	//Get jump speed
	int16_t sin, cos;
	CalcSine(-(ss_angle.f.u & 0xFC) - 0x40, &sin, &cos);
	obj->xsp = (cos * 0x680) >> 8;
	obj->ysp = (sin * 0x680) >> 8;
	
	//Set state
	obj->status.p.f.in_air = true;
	//sfx	sfx_Jump,0,0,0	; play jumping sound TODO
}

static void SpecialSonic_JumpHeight(Object *obj)
{
	return; //Stub
}

static void SpecialSonic_Fall(Object *obj)
{
	int32_t cx = obj->pos.l.x.v;
	int32_t cy = obj->pos.l.y.v;
	
	//Get next move delta
	int16_t sin, cos;
	CalcSine(ss_angle.f.u & 0xFC, &sin, &cos);
	
	int32_t xa = (sin * 0x2A) + ((int32_t)obj->xsp << 8);
	int32_t ya = (cos * 0x2A) + ((int32_t)obj->ysp << 8);
	
	//Move X
	cx += xa;
	if (SpecialSonic_ChkPos(obj, cx, cy))
	{
		//Hit wall
		cx -= xa;
		obj->xsp = xa = 0;
		obj->status.p.f.in_air = false;
		
		//Move Y
		cy += ya;
		if (SpecialSonic_ChkPos(obj, cx, cy))
		{
			cy -= ya;
			obj->ysp = ya = 0;
		}
		else
		{
			obj->xsp = xa >> 8;
			obj->ysp = ya >> 8;
		}
	}
	else
	{
		//Move Y
		cy += ya;
		if (SpecialSonic_ChkPos(obj, cx, cy))
		{
			cy -= ya;
			obj->ysp = ya = 0;
			obj->status.p.f.in_air = false;
			obj->xsp = xa >> 8;
			obj->ysp = ya >> 8;
		}
		else
		{
			obj->xsp = xa >> 8;
			obj->ysp = ya >> 8;
			obj->status.p.f.in_air = true;
		}
	}
}

//Special Stage Sonic movement
static void SpecialSonic_MoveLeft(Object *obj)
{
	obj->status.p.f.x_flip = true;
	
	int16_t inertia = obj->inertia;
	if (inertia <= 0)
	{
		//Accelerate
		if ((inertia -= 0xC) <= -0x800)
			inertia = -0x800;
		obj->inertia = inertia;
	}
	else
	{
		//Decelerate
		inertia -= 0x40;
		obj->inertia = inertia;
	}
}

static void SpecialSonic_MoveRight(Object *obj)
{
	obj->status.p.f.x_flip = false;
	
	int16_t inertia = obj->inertia;
	if (inertia >= 0)
	{
		//Accelerate
		if ((inertia += 0xC) >= 0x800)
			inertia = 0x800;
		obj->inertia = inertia;
	}
	else
	{
		//Decelerate
		inertia += 0x40;
		obj->inertia = inertia;
	}
}

static void SpecialSonic_Move(Object *obj)
{
	//Move left and right according to held direction
	if (jpad1_hold2 & JPAD_LEFT)
		SpecialSonic_MoveLeft(obj);
	if (jpad1_hold2 & JPAD_RIGHT)
		SpecialSonic_MoveRight(obj);
	
	//Friction
	if (!(jpad1_hold2 & (JPAD_LEFT | JPAD_RIGHT)))
	{
		if (obj->inertia > 0)
		{
			if ((obj->inertia -= 0xC) < 0)
				obj->inertia = 0;
		}
		else if (obj->inertia < 0)
		{
			if ((obj->inertia += 0xC) >= 0)
				obj->inertia = 0;
		}
	}
	
	//Apply inertia
	int16_t sin, cos;
	int32_t xa, ya;
	CalcSine(-((ss_angle.f.u + 0x20) & 0xC0), &sin, &cos);
	xa = cos * obj->inertia;
	ya = sin * obj->inertia;
	
	obj->pos.l.x.v += xa;
	obj->pos.l.y.v += ya;
	
	//Perform collision detection
	if (SpecialSonic_ChkPos(obj, obj->pos.l.x.v, obj->pos.l.y.v))
	{
		obj->pos.l.x.v -= xa;
		obj->pos.l.y.v -= ya;
		obj->inertia = 0;
	}
}

//Special Stage Sonic object
void Obj_SpecialSonic(Object *obj)
{
	Scratch_SpecialSonic *scratch = (Scratch_SpecialSonic*)&obj->scratch;
	
	//TODO: Debug mode
	
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine
			obj->routine += 2;
			
			//Set collision size
			obj->y_rad = 14;
			obj->x_rad = 7;
			
			//Set object drawing information
			obj->mappings = map_sonic;
			obj->tile = TILE_MAP(0, 0, 0, 0, 0x780);
			obj->render.b = 0;
			obj->render.f.align_fg = true;
			obj->priority = 0;
			
			//Initialize object state
			obj->anim = SonAnimId_Roll;
			obj->status.p.f.in_air = true;
			obj->status.p.f.in_ball = true;
	//Fallthrough
		case 2: //Moving
			//Enter debug mode
			if (debug_cheat && (jpad1_press1 & JPAD_B))
				debug_use = true;
			
			//Run player routine
			scratch->hit_block = 0;
			
			if (!obj->status.p.f.in_air)
			{
				SpecialSonic_Jump(obj);
				SpecialSonic_Move(obj);
				SpecialSonic_Fall(obj);
				SpecialSonic_Display(obj);
			}
			else
			{
				SpecialSonic_JumpHeight(obj);
				SpecialSonic_Move(obj);
				SpecialSonic_Fall(obj);
				SpecialSonic_Display(obj);
			}
			
			//Draw object
			Sonic_LoadGfx(obj);
			DisplaySprite(obj);
			break;
	}
}
