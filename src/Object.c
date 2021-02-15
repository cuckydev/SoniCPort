#include "Object.h"

#include "Video.h"
#include "Level.h"
#include "LevelScroll.h"

#include "Object/Sonic.h"

#include <string.h>

//Object draw queue
struct SpriteQueue
{
	uint32_t size;
	Object *obj[0x3F];
} sprite_queue[8];

//Object indices
//#ifndef SCP_FIX_BUGS
//	#define Obj_Null ObjectFall //Thats right, all null objects point to ObjectFall
//#else
	void Obj_Null(Object *obj)
	{
		if (obj->respawn_index)
			objstate[obj->respawn_index] &= 0x7F;
		DeleteObject(obj);
	}
//#endif
//Don't re-enable this until all objects are implemented
//...Trust me

void Obj_Sonic(Object *obj);
void Obj_TitleSonic(Object *obj);
void Obj_PSB(Object *obj);
void Obj_GHZBridge(Object *obj);
void Obj_Crabmeat(Object *obj);
void Obj_BuzzBomber(Object *obj);
void Obj_BuzzMissile(Object *obj);
void Obj_BuzzExplode(Object *obj);
void Obj_Explosion(Object *obj);
void Obj_Chopper(Object *obj);
void Obj_TitleCard(Object *obj);
void Obj_GameOverCard(Object *obj);
void Obj_Motobug(Object *obj);
void Obj_Credits(Object *obj);

