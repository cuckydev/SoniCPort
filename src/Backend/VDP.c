#include "VDP.h"

#include <stdio.h>
#include <string.h>

//Render backend interface
int Render_Init(const MD_Header *header);
void Render_Quit();
void Render_Screen(const uint32_t *screen);

//Input backend interface
int Input_HandleEvents();

//VDP compile options
#define VDP_SANITY //Enable sanity checks for the VDP
//#define VDP_PALETTE_DISPLAY //Enable palette display

//VDP internal state
static uint8_t vdp_vram[VRAM_SIZE];
static uint16_t vdp_cram[4][16];

static size_t vdp_plane_a_location, vdp_plane_b_location, vdp_sprite_location, vdp_hscroll_location;
static size_t vdp_plane_w, vdp_plane_h;
static uint8_t vdp_background_colour;

static int16_t vdp_vscroll_a, vdp_vscroll_b;

static MD_Vector vdp_hint, vdp_vint;

//VDP interface
int VDP_Init(const MD_Header *header)
{
	//Initialize backend
	if (Render_Init(header))
		return -1;
	
	//Initialize VDP state
	vdp_plane_a_location = 0;
	vdp_plane_b_location = 0;
	vdp_sprite_location  = 0;
	vdp_hscroll_location = 0;
	vdp_plane_w = 32;
	vdp_plane_h = 32;
	vdp_background_colour = 0;
	vdp_vscroll_a = 0;
	vdp_vscroll_b = 0;
	
	vdp_hint = header->h_interrupt;
	vdp_vint = header->v_interrupt;
	
	return 0;
}

void VDP_Quit()
{
	//Quit backend
	Render_Quit();
}

void VDP_WriteVRAM(size_t offset, const uint8_t *data, size_t len)
{
	#ifdef VDP_SANITY
	if (offset > VRAM_SIZE || offset + len > VRAM_SIZE)
	{
		puts("VDP_WriteVRAM: Out-of-bounds");
		return;
	}
	#endif
	memcpy(vdp_vram + offset, data, len);
}

void VDP_FillVRAM(size_t offset, uint8_t data, size_t len)
{
	#ifdef VDP_SANITY
	if (offset > VRAM_SIZE || offset + len > VRAM_SIZE)
	{
		puts("VDP_FillVRAM: Out-of-bounds");
		return;
	}
	#endif
	memset(vdp_vram + offset, data, len);
}

void VDP_WriteCRAM(size_t offset, const uint16_t *data, size_t len)
{
	#ifdef VDP_SANITY
	if (offset > COLOURS || offset + len > COLOURS)
	{
		puts("VDP_WriteCRAM: Out-of-bounds");
		return;
	}
	#endif
	memcpy(&vdp_cram[0][0] + offset, data, len << 1);
}

void VDP_FillCRAM(size_t offset, uint16_t data, size_t len)
{
	#ifdef VDP_SANITY
	if (offset > COLOURS || offset + len > COLOURS)
	{
		puts("VDP_FillCRAM: Out-of-bounds");
		return;
	}
	#endif
	uint16_t *outp = &vdp_cram[0][0] + offset;
	while (len-- > 0)
		*outp++ = data;
}

void VDP_SetPlaneALocation(size_t loc)
{
	#ifdef VDP_SANITY
	if (loc > VRAM_SIZE - PLANE_SIZE)
	{
		puts("VDP_SetPlaneALocation: Out-of-bounds");
		return;
	}
	#endif
	vdp_plane_a_location = loc;
}

void VDP_SetPlaneBLocation(size_t loc)
{
	#ifdef VDP_SANITY
	if (loc > VRAM_SIZE - PLANE_SIZE)
	{
		puts("VDP_SetPlaneBLocation: Out-of-bounds");
		return;
	}
	#endif
	vdp_plane_b_location = loc;
}

