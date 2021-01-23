#include "Object.h"

#include "Video.h"
#include "Level.h"
#include "LevelScroll.h"

#include <string.h>

//Object draw queue
struct SpriteQueue
{
	uint8_t size;
	Object *obj[0x3F];
} sprite_queue[8];

//Object execution
void Obj_TitleSonic();

static void (*object_func[])(Object*) = {
	/* ObjId_Null       */ NULL,
	/* ObjId_Sonic      */ NULL,
	/* ObjId_02         */ NULL,
	/* ObjId_03         */ NULL,
	/* ObjId_04         */ NULL,
	/* ObjId_05         */ NULL,
	/* ObjId_06         */ NULL,
	/* ObjId_07         */ NULL,
	/* ObjId_08         */ NULL,
	/* ObjId_09         */ NULL,
	/* ObjId_0A         */ NULL,
	/* ObjId_0B         */ NULL,
	/* ObjId_0C         */ NULL,
	/* ObjId_0D         */ NULL,
	/* ObjId_TitleSonic */ Obj_TitleSonic,
	/* ObjId_0F         */ NULL,
};

void ExecuteObjects()
{
	//TODO: checks Sonic's routine
	//Run all objects
	Object *obj = objects;
	for (int i = 0; i < OBJECTS; i++, obj++)
	{
		if (obj->type && object_func[obj->type] != NULL)
			object_func[obj->type](obj);
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
				(*sprite)->tile.s.y_flip   = obj->tile.s.y_flip   ^ (uint8_t)(tile >> 13) ^ 1;
				(*sprite)->tile.s.x_flip   = obj->tile.s.x_flip   ^ (uint8_t)(tile >> 12) ^ 1;
				(*sprite)->tile.s.palette  = obj->tile.s.palette  + (uint8_t)(tile >> 11);
				(*sprite)->tile.s.pattern  = obj->tile.s.pattern  + (uint16_t)tile;
				uint16_t px = x - ((int8_t)*mappings++) - (((size << 1) & 0x18) + 8);
				if ((px &= 0x1FF) == 0)
					px++; //Prevent sprite from being x=0 (acts as a mask)
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
				(*sprite)->tile.s.y_flip   = obj->tile.s.y_flip   ^ (uint8_t)(tile >> 13);
				(*sprite)->tile.s.x_flip   = obj->tile.s.x_flip   ^ (uint8_t)(tile >> 12) ^ 1;
				(*sprite)->tile.s.palette  = obj->tile.s.palette  + (uint8_t)(tile >> 11);
				(*sprite)->tile.s.pattern  = obj->tile.s.pattern  + (uint16_t)tile;
				uint16_t px = x - ((int8_t)*mappings++) - (((size << 1) & 0x18) + 8);
				if ((px &= 0x1FF) == 0)
					px++; //Prevent sprite from being x=0 (acts as a mask)
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
			(*sprite)->tile.s.y_flip   = obj->tile.s.y_flip   ^ (uint8_t)(tile >> 13) ^ 1;
			(*sprite)->tile.s.x_flip   = obj->tile.s.x_flip   ^ (uint8_t)(tile >> 12);
			(*sprite)->tile.s.palette  = obj->tile.s.palette  + (uint8_t)(tile >> 11);
			(*sprite)->tile.s.pattern  = obj->tile.s.pattern  + (uint16_t)tile;
			uint16_t px = x + (int8_t)*mappings++;
			if ((px &= 0x1FF) == 0)
				px++; //Prevent sprite from being x=0 (acts as a mask)
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
			(*sprite)->tile.s.y_flip   = obj->tile.s.y_flip   ^ (uint8_t)(tile >> 13);
			(*sprite)->tile.s.x_flip   = obj->tile.s.x_flip   ^ (uint8_t)(tile >> 12);
			(*sprite)->tile.s.palette  = obj->tile.s.palette  + (uint8_t)(tile >> 11);
			(*sprite)->tile.s.pattern  = obj->tile.s.pattern  + (uint16_t)tile;
			uint16_t px = x + (int8_t)*mappings++;
			if ((px &= 0x1FF) == 0)
				px++; //Prevent sprite from being x=0 (acts as a mask)
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
						{&scrposx.f.u,    &scrposy.f.u},
						{&bgscrposx.f.u,  &bgscrposy.f.u},
						{&bg3scrposx.f.u, &bg3scrposy.f.u},
					};
					int16_t **scrpos = bs_scrpos[(obj->render.f.align_bg << 1) | obj->render.f.align_fg];
					
					//Get object X position
					int16_t ox = obj->pos.l.x.f.u - *scrpos[0];
					if ((ox + obj->width_pixels) < 0 || (ox - obj->width_pixels) >= SCREEN_WIDTH)
						continue;
					x = 128 + ox; //VDP sprites start at 128
					
					//Get object Y position
					if (!obj->render.f.assume_height)
					{
						int16_t oy = obj->pos.l.y.f.u - *scrpos[1];
						if ((oy + obj->y_rad) < 0 || (ox - obj->y_rad) >= SCREEN_HEIGHT)
							continue;
						y = 128 + oy; //VDP sprites start at 128
					}
					else
					{
						int16_t oy = obj->pos.l.y.f.u - *scrpos[1] + 0x80;
						if (oy < 0x60 || oy >= (0x180 + SCREEN_TALLADD))
							continue;
						y = 128 + oy; //VDP sprites start at 128
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
		obj->frame_time = 0;
	}
	
	//Wait for current animation frame to end
	if (--obj->frame_time >= 0)
		return;
	
	//Get animation script to use
	anim <<= 1;
	anim_script += (anim_script[anim] << 8) | (anim_script[anim + 1] << 0);
	obj->frame_time = anim_script[0];
	
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
			//Change animation (falls through to the routine increment below)
			obj->anim = anim_script[2 + obj->anim_frame];
		}
		if (++cmd == 0) //0xFC
		{
			//Increment routine (falls through to the routine secondary code below)
			obj->routine += 2;
		}
		if (++cmd == 0) //0xFB
		{
			//Clear secondary routine (falls through to the increment below)
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
	struct SpriteQueue *queue = &sprite_queue[obj->priority];
	
	//Push to queue
	if (queue->size >= (sizeof(queue->obj) / sizeof(Object*)))
		return;
	queue->obj[queue->size++] = obj;
}
