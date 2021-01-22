#include "Object.h"

#include "Video.h"
#include "LevelScroll.h"

#include <string.h>

//Object draw queue
struct SpriteQueue
{
	uint16_t size;
	Object *obj[0x3F];
} sprite_queue[8];

//Object functions

//Object drawing
void BuildSprites_Draw(VDP_Sprite **sprite, uint8_t *sprite_i, uint16_t x, uint16_t y, Object *obj, const uint8_t *mappings, uint8_t pieces)
{
	if (obj->render.f.x_flip)
	{
		if (obj->render.f.y_flip)
		{
			//XY flip
			do
			{
				//Don't overflow the sprite buffer
				if (*sprite_i >= BUFFER_SPRITES)
					break;
				
				//Write sprite
				int8_t py = (int8_t)*mappings++;
				uint8_t size = *mappings++;
				(*sprite)->y = y + py - (((size << 3) & 0x1B) + 8);
				(*sprite)->info.w = (size << 8) | (*sprite_i)++;
				uint16_t tile = (*mappings++ << 8) | (*mappings++ << 0);
				(*sprite)->tile.w = (obj->tile.w + tile) ^ 0x1800;
				uint16_t px = x + ((int8_t)*mappings++) - (((size << 1) & 0x1B) + 8);
				if ((px &= 0x1FF) == 0)
					px++; //Prevent sprite from being x=0 (acts as a mask)
				(*sprite)->x = px;
				
				(*sprite)++;
			} while(pieces-- > 0);
		}
		else
		{
			//X flip
			do
			{
				//Don't overflow the sprite buffer
				if (*sprite_i >= BUFFER_SPRITES)
					break;
				
				//Write sprite
				int8_t py = (int8_t)*mappings++;
				(*sprite)->y = y + py;
				uint8_t size = *mappings++;
				(*sprite)->info.w = (size << 8) | (*sprite_i)++;
				uint16_t tile = (*mappings++ << 8) | (*mappings++ << 0);
				(*sprite)->tile.w = (obj->tile.w + tile) ^ 0x1800;
				uint16_t px = x + ((int8_t)*mappings++) - (((size << 1) & 0x1B) + 8);
				if ((px &= 0x1FF) == 0)
					px++; //Prevent sprite from being x=0 (acts as a mask)
				(*sprite)->x = px;
				
				(*sprite)++;
			} while(pieces-- > 0);
		}
	}
	else if (obj->render.f.y_flip)
	{
		//Y flip
		do
		{
			//Don't overflow the sprite buffer
			if (*sprite_i >= BUFFER_SPRITES)
				break;
			
			//Write sprite
			int8_t py = (int8_t)*mappings++;
			uint8_t size = *mappings++;
			(*sprite)->y = y + py - (((size << 3) & 0x1B) + 8);
			(*sprite)->info.w = (size << 8) | (*sprite_i)++;
			uint16_t tile = (*mappings++ << 8) | (*mappings++ << 0);
			(*sprite)->tile.w = (obj->tile.w + tile) ^ 0x1000;
			uint16_t px = x + (int8_t)*mappings++;
			if ((px &= 0x1FF) == 0)
				px++; //Prevent sprite from being x=0 (acts as a mask)
			(*sprite)->x = px;
			
			(*sprite)++;
		} while(pieces-- > 0);
	}
	else
	{
		//No flip
		do
		{
			//Don't overflow the sprite buffer
			if (*sprite_i >= BUFFER_SPRITES)
				break;
			
			//Write sprite
			int8_t py = (int8_t)*mappings++;
			(*sprite)->y = y + py;
			uint8_t size = *mappings++;
			(*sprite)->info.w = (size << 8) | (*sprite_i)++;
			uint16_t tile = (*mappings++ << 8) | (*mappings++ << 0);
			(*sprite)->tile.w = obj->tile.w + tile;
			uint16_t px = x + (int8_t)*mappings++;
			if ((px &= 0x1FF) == 0)
				px++; //Prevent sprite from being x=0 (acts as a mask)
			(*sprite)->x = px;
			
			(*sprite)++;
		} while(pieces-- > 0);
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
					
					//Get object mappings to use
					const uint8_t *mappings;
					uint8_t pieces;
					
					if (!obj->render.f.raw_mappings)
					{
						//Index mapping by frame
						const uint16_t *mapping_ind = (const uint16_t*)obj->mappings;
						mappings = obj->mappings + mapping_ind[obj->frame];
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
				else
				{
					//Positions map directly to VDP coordinates
					x = obj->pos.s.x;
					y = obj->pos.s.y;
				}
			}
		}
	}
	
	//Terminate end of sprite list
	sprite[-1].info.s.link = 0;
}
