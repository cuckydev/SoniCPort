#include "Video.h"

#include "Constants.h"
#include "Palette.h"

#include <string.h>

//Video state
uint8_t vbla_routine;

int16_t scrposy_dup, bgscrposy_dup, scrposx_dup, bgscrposx_dup;

uint8_t wtr_state;

uint64_t sprite_buffer[0x50];
uint16_t hscroll_buffer[SCREEN_HEIGHT][2];

//Video interface
void VDPSetupGame()
{
	//Initialize VDP state
	VDP_SetPlaneALocation(VRAM_FG);
	VDP_SetPlaneBLocation(VRAM_BG);
	VDP_SetSpriteLocation(VRAM_SPRITES);
	VDP_SetHScrollLocation(VRAM_HSCROLL);
	VDP_SetPlaneSize(64, 32);
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
	VDP_WriteVRAM(VRAM_SPRITES, (uint8_t*)sprite_buffer, sizeof(sprite_buffer));
	VDP_WriteVRAM(VRAM_HSCROLL, (uint8_t*)hscroll_buffer, sizeof(hscroll_buffer));
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
	bgscrposx_dup = 0;
	
	//Clear sprite buffer and hscroll buffer
	memset(sprite_buffer, 0, sizeof(sprite_buffer));
	memset(hscroll_buffer, 0, sizeof(hscroll_buffer));
}

void CopyTilemap(const uint8_t *tilemap, size_t offset, size_t width, size_t height)
{
	width++;
	do
	{
		for (size_t x = 0; x < width; x++)
		{
			uint8_t v0 = *tilemap++, v1 = *tilemap++;
			#ifdef SCP_LIL_ENDIAN
				VDP_WriteVRAM(offset++, &v1, 1);
				VDP_WriteVRAM(offset++, &v0, 1);
			#else
				VDP_WriteVRAM(offset++, &v0, 1);
				VDP_WriteVRAM(offset++, &v1, 1);
			#endif
		}
		offset += 128 - (width << 1);
	}
	while (height-- > 0);
}
