#include "HUD.h"

#include "Level.h"
#include "LevelScroll.h"
#include "Game.h"
#include "Video.h"
#include "Object/Sonic.h"

#include <stdint.h>

//HUD assets
static const uint8_t art_hud_num[] = {
	#include "Resource/Art/HUDNum.h"
};
static const uint8_t art_life_num[] = {
	#include "Resource/Art/LifeNum.h"
};

//HUD art writing
static const uint32_t hud_dec[] = {
	100000,
	10000,
	1000,
	100,
	10,
	1
};

static const uint8_t hud_cmd_base[] = {
	/*Hud_TilesBase:*/0x16, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x14, 0x00, 0x00,
	/*Hud_TilesZero:*/0xFF, 0xFF, 0x00
};
static const uint8_t *hud_cmd_ringbase = &hud_cmd_base[12];

void HUD_WriteCmd(size_t offset, const uint8_t *cmd, size_t cmds)
{
	VDP_SeekVRAM(offset);
	
	do
	{
		//Get tile
		int16_t tile = (int8_t)*cmd++;
		if (tile >= 0)
		{
			const uint8_t *art = art_hud_num + (tile <<= 5);
			VDP_WriteVRAM(art, 64);
		}
		else
		{
			VDP_FillVRAM(0, 64);
		}
	} while (cmds-- > 0);
}

void HUD_WriteNumber(size_t offset, uint32_t value, const uint32_t *dec, size_t decs)
{
	bool digit_write = false;
	do
	{
		//Get digit
		uint16_t digit;
		for (digit = 0; value >= *dec; digit++, value -= *dec);
		
		//Write digit
		if (digit)
			digit_write = true;
		if (digit_write)
		{
			const uint8_t *art = art_hud_num + (digit <<= 6);
			VDP_SeekVRAM(offset);
			VDP_WriteVRAM(art, 64);
		}
		
		//Increment offset and decimal
		offset += 64;
		dec++;
	} while (decs-- > 0);
}

void HUD_WriteNumber2(size_t offset, uint32_t value, const uint32_t *dec, size_t decs)
{
	do
	{
		//Get digit
		uint16_t digit;
		for (digit = 0; value >= *dec; digit++, value -= *dec);
		
		//Write digit
		const uint8_t *art = art_hud_num + (digit <<= 6);
		VDP_SeekVRAM(offset);
		VDP_WriteVRAM(art, 64);
		
		//Increment offset and decimal
		offset += 64;
		dec++;
	} while (decs-- > 0);
}

void HUD_WriteHex(size_t offset, uint32_t value)
{
	VDP_SeekVRAM(offset);
	
	size_t decs = 7;
	do
	{
		//Get digit
		value = (value & 0xFFFF0000) | (((value & 0x0FFF) << 4) | ((value & 0xF000) >> 12));
		
		uint16_t digit = value & 0xF;
		if (digit >= 0xA)
			digit += 7;
		
		//Write digit
		const uint8_t *art = art_text + (digit <<= 5);
		VDP_WriteVRAM(art, 32);
		value = (value >> 16) | (value << 16);
	} while (decs-- > 0);
}

//HUD functions
void HUD_Lives()
{
	size_t offset = 0xFBA0;
	
	uint32_t value = lives;
	
	const uint32_t *dec = &hud_dec[4];
	size_t decs = 1;
	
	bool digit_write = false;
	do
	{
		//Get digit
		uint16_t digit;
		for (digit = 0; value >= *dec; digit++, value -= *dec);
		
		//Write digit
		if (digit)
			digit_write = true;
		if (digit_write || decs == 0)
		{
			const uint8_t *art = art_life_num + (digit <<= 5);
			VDP_SeekVRAM(offset);
			VDP_WriteVRAM(art, 32);
		}
		else
		{
			VDP_SeekVRAM(offset);
			VDP_FillVRAM(0, 32);
		}
		
		//Increment offset and decimal
		offset += 64;
		dec++;
	} while (decs-- > 0);
}

void HUD_Base()
{
	//Write lives and initial HUD cmd
	HUD_Lives();
	HUD_WriteCmd(0xDC40, hud_cmd_base, 14);
}

void HUD_Update()
{
	if (!debug_mode)
	{
		//Update score
		if (score_count)
		{
			score_count = false;
			HUD_WriteNumber(0xDC80, score, &hud_dec[0], 5);
		}
		
		//Update rings
		if (ring_count)
		{
			if (ring_count & 0x80)
			{
				//Set rings to 0
				HUD_WriteCmd(0xDF40, hud_cmd_ringbase, 2);
				//Fallthrough to the below code?
			}
			
			//Update rings count
			ring_count = false;
			HUD_WriteNumber(0xDF40, rings, &hud_dec[3], 2);
		}
		
		//Update time
		if (time_count)
		{
			//Time Over if time is 9:59:59(frames)
			if (time.pad == 0 && time.min == 9 && time.sec == 59 && time.frame == 59)
			{
				time_count = false;
				KillSonic(player, player);
				time_over = true;
				return;
			}
			
			//Increment time
			if (++time.frame >= 60)
			{
				time.frame = 0;
				if (++time.sec >= 60)
				{
					time.sec = 0;
					if (++time.min > 9)
						time.min = 9;
				}
			}
			
			//Write time
			HUD_WriteNumber2(0xDE40, time.min, &hud_dec[5], 0);
			HUD_WriteNumber2(0xDEC0, time.sec, &hud_dec[4], 1);
		}
		
		//Update lives
		if (life_count)
			HUD_Lives();
	}
	else
	{
		//Update position
		HUD_WriteHex(0xDC40, (scrpos_x.f.u << 16) | player->pos.l.x.f.u);
		HUD_WriteHex(0xDD40, (scrpos_y.f.u << 16) | player->pos.l.y.f.u);
		
		//Update rings
		if (ring_count)
		{
			if (ring_count & 0x80)
			{
				//Set rings to 0
				HUD_WriteCmd(0xDF40, hud_cmd_ringbase, 2);
				//Fallthrough to the below code?
			}
			
			//Update rings count
			ring_count = false;
			HUD_WriteNumber(0xDF40, rings, &hud_dec[3], 2);
		}
		
		//Update sprite count
		HUD_WriteNumber2(0xDEC0, sprite_count, &hud_dec[4], 1);
	}
}
