#include "GM_Sega.h"

#include "Constants.h"
#include "Game.h"
#include "Video.h"
#include "Palette.h"

#include <Backend/VDP.h>

//SEGA art
#ifdef SCP_REV00
	static const uint8_t art_sega[] = {
		#include <Resource/Art/SegaREV00.h>
	};
	static const uint8_t map_sega[] = {
		#include <Resource/Tilemap/SegaREV00.h>
	};
#else
	static const uint8_t art_sega[] = {
		#include <Resource/Art/SegaREV01.h>
	};
	static const uint8_t map_sega[] = {
		#include <Resource/Tilemap/SegaREV01.h>
	};
#endif

//SEGA palette cycle
static const uint8_t pal_sega1[] = {
	#include <Resource/Palette/Sega1.h>
};
static const uint8_t pal_sega2[] = {
	#include <Resource/Palette/Sega2.h>
};

int PalCycle_Sega()
{
	uint16_t *a1;
	const uint8_t *a0;
	int16_t d0, d1;
	
	if (!(pcyc_time & 0x00FF))
	{
		//Get palette pointers to use
		a1 = &dry_palette[1][0];
		a0 = pal_sega1;
		
		//Get area of palette to copy, clipping at 0
		d1 = 6;
		d0 = pcyc_num;
		
		while (d0 < 0)
		{
			a0 += 2;
			d1--;
			d0 += 2;
		}
		
		//Write palette
		a1 += d0 >> 1;
		while (d1-- > 0)
		{
			if (!((a1 - &dry_palette[1][0]) & 0xF))
				a1++;
			if (a1 < &dry_palette[4][0])
				*a1++ = (*a0++ << 8) | (*a0++ << 0);
			else
				a1++;
		}
		
		//Handle cycle timer
		if (!((d0 = pcyc_num + 2) & 0x1E))
			d0 += 2;
		
		if (d0 >= 0x64)
		{
			pcyc_time = 0x0401;
			d0 = -0xC;
		}
		
		pcyc_num = d0;
		
		return 1;
	}
	else
	{
		//Palette timer
		pcyc_time = (((uint8_t)(pcyc_time >> 8) - 1) << 8) | (pcyc_time & 0x00FF);
		if (!(pcyc_time & 0x8000))
			return 1;
		pcyc_time = 0x0400 | (pcyc_time & 0x00FF);
		
		//Get palette index
		if ((d0 = (pcyc_num + 0xC)) >= 0x30)
			return 0;
		
		//Get palette to copy
		pcyc_num = d0;
		a0 = pal_sega2 + d0;
		
		//Copy border palette
		a1 = &dry_palette[0][2];
		for (size_t i = 0; i < 5; i++)
			*a1++ = (*a0++ << 8) | (*a0++ << 0);
		
		//Copy filled palette
		a1 = &dry_palette[1][0];
		for (size_t i = 0; i < (0x30 - 3); i++)
		{
			if (!((a1 - &dry_palette[1][0]) & 0xF))
				a1++;
			*a1++ = (a0[0] << 8) | (a0[1] << 0);
		}
		
		return 1;
	}
}

//SEGA gamemode
int GM_Sega()
{
	int result;
	
	//Stop music
	//sfx	bgm_Stop,0,1,1
	
	//Clear the pattern load queue and fade out
	if ((result = PaletteFadeOut()))
		return result;
	
	//Set VDP state
	VDP_SetPlaneALocation(VRAM_FG);
	VDP_SetPlaneBLocation(VRAM_BG);
	VDP_SetBackgroundColour(0);
	
	//Clear screen and load SEGA graphics
	ClearScreen();
	
	VDP_WriteVRAM(0, art_sega, sizeof(art_sega));
	
	CopyTilemap(&map_sega[0x0000], 0xE510, 23, 7);
	CopyTilemap(&map_sega[0x0180], 0xC000, 39, 27);
	#ifdef SCP_JP
		CopyTilemap(&map_sega[0x0A40], 0xC53A, 2, 1); //Hide trademark symbol
	#endif
	
	//Load palette and initialize cycle
	PalLoad2(PalId_SegaBG);
	pcyc_num = -10;
	pcyc_time = 0x0000;
	pcyc_buffer[6] = 0;
	pcyc_buffer[5] = 0;
	
	//Run palette cycle
	do
	{
		vbla_routine = 0x02;
		if ((result = WaitForVBla()))
			return result;
	}
	while (PalCycle_Sega());
	
	//Play "SEGA" sound
	
	vbla_routine = 0x14;
	if ((result = WaitForVBla()))
		return result;
	
	//Wait a bit before resuming
	demo_length = 30;
	do
	{
		vbla_routine = 0x02;
		if ((result = WaitForVBla()))
			return result;
		if (!demo_length)
			break;
	} while (1); //TODO: start button isn't pressed
	
	return 0;
}
