#include "Video.h"

#include <Constants.h>
#include "Palette.h"
#include "LevelScroll.h"

#include <string.h>

//Video state
uint8_t vbla_routine;

uint8_t wtr_state;

uint64_t sprite_buffer[0x50];
int16_t hscroll_buffer[SCREEN_HEIGHT][2];

//Video interface
void VDPSetupGame()
{
	//Initialize VDP state
	VDP_SetPlaneALocation(VRAM_FG);
	VDP_SetPlaneBLocation(VRAM_BG);
	VDP_SetSpriteLocation(VRAM_SPRITES);
	VDP_SetHScrollLocation(VRAM_HSCROLL);
	VDP_SetPlaneSize(PLANE_WIDTH, PLANE_HEIGHT);
	VDP_SetBackgroundColour(0);
	
	//Clear VRAM and CRAM
	VDP_FillVRAM(0, 0x00, 0x10000);
	VDP_FillCRAM(0, 0x0000, 64);
	
	//Clear internal palette
	memset(dry_palette, 0, sizeof(dry_palette));
	memset(dry_palette_dup, 0, sizeof(dry_palette_dup));
	memset(wet_palette, 0, sizeof(wet_palette));
	memset(wet_palette_dup, 0, sizeof(wet_palette_dup));
}

void VDPSetupFrame()
{
	//Read controllers
	
	//Copy palette
	#if 1
		if (wtr_state)
			VDP_WriteCRAM(0, &wet_palette[0][0], 0x40);
		else
			VDP_WriteCRAM(0, &dry_palette[0][0], 0x40);
	#else
		srand(0);
		uint16_t pal[0x40];
		for (int i = 0; i < 0x40; i++)
			pal[i] = rand() & 0xEEE;
		VDP_WriteCRAM(0, pal, 0x40);
	#endif
	
	//Copy buffers
	VDP_WriteVRAM(VRAM_SPRITES, (const uint8_t*)sprite_buffer, sizeof(sprite_buffer));
	VDP_WriteVRAM(VRAM_HSCROLL, (const uint8_t*)hscroll_buffer, sizeof(hscroll_buffer));
}

int WaitForVBla()
{
	return VDP_Render();
}

void ClearScreen()
{
	//Clear foreground and background planes
	VDP_FillVRAM(VRAM_FG, 0x00, PLANE_SIZE);
	VDP_FillVRAM(VRAM_BG, 0x00, PLANE_SIZE);
	
	//Reset screen position duplicates
	scrposy_dup = 0;
	bgscrposy_dup = 0;
	scrposx_dup = 0;
	bg3scrposy_dup = 0;
	
	//Clear sprite buffer and hscroll buffer
	memset(sprite_buffer, 0, sizeof(sprite_buffer));
	memset(hscroll_buffer, 0, sizeof(hscroll_buffer));
}

void CopyTilemap(const uint8_t *tilemap, size_t offset, size_t width, size_t height)
{
	while (height-- > 0)
	{
		for (size_t x = 0; x < width; x++)
		{
			uint16_t v = (*tilemap++ << 8) | (*tilemap++ << 0);
			VDP_WriteVRAM(offset + (x << 1), (const uint8_t*)&v, 2);
		}
		offset += PLANE_WIDTH * 2;
	}
}