static void (*object_func[])(Object*) = {
	/* ObjId_Null         */ NULL,
	/* ObjId_Sonic        */ Obj_Sonic,
	/* ObjId_02           */ Obj_Null,
	/* ObjId_03           */ Obj_Null,
	/* ObjId_04           */ Obj_Null,
	/* ObjId_05           */ Obj_Null,
	/* ObjId_06           */ Obj_Null,
	/* ObjId_07           */ Obj_Null,
	/* ObjId_08           */ Obj_Null,
	/* ObjId_09           */ Obj_Null,
	/* ObjId_0A           */ Obj_Null,
	/* ObjId_0B           */ Obj_Null,
	/* ObjId_0C           */ Obj_Null,
	/* ObjId_0D           */ Obj_Null,
	/* ObjId_TitleSonic   */ Obj_TitleSonic,
	/* ObjId_PSB          */ Obj_PSB,
	/* ObjId_10           */ Obj_Null,
	/* ObjId_GHZBridge    */ Obj_GHZBridge,
	/* ObjId_12           */ Obj_Null,
	/* ObjId_13           */ Obj_Null,
	/* ObjId_14           */ Obj_Null,
	/* ObjId_15           */ Obj_Null,
	/* ObjId_16           */ Obj_Null,
	/* ObjId_17           */ Obj_Null,
	/* ObjId_18           */ Obj_Null,
	/* ObjId_19           */ Obj_Null,
	/* ObjId_1A           */ Obj_Null,
	/* ObjId_1B           */ Obj_Null,
	/* ObjId_1C           */ Obj_Null,
	/* ObjId_1D           */ Obj_Null,
	/* ObjId_1E           */ Obj_Null,
	/* ObjId_Crabmeat     */ Obj_Crabmeat,
	/* ObjId_20           */ Obj_Null,
	/* ObjId_21           */ Obj_Null,
	/* ObjId_BuzzBomber   */ Obj_BuzzBomber,
	/* ObjId_BuzzMissile  */ Obj_BuzzMissile,
	/* ObjId_BuzzExplode  */ Obj_BuzzExplode,
	/* ObjId_25           */ Obj_Null,
	/* ObjId_26           */ Obj_Null,
	/* ObjId_Explosion    */ Obj_Explosion,
	/* ObjId_Animal       */ Obj_Null,
	/* ObjId_29           */ Obj_Null,
	/* ObjId_2A           */ Obj_Null,
	/* ObjId_Chopper      */ Obj_Chopper,
	/* ObjId_2C           */ Obj_Null,
	/* ObjId_2D           */ Obj_Null,
	/* ObjId_2E           */ Obj_Null,
	/* ObjId_2F           */ Obj_Null,
	/* ObjId_30           */ Obj_Null,
	/* ObjId_31           */ Obj_Null,
	/* ObjId_32           */ Obj_Null,
	/* ObjId_33           */ Obj_Null,
	/* ObjId_TitleCard    */ Obj_TitleCard,
	/* ObjId_35           */ Obj_Null,
	/* ObjId_36           */ Obj_Null,
	/* ObjId_RingLoss     */ Obj_Null,
	/* ObjId_38           */ Obj_Null,
	/* ObjId_GameOverCard */ Obj_GameOverCard,
	/* ObjId_3A           */ Obj_Null,
	/* ObjId_3B           */ Obj_Null,
	/* ObjId_3C           */ Obj_Null,
	/* ObjId_3D           */ Obj_Null,
	/* ObjId_3E           */ Obj_Null,
	/* ObjId_3F           */ Obj_Null,
	/* ObjId_Motobug      */ Obj_Motobug,
	/* ObjId_41           */ Obj_Null,
	/* ObjId_42           */ Obj_Null,
	/* ObjId_43           */ Obj_Null,
	/* ObjId_44           */ Obj_Null,
	/* ObjId_45           */ Obj_Null,
	/* ObjId_46           */ Obj_Null,
	/* ObjId_47           */ Obj_Null,
	/* ObjId_48           */ Obj_Null,
	/* ObjId_49           */ Obj_Null,
	/* ObjId_4A           */ Obj_Null,
	/* ObjId_4B           */ Obj_Null,
	/* ObjId_4C           */ Obj_Null,
	/* ObjId_4D           */ Obj_Null,
	/* ObjId_4E           */ Obj_Null,
	/* ObjId_4F           */ Obj_Null,
	/* ObjId_50           */ Obj_Null,
	/* ObjId_51           */ Obj_Null,
	/* ObjId_52           */ Obj_Null,
	/* ObjId_53           */ Obj_Null,
	/* ObjId_54           */ Obj_Null,
	/* ObjId_55           */ Obj_Null,
	/* ObjId_56           */ Obj_Null,
	/* ObjId_57           */ Obj_Null,
	/* ObjId_58           */ Obj_Null,
	/* ObjId_59           */ Obj_Null,
	/* ObjId_5A           */ Obj_Null,
	/* ObjId_5B           */ Obj_Null,
	/* ObjId_5C           */ Obj_Null,
	/* ObjId_5D           */ Obj_Null,
	/* ObjId_5E           */ Obj_Null,
	/* ObjId_5F           */ Obj_Null,
	/* ObjId_60           */ Obj_Null,
	/* ObjId_61           */ Obj_Null,
	/* ObjId_62           */ Obj_Null,
	/* ObjId_63           */ Obj_Null,
	/* ObjId_64           */ Obj_Null,
	/* ObjId_65           */ Obj_Null,
	/* ObjId_66           */ Obj_Null,
	/* ObjId_67           */ Obj_Null,
	/* ObjId_68           */ Obj_Null,
	/* ObjId_69           */ Obj_Null,
	/* ObjId_6A           */ Obj_Null,
	/* ObjId_6B           */ Obj_Null,
	/* ObjId_6C           */ Obj_Null,
	/* ObjId_6D           */ Obj_Null,
	/* ObjId_6E           */ Obj_Null,
	/* ObjId_6F           */ Obj_Null,
	/* ObjId_70           */ Obj_Null,
	/* ObjId_71           */ Obj_Null,
	/* ObjId_72           */ Obj_Null,
	/* ObjId_73           */ Obj_Null,
	/* ObjId_74           */ Obj_Null,
	/* ObjId_75           */ Obj_Null,
	/* ObjId_76           */ Obj_Null,
	/* ObjId_77           */ Obj_Null,
	/* ObjId_78           */ Obj_Null,
	/* ObjId_79           */ Obj_Null,
	/* ObjId_7A           */ Obj_Null,
	/* ObjId_7B           */ Obj_Null,
	/* ObjId_7C           */ Obj_Null,
	/* ObjId_7D           */ Obj_Null,
	/* ObjId_7E           */ Obj_Null,
	/* ObjId_7F           */ Obj_Null,
	/* ObjId_80           */ Obj_Null,
	/* ObjId_81           */ Obj_Null,
	/* ObjId_82           */ Obj_Null,
	/* ObjId_83           */ Obj_Null,
	/* ObjId_84           */ Obj_Null,
	/* ObjId_85           */ Obj_Null,
	/* ObjId_86           */ Obj_Null,
	/* ObjId_87           */ Obj_Null,
	/* ObjId_88           */ Obj_Null,
	/* ObjId_89           */ Obj_Null,
	/* ObjId_Credits      */ Obj_Credits,
	/* ObjId_8B           */ Obj_Null,
	/* ObjId_8C           */ Obj_Null,
};

