#include "Object.h"

#include <Constants.h>
#include "Level.h"
#include "PLC.h"

//Title card configuration
static const struct TitleCard_Item
{
	int16_t y;
	uint8_t routine, frame;
} titlecard_item[4] = {
	{0x00D0 + SCREEN_TALLADD2, 0x02, 0x00},
	{0x00E4 + SCREEN_TALLADD2, 0x02, 0x06},
	{0x00EA + SCREEN_TALLADD2, 0x02, 0x07},
	{0x00E0 + SCREEN_TALLADD2, 0x02, 0x0A},
};

#define TO_ADD   SCREEN_WIDEADD2
#define FROM_ADD (TO_ADD + ((SCREEN_WIDEADD2 + 0xF) & ~0xF))
#define FROM_SUB ((0x10 - TO_ADD) & 0xF)

static const struct TitleCard_Config
{
	int16_t x0, x1;
} titlecard_config[7][4] = {
	{{0x0000 - FROM_SUB, 0x0120 + TO_ADD}, {-0x0104 - FROM_SUB, 0x013C + TO_ADD}, {0x0414 + FROM_ADD, 0x0154 + TO_ADD}, {0x0214 + FROM_ADD, 0x0154 + TO_ADD}}, //GHZ
	{{0x0000 - FROM_SUB, 0x0120 + TO_ADD}, {-0x010C - FROM_SUB, 0x0134 + TO_ADD}, {0x040C + FROM_ADD, 0x014C + TO_ADD}, {0x020C + FROM_ADD, 0x014C + TO_ADD}}, //LZ
	{{0x0000 - FROM_SUB, 0x0120 + TO_ADD}, {-0x0120 - FROM_SUB, 0x0120 + TO_ADD}, {0x03F8 + FROM_ADD, 0x0138 + TO_ADD}, {0x01F8 + FROM_ADD, 0x0138 + TO_ADD}}, //MZ
	{{0x0000 - FROM_SUB, 0x0120 + TO_ADD}, {-0x0104 - FROM_SUB, 0x013C + TO_ADD}, {0x0414 + FROM_ADD, 0x0154 + TO_ADD}, {0x0214 + FROM_ADD, 0x0154 + TO_ADD}}, //SLZ
	{{0x0000 - FROM_SUB, 0x0120 + TO_ADD}, {-0x00FC - FROM_SUB, 0x0144 + TO_ADD}, {0x041C + FROM_ADD, 0x015C + TO_ADD}, {0x021C + FROM_ADD, 0x015C + TO_ADD}}, //SYZ
	{{0x0000 - FROM_SUB, 0x0120 + TO_ADD}, {-0x00FC - FROM_SUB, 0x0144 + TO_ADD}, {0x041C + FROM_ADD, 0x015C + TO_ADD}, {0x021C + FROM_ADD, 0x015C + TO_ADD}}, //SBZ
	{{0x0000 - FROM_SUB, 0x0120 + TO_ADD}, {-0x011C - FROM_SUB, 0x0124 + TO_ADD}, {0x03EC + FROM_ADD, 0x03EC + TO_ADD}, {0x01EC + FROM_ADD, 0x012C + TO_ADD}}, //FZ
};

//Title card mappings
static const uint8_t map_titlecard[] = {
	#include <Resource/Mappings/TitleCard.h>
};

//Title card object
typedef struct
{
	uint8_t pad[8];   //0x28-0x2F
	int16_t main_x;   //0x30
	int16_t final_x;  //0x32
} Scratch_TitleCard;

void Obj_TitleCard(Object *obj)
{
	Scratch_TitleCard *scratch = (Scratch_TitleCard*)&obj->scratch;
	
	switch (obj->routine)
	{
		case 0: //Initialization
		{
			Object *a1 = obj;
			
			//Get title card to render
			uint8_t d0 = LEVEL_ZONE(level_id);
			if (level_id == LEVEL_ID(ZoneId_LZ, 3))
				d0 = 5;
			
			uint16_t d2 = d0;
			if (level_id == LEVEL_ID(ZoneId_SBZ, 2))
			{
				d0 = 6;
				d2 = 11;
			}
			
			//Get configuration
			const struct TitleCard_Config *config = &titlecard_config[d0][0];
			const struct TitleCard_Item *item = titlecard_item;
			
			//Create card objects
			for (int i = 0; i < 4; i++, config++, item++, a1++)
			{
				//Write object info from configuration
				Scratch_TitleCard *scratch_a1 = (Scratch_TitleCard*)&a1->scratch;
				a1->type = ObjId_TitleCard;
				a1->pos.s.x = config->x0;
				scratch_a1->final_x = config->x0;
				scratch_a1->main_x = config->x1;
				a1->pos.s.y = item->y;
				a1->routine = item->routine;
				if ((d0 = item->frame) == 0)
					d0 = d2;
				
				//Initialize object graphics
				if (d0 == 7)
				{
					d0 += LEVEL_ACT(level_id);
					if (LEVEL_ACT(level_id) == 3)
						d0--;
				}
				
				a1->frame = d0;
				a1->mappings = map_titlecard;
				a1->tile.w = 0;
				a1->tile.s.priority = true;
				a1->tile.s.pattern = 0x580;
				a1->width_pixels = 0;
				a1->render.b = 0;
				a1->priority = 0;
				a1->frame_time.b = 60;
			}
		}
	//Fallthrough
		case 2: //Moving to on-screen position
			//Move
			if (obj->pos.s.x > scratch->main_x)
				obj->pos.s.x -= 16;
			else if (obj->pos.s.x < scratch->main_x)
				obj->pos.s.x += 16;
			
			//Draw
			if (obj->pos.s.x >= 0 && obj->pos.s.x < (0x200 + SCREEN_WIDEADD))
				DisplaySprite(obj);
			break;
		case 4: //Moving off-screen
		case 6:
			//Wait for timer to expire
			if (obj->frame_time.b)
			{
				obj->frame_time.b--;
				DisplaySprite(obj);
				break;
			}
			
			//Delete and/or load level art once off-screen
			if (!obj->render.f.on_screen || obj->pos.s.x == scratch->final_x)
			{
				if (obj->routine == 4)
				{
					AddPLC(PlcId_Explode);
					AddPLC(PlcId_GHZAnimals + LEVEL_ZONE(level_id));
				}
				DeleteObject(obj);
				break;
			}
			
			//Move
			if (obj->pos.s.x >= scratch->final_x)
				obj->pos.s.x -= 16;
			else
				obj->pos.s.x += 16;
			
			//Draw
			if (obj->pos.s.x >= 0 && obj->pos.s.x < (0x200 + SCREEN_WIDEADD))
				DisplaySprite(obj);
			break;
	}
}