void VDP_SetSpriteLocation(size_t loc)
{
	#ifdef VDP_SANITY
	if (loc > VRAM_SIZE - SPRITES_SIZE)
	{
		puts("VDP_SetSpriteLocation: Out-of-bounds");
		return;
	}
	#endif
	vdp_sprite_location = loc;
}

void VDP_SetHScrollLocation(size_t loc)
{
	#ifdef VDP_SANITY
	if (loc > VRAM_SIZE - SCREEN_HEIGHT * 4)
	{
		puts("VDP_SetHScrollLocation: Out-of-bounds");
		return;
	}
	#endif
	vdp_hscroll_location = loc;
}

void VDP_SetPlaneSize(size_t w, size_t h)
{
	#ifdef VDP_SANITY
	if (w * h > 0x1000)
	{
		puts("VDP_SetPlaneSize: Requested plane size exceeds 0x2000 bytes");
		return;
	}
	#endif
	vdp_plane_w = w;
	vdp_plane_h = h;
}

void VDP_SetBackgroundColour(uint8_t index)
{
	#ifdef VDP_SANITY
	if (index >= COLOURS)
	{
		puts("VDP_SetBackgroundColour: Illegal colour index");
		return;
	}
	#endif
	vdp_background_colour = index;
}

void VDP_SetVScroll(int16_t scroll_a, int16_t scroll_b)
{
	vdp_vscroll_a = scroll_a;
	vdp_vscroll_b = scroll_b;
}

//VDP rendering
#define SCREEN_PITCH SCREEN_WIDTH + 16

#define SCANLINE_SPRITES 20

static uint32_t vdp_screen_internal[SCREEN_HEIGHT + 16][SCREEN_PITCH];
static uint32_t *vdp_screen;

static uint32_t vdp_screen_pal[4][16];

static struct VDP_SpriteCache
{
	uint8_t sprite[SCANLINE_SPRITES];
	uint8_t pushind;
} vdp_sprite_cache[SCREEN_HEIGHT];

uint32_t VDP_GetColour(size_t index)
{
	uint16_t cv = *(&vdp_cram[0][0] + index);
	uint8_t r = (cv & 0x00E) >> 0;
	uint8_t g = (cv & 0x0E0) >> 4;
	uint8_t b = (cv & 0xE00) >> 8;
	
	static const uint8_t col_level[] = {0, 27, 49, 71, 87, 103, 119, 130, 146, 157, 174, 190, 206, 228, 255};
	return (col_level[r] << 24) | (col_level[g] << 16) | (col_level[b] << 8) | 0xFF;
}

uint8_t *VDP_GetPatternAddress(size_t pattern)
{
	return vdp_vram + (pattern << 5);
}

#define WRITE_BYTE(from, to, pal)         \
{                                         \
	uint8_t v;                            \
	if ((v = *from >> 4))                 \
		*to++ = vdp_screen_pal[(pal)][v]; \
	else                                  \
		to++;                             \
	if ((v = *from & 0xF))                \
		*to++ = vdp_screen_pal[(pal)][v]; \
	else                                  \
		to++;                             \
	from++;                               \
}

#define WRITE_BYTE_FLIP(from, to, pal)    \
{                                         \
	uint8_t v;                            \
	if ((v = *from & 0xF))                \
		*to++ = vdp_screen_pal[(pal)][v]; \
	else                                  \
		to++;                             \
	if ((v = *from >> 4))                 \
		*to++ = vdp_screen_pal[(pal)][v]; \
	else                                  \
		to++;                             \
	from--;                               \
}