//Object functions
Object *FindFreeObj()
{
	Object *obj = level_objects;
	for (int i = 0; i < LEVEL_OBJECTS; i++, obj++)
		if (obj->type == ObjId_Null)
			return obj;
	return NULL; //Original would return the address at the end of object space, I believe
}


Object *FindNextFreeObj(Object *obj)
{
	for (; (obj - objects) < OBJECTS; obj++)
		if (obj->type == ObjId_Null)
			return obj;
	return NULL; //Original would return the address at the end of object space, I believe
}

void ExecuteObjects()
{
	Object *obj;
	
	if (player->routine < 6)
	{
		//Run all objects
		obj = objects;
		for (int i = 0; i < OBJECTS; i++, obj++)
			if (obj->type)
				object_func[obj->type](obj);
	}
	else
	{
		//Run reserved objects
		obj = objects;
		for (int i = 0; i < RESERVED_OBJECTS; i++, obj++)
			if (obj->type)
				object_func[obj->type](obj);
		
		//Draw level objects
		for (int i = 0; i < LEVEL_OBJECTS; i++, obj++)
			if (obj->type && obj->render.f.on_screen)
				DisplaySprite(obj);
	}
}

//Object drawing
void BuildSprites_Draw(VDP_Sprite **sprite, uint8_t *sprite_i, uint16_t x, uint16_t y, Object *obj, const uint8_t *mappings, uint8_t pieces)
{
	if (obj->render.f.x_flip)
	{
		if (obj->render.f.y_flip)
		{
			//XY flip
			while(pieces-- > 0)
			{
				//Don't overflow the sprite buffer
				if (*sprite_i >= BUFFER_SPRITES)
					break;
				
				//Write sprite
				int8_t py = (int8_t)*mappings++;
				uint8_t size = *mappings++;
				(*sprite)->y = y - py - (((size << 3) & 0x18) + 8);
				(*sprite)->size.s.width = (size >> 2);
				(*sprite)->size.s.height = size;
				(*sprite)->link = ++(*sprite_i);
				uint16_t tile = (*mappings++ << 8) | (*mappings++ << 0);
				(*sprite)->tile.s.priority = obj->tile.s.priority ^ (uint8_t)(tile >> 15);
				(*sprite)->tile.s.palette  = obj->tile.s.palette  + (uint8_t)(tile >> 13);
				(*sprite)->tile.s.y_flip   = obj->tile.s.y_flip   ^ (uint8_t)(tile >> 12) ^ 1;
				(*sprite)->tile.s.x_flip   = obj->tile.s.x_flip   ^ (uint8_t)(tile >> 11) ^ 1;
				(*sprite)->tile.s.pattern  = obj->tile.s.pattern  + (uint16_t)tile;
				uint16_t px = x - ((int8_t)*mappings++) - (((size << 1) & 0x18) + 8);
				#if (SCREEN_WIDTH <= 320)
					if ((px &= 0x1FF) == 0)
						px++; //Prevent sprite from being x=0 (acts as a mask)
				#else
					if (px == 0)
						px++;
				#endif
				(*sprite)->x = px;
				
				(*sprite)++;
			}
		}
		else
		{
			//X flip
			while(pieces-- > 0)
			{
				//Don't overflow the sprite buffer
				if (*sprite_i >= BUFFER_SPRITES)
					break;
				
				//Write sprite
				int8_t py = (int8_t)*mappings++;
				(*sprite)->y = y + py;
				uint8_t size = *mappings++;
				(*sprite)->size.s.width = (size >> 2);
				(*sprite)->size.s.height = size;
				(*sprite)->link = ++(*sprite_i);
				uint16_t tile = (*mappings++ << 8) | (*mappings++ << 0);
				(*sprite)->tile.s.priority = obj->tile.s.priority ^ (uint8_t)(tile >> 15);
				(*sprite)->tile.s.palette  = obj->tile.s.palette  + (uint8_t)(tile >> 13);
				(*sprite)->tile.s.y_flip   = obj->tile.s.y_flip   ^ (uint8_t)(tile >> 12);
				(*sprite)->tile.s.x_flip   = obj->tile.s.x_flip   ^ (uint8_t)(tile >> 11) ^ 1;
				(*sprite)->tile.s.pattern  = obj->tile.s.pattern  + (uint16_t)tile;
				uint16_t px = x - ((int8_t)*mappings++) - (((size << 1) & 0x18) + 8);
				#if (SCREEN_WIDTH <= 320)
					if ((px &= 0x1FF) == 0)
						px++; //Prevent sprite from being x=0 (acts as a mask)
				#else
					if (px == 0)
						px++;
				#endif
				(*sprite)->x = px;
				
				(*sprite)++;
			}
		}
	}
	else if (obj->render.f.y_flip)
	{
		//Y flip
		while(pieces-- > 0)
		{
			//Don't overflow the sprite buffer
			if (*sprite_i >= BUFFER_SPRITES)
				break;
			
			//Write sprite
			int8_t py = (int8_t)*mappings++;
			uint8_t size = *mappings++;
			(*sprite)->y = y - py - (((size << 3) & 0x18) + 8);
			(*sprite)->size.s.width = (size >> 2);
			(*sprite)->size.s.height = size;
			(*sprite)->link = ++(*sprite_i);
			uint16_t tile = (*mappings++ << 8) | (*mappings++ << 0);
			(*sprite)->tile.s.priority = obj->tile.s.priority ^ (uint8_t)(tile >> 15);
			(*sprite)->tile.s.palette  = obj->tile.s.palette  + (uint8_t)(tile >> 13);
			(*sprite)->tile.s.y_flip   = obj->tile.s.y_flip   ^ (uint8_t)(tile >> 12) ^ 1;
			(*sprite)->tile.s.x_flip   = obj->tile.s.x_flip   ^ (uint8_t)(tile >> 11);
			(*sprite)->tile.s.pattern  = obj->tile.s.pattern  + (uint16_t)tile;
			uint16_t px = x + (int8_t)*mappings++;
			#if (SCREEN_WIDTH <= 320)
				if ((px &= 0x1FF) == 0)
					px++; //Prevent sprite from being x=0 (acts as a mask)
			#else
				if (px == 0)
					px++;
			#endif
			(*sprite)->x = px;
			
			(*sprite)++;
		}
	}
	else
	{
		//No flip
		while(pieces-- > 0)
		{
			//Don't overflow the sprite buffer
			if (*sprite_i >= BUFFER_SPRITES)
				break;
			
			//Write sprite
			int8_t py = (int8_t)*mappings++;
			(*sprite)->y = y + py;
			uint8_t size = *mappings++;
			(*sprite)->size.s.width = (size >> 2);
			(*sprite)->size.s.height = size;
			(*sprite)->link = ++(*sprite_i);
			uint16_t tile = (*mappings++ << 8) | (*mappings++ << 0);
			(*sprite)->tile.s.priority = obj->tile.s.priority ^ (uint8_t)(tile >> 15);
			(*sprite)->tile.s.palette  = obj->tile.s.palette  + (uint8_t)(tile >> 13);
			(*sprite)->tile.s.y_flip   = obj->tile.s.y_flip   ^ (uint8_t)(tile >> 12);
			(*sprite)->tile.s.x_flip   = obj->tile.s.x_flip   ^ (uint8_t)(tile >> 11);
			(*sprite)->tile.s.pattern  = obj->tile.s.pattern  + (uint16_t)(tile & 0x7FF);
			uint16_t px = x + (int8_t)*mappings++;
			#if (SCREEN_WIDTH <= 320)
				if ((px &= 0x1FF) == 0)
					px++; //Prevent sprite from being x=0 (acts as a mask)
			#else
				if (px == 0)
					px++;
			#endif
			(*sprite)->x = px;
			
			(*sprite)++;
		}
	}
}

