#include "Object.h"

#include "Game.h"
#include "Level.h"
#include "PLC.h"

//Game Over card assets
static const uint8_t map_game_over[] = {
	#include <Resource/Mappings/GameOver.h>
};

//Game Over card constants
#define TO_ADD   SCREEN_WIDEADD2
#define FROM_ADD (TO_ADD + ((SCREEN_WIDEADD2 + 0xF) & ~0xF))
#define FROM_SUB ((0x10 - TO_ADD) & 0xF)

//Game Over card object
void Obj_GameOverCard(Object *obj)
{
	switch (obj->routine)
	{
		case 0: //Initialization
			//Wait for art to be finished loading
			if (plc_buffer[0].art != NULL)
				break;
			
			//Increment routine and set position
			obj->routine += 2;
			
			obj->pos.s.x = 0x50 - FROM_SUB;
			if (obj->frame & 1)
				obj->pos.s.x = 0x1F0 + FROM_ADD;
			obj->pos.s.y = 0xF0 + SCREEN_TALLADD2;
			
			//Set object drawing information
			obj->mappings = map_game_over;
			obj->tile = TILE_MAP(1, 0, 0, 0, 0x55E);
			obj->render.b = 0;
			obj->priority = 0;
	//Fallthrough
		case 2: //Moving
			if (obj->pos.s.x == (0x120 + TO_ADD))
			{
				//Start waiting for restart
				obj->frame_time.w = 720;
				obj->routine += 2;
				//Doesn't call DisplaySprite or fallthrough
			}
			else
			{
				//Move
				if (obj->pos.s.x > (0x120 + TO_ADD))
					obj->pos.s.x -= 16;
				else if (obj->pos.s.x < (0x120 + TO_ADD))
					obj->pos.s.x += 16;
				
				//Draw
				DisplaySprite(obj);
			}
			break;
		case 4:
			if (!(jpad1_hold1 & (JPAD_A | JPAD_C | JPAD_B)))
			{
				//Don't change handle timer if 'OVER'
				if (obj->frame & 1)
				{
					DisplaySprite(obj);
					break;
				}
				
				//Decrement timer
				if (obj->frame_time.w)
				{
					obj->frame_time.w--;
					DisplaySprite(obj);
					break;
				}
			}
			
			//Change gamemode
			if (!time_over)
			{
				//Go to continue screen, or SEGA screen if we have no continues
				gamemode = continues ? GameMode_Continue : GameMode_Sega;
			}
			else
			{
				//Restart level
				#ifndef SCP_REV00
					//lamp_time = 0; //TODO
				#endif
				restart = true;
			}
			
			DisplaySprite(obj);
			break;
	}
}