void VDP_DrawPlaneColumn(uint32_t *to, const uint16_t *plane, uint16_t x, uint16_t y)
{
	//Get plane tile to use
	size_t px = (x >> 3) % vdp_plane_w;
	size_t py = (y >> 3) % vdp_plane_h;
	const uint16_t *pb = plane + py * vdp_plane_w;
	
	//Draw plane column
	uint32_t *toend = to + SCREEN_WIDTH;
	to -= x & 7;
	
	for (; to < toend; px = (px + 1) % vdp_plane_w, plane++)
	{
		//Get tile information
		uint16_t tile = pb[px];
		uint8_t palette = PLANE_R_PALETTE(tile);
		uint8_t x_flip = PLANE_R_XFLIP(tile);
		uint8_t y_flip = PLANE_R_YFLIP(tile);
		uint16_t pattern = PLANE_R_PATTERN(tile);
		
		//Write tile
		const uint8_t *from;
		if (y_flip)
			from = VDP_GetPatternAddress(pattern) + (((y ^ 7) & 7) << 2);
		else
			from = VDP_GetPatternAddress(pattern) + ((y & 7) << 2);
		if (x_flip)
		{
			from += 3;
			WRITE_BYTE_FLIP(from, to, palette)
			WRITE_BYTE_FLIP(from, to, palette)
			WRITE_BYTE_FLIP(from, to, palette)
			WRITE_BYTE_FLIP(from, to, palette)
		}
		else
		{
			WRITE_BYTE(from, to, palette)
			WRITE_BYTE(from, to, palette)
			WRITE_BYTE(from, to, palette)
			WRITE_BYTE(from, to, palette)
		}
	}
}

int VDP_Render()
{
	//Handle events
	int result;
	if ((result = Input_HandleEvents()))
		return result;
	
	//Get VDP screen pointer
	vdp_screen = &vdp_screen_internal[8][8];
	
	//Calculate sprite cache
	struct VDP_SpriteCache *scache = vdp_sprite_cache;
	const uint64_t *sprite = (const uint64_t*)(vdp_vram + vdp_sprite_location);
	
	for (size_t i = 0; i < SPRITES; i++, scache++, sprite++)
	{
		//Get sprite values
		int16_t sprite_y = SPRITE_R_Y(*sprite);
		uint8_t sprite_height = SPRITE_R_HEIGHT(*sprite);
		
		//Get sprite bounding area
		int16_t top = sprite_y - 128;
		if (top < 0)
			top = 0;
		int16_t bottom = sprite_y - 128 + ((sprite_height + 1) << 3);
		if (bottom > SCREEN_HEIGHT)
			bottom = SCREEN_HEIGHT;
		
		//Write sprite cache
		for (int16_t v = top; v < bottom; v++)
			if (scache->pushind < SCANLINE_SPRITES)
				scache->sprite[scache->pushind++] = i;
	}
	
	//Render VDP screen
	uint32_t *to = vdp_screen;
	
	const uint16_t *hscroll = (uint16_t*)(vdp_vram + vdp_hscroll_location);
	
	for (size_t y = 0; y < SCREEN_HEIGHT; y++)
	{
		//Get palette
		uint32_t *pal_to = &vdp_screen_pal[0][0];
		for (size_t i = 0; i < 4 * 16; i++)
			*pal_to++ = VDP_GetColour(i);
		
		//Clear screen
		for (size_t i = 0; i < SCREEN_WIDTH; i++)
			to[i] = vdp_screen_pal[0][vdp_background_colour];
		
		//Draw planes
		VDP_DrawPlaneColumn(to, (uint16_t*)(vdp_vram + vdp_plane_b_location), *hscroll++, vdp_vscroll_b + y);
		VDP_DrawPlaneColumn(to, (uint16_t*)(vdp_vram + vdp_plane_a_location), *hscroll++, vdp_vscroll_a + y);
		
		#ifdef VDP_PALETTE_DISPLAY
			for (size_t i = 0; i < 4*16; i++)
				to[i] = vdp_screen_pal[0][i];
		#endif
		
		//Render next row
		to += SCREEN_PITCH;
		
		//Send horizontal interrupt
		vdp_hint();
	}
	
	//Send vertical interrupt
	vdp_vint();
	
	//Render screen
	Render_Screen(vdp_screen);
	
	return 0;
}