void BuildSprites()
{
	//Draw each sprite priority queue
	VDP_Sprite *sprite = sprite_buffer;
	uint8_t sprite_i = 0;
	struct SpriteQueue *queue = sprite_queue;
	
	for (int i = 0; i < 8; i++, queue++)
	{
		//Iterate through all queued objects
		for (int j = 0; queue->size != 0; j++, queue->size--)
		{
			Object *obj = queue->obj[j];
			if (obj->mappings == NULL) //This line isn't in the original, but without it, the title screen segfaults
				continue;              //Basically, the bug that causes the 'PRESS START BUTTON' text to not appear gives the object null mappings
			if (obj->type != ObjId_Null)
			{
				//Get object position on screen and check if visible
				obj->render.f.on_screen = false;
				
				uint16_t x, y;
				if (obj->render.f.align_bg || obj->render.f.align_fg)
				{
					//Get screen position to use
					static int16_t *bs_scrpos[4][2] = {
						{NULL, NULL},
						{&scrpos_x.f.u,     &scrpos_y.f.u},
						{&bg_scrpos_x.f.u,  &bg_scrpos_y.f.u},
						{&bg3_scrpos_x.f.u, &bg3_scrpos_y.f.u},
					};
					int16_t **scrpos = bs_scrpos[(obj->render.f.align_bg << 1) | obj->render.f.align_fg];
					
					//Get object X position
					int16_t ox = obj->pos.l.x.f.u - *scrpos[0];
					if ((ox + obj->width_pixels) < 0 || (ox - obj->width_pixels) >= SCREEN_WIDTH)
						continue;
					x = 128 + ox; //VDP sprites start at 128
					
					//Get object Y position
					if (obj->render.f.yrad_height)
					{
						int16_t oy = obj->pos.l.y.f.u - *scrpos[1];
						if ((oy + obj->y_rad) < 0 || (oy - obj->y_rad) >= SCREEN_HEIGHT)
							continue;
						y = 128 + oy; //VDP sprites start at 128
					}
					else
					{
						int16_t oy = obj->pos.l.y.f.u - *scrpos[1] + 0x80;
						if (oy < 0x60 || oy >= (0x180 + SCREEN_TALLADD))
							continue;
						y = oy;
					}
				}
				else
				{
					//Positions map directly to VDP coordinates
					x = obj->pos.s.x;
					y = obj->pos.s.y;
				}
				
				//Get object mappings to use
				const uint8_t *mappings;
				uint8_t pieces;
				
				if (!obj->render.f.raw_mappings)
				{
					//Index mapping by frame
					const uint8_t *mapping_ind = (const uint8_t*)obj->mappings + (obj->frame << 1);
					mappings = obj->mappings + ((mapping_ind[0] << 8) | (mapping_ind[1] << 0));
					pieces = *mappings++;
				}
				else
				{
					//Directly use object mappings pointer
					mappings = obj->mappings;
					pieces = 0;
				}
				
				//Draw object
				BuildSprites_Draw(&sprite, &sprite_i, x, y, obj, mappings, pieces);
				obj->render.f.on_screen = true;
			}
		}
	}
	
	//Terminate end of sprite list
	if (sprite_i >= BUFFER_SPRITES)
	{
		sprite[-1].link = 0;
	}
	else
	{
		sprite->y = 0;
		sprite->size.b = 0;
		sprite->link = 0;
	}
}

