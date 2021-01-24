#include "Video.h"

#include <Constants.h>
#include "Palette.h"
#include "LevelScroll.h"

#include <string.h>

//Video state
uint8_t vbla_routine;

uint8_t wtr_state;

int16_t vid_scrpos_y_dup, vid_bg_scrpos_y_dup, vid_scrpos_x_dup, vid_bg_scrpos_x_dup, vid_bg3_scrpos_y_dup, vid_bg3_scrpos_x_dup;

VDP_Sprite sprite_buffer[BUFFER_SPRITES]; //Apparently the last 16 entries of this intrude other memory in the original
                                          //... now how would I emulate that?
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

void WaitForVBla()
{
	//Render the VDP
	VDP_Render();
}

void ClearScreen()
{
	//Clear foreground and background planes
	VDP_FillVRAM(VRAM_FG, 0x00, PLANE_SIZE);
	VDP_FillVRAM(VRAM_BG, 0x00, PLANE_SIZE);
	
	//Reset screen position duplicates
	vid_scrpos_y_dup = 0;
	vid_bg_scrpos_y_dup = 0;
	vid_scrpos_x_dup = 0;
	vid_bg_scrpos_x_dup = 0;
	
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
			VDP_Tile tile = TILE_TO_STRUCT(v);
			VDP_WriteVRAM(offset + (x << 1), (const uint8_t*)&tile, 2);
		}
		offset += PLANE_WIDTH * 2;
	}
}