//Object functions
void AnimateSprite(Object *obj, const uint8_t *anim_script)
{
	//Check if animation changed
	uint8_t anim = obj->anim;
	if (anim != obj->prev_anim)
	{
		//Reset animation state
		obj->prev_anim = anim;
		obj->anim_frame = 0;
		obj->frame_time.b = 0;
	}
	
	//Wait for current animation frame to end
	if (--obj->frame_time.b >= 0)
		return;
	
	//Get animation script to use
	anim <<= 1;
	anim_script += (anim_script[anim] << 8) | (anim_script[anim + 1] << 0);
	obj->frame_time.b = anim_script[0];
	
	//Read current animation command
	uint8_t cmd = anim_script[1 + obj->anim_frame];
	
	if (!(cmd & 0x80))
	{
		Anim_Next:
		//Set animation frame
		obj->frame = cmd & 0x1F;
		obj->render.f.x_flip = obj->status.o.f.x_flip ^ ((cmd >> 5) & 1);
		obj->render.f.y_flip = obj->status.o.f.y_flip ^ ((cmd >> 6) & 1);
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
			//Change animation
			obj->anim = anim_script[2 + obj->anim_frame];
		}
		if (++cmd == 0) //0xFC
		{
			//Increment routine
			obj->routine += 2;
		}
		if (++cmd == 0) //0xFB
		{
			//Clear secondary routine
			obj->routine_sec = 0;
		}
		if (++cmd == 0) //0xFA
		{
			//Increment secondary routine
			obj->routine_sec += 2;
		}
	}
}

void DisplaySprite(Object *obj)
{
	//Get queue to use
	struct SpriteQueue *queue = &sprite_queue[obj->priority & 7];
	
	//Push to queue
	if (queue->size >= (sizeof(queue->obj) / sizeof(Object*)))
		return;
	queue->obj[queue->size++] = obj;
}

void DeleteObject(Object *obj)
{
	//Clear object memory
	memset(obj, 0, sizeof(Object));
	obj->mappings = NULL; //NULL isn't guaranteed to be 0
}

void SpeedToPos(Object *obj)
{
	obj->pos.l.x.v += obj->xsp << 8;
	obj->pos.l.y.v += obj->ysp << 8;
}

void ObjectFall(Object *obj)
{
	obj->pos.l.x.v += obj->xsp << 8;
	obj->pos.l.y.v += obj->ysp << 8;
	obj->ysp += 0x38;
}

void RememberState(Object *obj)
{
	int16_t ox = obj->pos.l.x.f.u & ~0x7F;
	int16_t sx = (scrpos_x.f.u - 0x80) & ~0x7F;
	if ((uint16_t)(ox - sx) > (((SCREEN_WIDTH + 0x80) & ~0x7F) + 0x100))
	{
		//Off-screen
		if (obj->respawn_index)
			objstate[obj->respawn_index] &= 0x7F;
		DeleteObject(obj);
	}
	else
	{
		//On-screen
		DisplaySprite(obj);
	}
}

void PlatformObject(Object *obj, uint16_t x_rad)
{
	//Check if player is colliding with platform
	Object *pla = player;
	
	if (pla->ysp < 0)
		return;
	
	int16_t x_off = pla->pos.l.x.f.u - obj->pos.l.x.f.u + x_rad;
	if (x_off < 0 || x_off >= (x_rad << 1))
		return;
	
	Platform3(obj, pla, obj->pos.l.y.f.u - 8);
}

void Platform3(Object *obj, Object *pla, int16_t top)
{
	Scratch_Sonic *scratch = (Scratch_Sonic*)&pla->scratch;
	
	//Check if player is touching the top of platform
	int16_t py = pla->pos.l.y.f.u;
	int16_t by = py + pla->y_rad + 4;
	if (top > by)
		return;
	top -= by;
	if (top < -16)
		return;
	
	//Check if player can collide with platform
	if (lock_multi || pla->routine >= 6)
		return;
	
	//Clip on top of platform
	pla->pos.l.y.f.u = top + py + 3;
	
	//Modify platform state
	obj->routine += 2;
	
	if (pla->status.p.f.object_stand)
	{
		//Release from last standing object
		Object *prv = objects + scratch->standing_obj;
		prv->status.o.f.player_stand = false;
		prv->routine_sec = 0;
		if (prv->routine == 4)
			prv->routine = 2;
	}
	
	//Modify player state
	scratch->standing_obj = obj - objects;
	pla->angle = 0;
	pla->ysp = 0;
	pla->inertia = pla->xsp;
	if (pla->status.p.f.in_air)
		Sonic_ResetOnFloor(pla);
	
	pla->status.p.f.object_stand = true;
	obj->status.o.f.player_stand = true;
}

bool ExitPlatform(Object *obj, int16_t x_rad, int16_t x_dia2, int16_t *x_off_p)
{
	int16_t x_dia = x_dia2 << 1;
	Object *pla = player;
	
	//Check if we've jumped off
	if (!pla->status.p.f.in_air)
	{
		//Check if we've walked off
		int16_t x_off = pla->pos.l.x.f.u - obj->pos.l.x.f.u + x_rad;
		if (x_off >= 0 && x_off < x_dia)
		{
			if (x_off_p != NULL)
				*x_off_p = x_off;
			return false;
		}
	}
	
	//Release player from platform
	pla->status.p.f.object_stand = false;
	obj->routine = 2;
	obj->status.o.f.player_stand = false;
	return true;
}
