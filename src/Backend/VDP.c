#include "VDP.h"

#include "MegaDrive.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Render backend interface
int Render_Init(const MD_Header *header);
void Render_Quit();
void Render_Screen(const uint32_t *screen);

//Input backend interface
int Input_HandleEvents();

//VDP compile options
#define VDP_SANITY //Enable sanity checks for the VDP (slower, but technically safer, basically for testing)
//#define VDP_PALETTE_DISPLAY //Enable palette display

//VDP masks
#define VDP_MASK_PLANEPRI (1 << 0)
#define VDP_MASK_SPRITE   (1 << 1)

//VDP internal state
static ALIGNED2 uint8_t vdp_vram[VRAM_SIZE];
static uint16_t vdp_cram[4][16];

static size_t vdp_plane_a_location, vdp_plane_b_location, vdp_sprite_location, vdp_hscroll_location;
static size_t vdp_plane_w, vdp_plane_h;
static uint8_t vdp_background_colour;

static int16_t vdp_vscroll_a, vdp_vscroll_b;

static int16_t vdp_hint_pos;

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
	vdp_hint_pos = -1;
	
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
	loc &= ~0x3FF;
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
	loc &= ~0x1FFF;
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
	loc &= ~0x1FF;
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
	loc &= ~0x3FF;
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

void VDP_SetHIntPosition(int16_t pos)
{
	vdp_hint_pos = pos;
}

//VDP rendering
#define SCREEN_PITCH SCREEN_WIDTH + (VDP_INTERNAL_PAD * 2)

#define SCANLINE_SPRITES 40

static uint32_t vdp_screen_internal[SCREEN_HEIGHT][SCREEN_PITCH];
static uint8_t vdp_mask_internal[SCREEN_HEIGHT][SCREEN_PITCH];

static uint32_t *vdp_screen;
static uint8_t *vdp_mask;

static uint32_t vdp_screen_pal[4][16];

static struct VDP_SpriteCache
{
	const VDP_Sprite *sprite[SCANLINE_SPRITES];
	uint8_t pushind;
	uint16_t pixels;
} vdp_sprite_cache[SCREEN_HEIGHT];

uint32_t VDP_GetColour(size_t index)
{
	#ifdef VDP_SANITY
	if (index >= COLOURS)
	{
		puts("VDP_GetColour: Illegal colour index");
		return 0xFF0000FF;
	}
	#endif
	
	uint16_t cv = vdp_cram[index >> 4][index & 0xF];
	uint8_t r = (cv & 0x00E) >> 1;
	uint8_t g = (cv & 0x0E0) >> 5;
	uint8_t b = (cv & 0xE00) >> 9;
	
	static const uint8_t col_level[] = {0, 52, 87, 116, 144, 172, 206, 255};
	return (col_level[r] << 24) | (col_level[g] << 16) | (col_level[b] << 8) | 0xFF;
}

uint8_t *VDP_GetPatternAddress(size_t pattern)
{
	#ifdef VDP_SANITY
	if (pattern >= (VRAM_SIZE >> 5))
	{
		puts("VDP_GetPatternAddress: Out-of-bounds");
		return vdp_vram;
	}
	#endif
	
	return vdp_vram + (pattern << 5);
}

#define WRITE_NIBBLE(from, to, tom, pal, and, or, nibs) \
{                                             \
	uint8_t v;                                \
	if ((v = (*from >> nibs) & 0xF))          \
	{                                         \
		if (*tom & and)                       \
		{                                     \
			*tom |= or;                       \
			to++;                             \
		}                                     \
		else                                  \
		{                                     \
			*tom |= or;                       \
			*to++ = vdp_screen_pal[(pal)][v]; \
		}                                     \
		tom++;                                \
	}                                         \
	else                                      \
	{                                         \
		to++;                                 \
		tom++;                                \
	}                                         \
}

#define WRITE_BYTE(from, to, tom, pal, and, or)  \
{                                                \
	WRITE_NIBBLE(from, to, tom, pal, and, or, 4) \
	WRITE_NIBBLE(from, to, tom, pal, and, or, 0) \
	from++;                                      \
}

#define WRITE_BYTE_FLIP(from, to, tom, pal, and, or) \
{                                                \
	WRITE_NIBBLE(from, to, tom, pal, and, or, 0) \
	WRITE_NIBBLE(from, to, tom, pal, and, or, 4) \
	from--;                                      \
}

void VDP_DrawPlaneRow(uint32_t *to, uint8_t *tom, const VDP_Tile *plane, int16_t x, int16_t y)
{
	//Get plane tile to use
	size_t px = (x >> 3) % vdp_plane_w;
	size_t py = (y >> 3) % vdp_plane_h;
	const VDP_Tile *pb = plane + py * vdp_plane_w;
	
	//Draw plane row
	uint32_t *toend = to + SCREEN_WIDTH;
	to -= x & 7;
	tom -= x & 7;
	y &= 7;
	
	for (; to < toend; px = (px + 1) % vdp_plane_w, plane++)
	{
		//Get tile information
		const VDP_Tile *tile = pb + px;
		uint8_t or = tile->s.priority ? VDP_MASK_PLANEPRI : 0;
		uint8_t palette = tile->s.palette;
		uint8_t x_flip = tile->s.x_flip;
		uint8_t y_flip = tile->s.y_flip;
		uint16_t pattern = tile->s.pattern;
		
		//Write tile
		const uint8_t *from;
		if (y_flip)
			from = VDP_GetPatternAddress(pattern) + ((y ^ 7) << 2);
		else
			from = VDP_GetPatternAddress(pattern) + (y << 2);
		if (x_flip)
		{
			from += 3;
			WRITE_BYTE_FLIP(from, to, tom, palette, VDP_MASK_PLANEPRI, or)
			WRITE_BYTE_FLIP(from, to, tom, palette, VDP_MASK_PLANEPRI, or)
			WRITE_BYTE_FLIP(from, to, tom, palette, VDP_MASK_PLANEPRI, or)
			WRITE_BYTE_FLIP(from, to, tom, palette, VDP_MASK_PLANEPRI, or)
		}
		else
		{
			WRITE_BYTE(from, to, tom, palette, VDP_MASK_PLANEPRI, or)
			WRITE_BYTE(from, to, tom, palette, VDP_MASK_PLANEPRI, or)
			WRITE_BYTE(from, to, tom, palette, VDP_MASK_PLANEPRI, or)
			WRITE_BYTE(from, to, tom, palette, VDP_MASK_PLANEPRI, or)
		}
	}
}

void VDP_DrawSpriteRow(uint32_t *to, uint8_t *tom, const VDP_Sprite *sprite, int16_t y)
{
	//Get sprite information
	uint8_t and = sprite->tile.s.priority ? VDP_MASK_SPRITE : (VDP_MASK_PLANEPRI | VDP_MASK_SPRITE);
	uint8_t x_flip = sprite->tile.s.x_flip;
	uint8_t y_flip = sprite->tile.s.y_flip;
	uint8_t width = sprite->size.s.width;
	uint8_t height = sprite->size.s.height;
	uint16_t palette = sprite->tile.s.palette;
	uint16_t pattern = sprite->tile.s.pattern;
	
	//Get sprite left and right coordinates
	int16_t width_pixels = (width + 1) << 3;
	
	int16_t left = sprite->x - 128;
	if (left <= -width_pixels || left >= SCREEN_WIDTH)
		return;
	to += left;
	tom += left;
	
	int16_t right = left + width_pixels;
	
	//Get Y tile
	y -= (sprite->y - 128);
	size_t ty = y >> 3;
	if (y_flip)
	{
		ty = height - ty;
		y = (y & 7) ^ 7;
	}
	else
	{
		y &= 7;
	}
	pattern += ty;
	
	//Get X tile
	if (x_flip)
	{
		pattern += width * (height + 1);
		for (; left < right; left += 8)
		{
			//Write tile
			const uint8_t *from = VDP_GetPatternAddress(pattern) + (y << 2) + 3;
			WRITE_BYTE_FLIP(from, to, tom, palette, and, VDP_MASK_SPRITE)
			WRITE_BYTE_FLIP(from, to, tom, palette, and, VDP_MASK_SPRITE)
			WRITE_BYTE_FLIP(from, to, tom, palette, and, VDP_MASK_SPRITE)
			WRITE_BYTE_FLIP(from, to, tom, palette, and, VDP_MASK_SPRITE)
			pattern -= height + 1;
		}
	}
	else
	{
		for (; left < right; left += 8)
		{
			//Write tile
			const uint8_t *from = VDP_GetPatternAddress(pattern) + (y << 2);
			WRITE_BYTE(from, to, tom, palette, and, VDP_MASK_SPRITE)
			WRITE_BYTE(from, to, tom, palette, and, VDP_MASK_SPRITE)
			WRITE_BYTE(from, to, tom, palette, and, VDP_MASK_SPRITE)
			WRITE_BYTE(from, to, tom, palette, and, VDP_MASK_SPRITE)
			pattern += height + 1;
		}
	}
}

static inline void VDP_DrawScanline(size_t y, uint32_t *to, uint8_t *tom, struct VDP_SpriteCache *scache, const int16_t *hscroll)
{
	//Clear scanline
	for (size_t i = 0; i < SCREEN_WIDTH; i++)
		to[i] = vdp_screen_pal[0][vdp_background_colour];
	memset(tom, 0, SCREEN_WIDTH);
	
	//Draw planes
	VDP_DrawPlaneRow(to, tom, (const VDP_Tile*)(vdp_vram + vdp_plane_b_location), -hscroll[1], y + vdp_vscroll_b);
	VDP_DrawPlaneRow(to, tom, (const VDP_Tile*)(vdp_vram + vdp_plane_a_location), -hscroll[0], y + vdp_vscroll_a);
	
	//Draw sprites
	for (uint8_t i = 0; i < scache->pushind; i++)
		VDP_DrawSpriteRow(to, tom, scache->sprite[i], y);
	
	#ifdef VDP_PALETTE_DISPLAY
		for (size_t i = 0; i < 4 * 16; i++)
			to[i] = vdp_screen_pal[i >> 4][i & 0xF];
	#endif
}

static inline void VDP_RefreshPalette()
{
	uint32_t *pal_to = &vdp_screen_pal[0][0];
	for (size_t i = 0; i < 4 * 16; i++)
		*pal_to++ = VDP_GetColour(i);
}

void VDP_Render()
{
	//Get VDP screen pointer
	vdp_screen = &vdp_screen_internal[0][VDP_INTERNAL_PAD];
	vdp_mask = &vdp_mask_internal[0][VDP_INTERNAL_PAD];
	
	//Calculate sprite cache
	memset(vdp_sprite_cache, 0, sizeof(vdp_sprite_cache));
	
	for (uint8_t i = 0;;)
	{
		//Get sprite values
		const VDP_Sprite *sprite = (const VDP_Sprite*)(vdp_vram + vdp_sprite_location + (i << 3));
		uint16_t sprite_y = sprite->y;
		uint8_t sprite_width = sprite->size.s.width;
		uint8_t sprite_height = sprite->size.s.height;
		uint8_t sprite_link = sprite->link;
		
		//Get sprite bounding area
		int top = sprite_y - 128;
		int bottom = top + ((sprite_height + 1) << 3);
		if (top < 0)
			top = 0;
		if (bottom > SCREEN_HEIGHT)
			bottom = SCREEN_HEIGHT;
		
		//Write sprite cache
		for (int v = top; v < bottom; v++)
		{
			struct VDP_SpriteCache *scache = &vdp_sprite_cache[v];
			scache->pixels += sprite_width + 1;
			if (scache->pixels <= SCANLINE_SPRITES)
				scache->sprite[scache->pushind++] = sprite;
		}
		
		//Go to next sprite
		if (sprite_link != 0)
			i = sprite_link;
		else
			break;
	}
	
	//Render VDP screen
	VDP_RefreshPalette();
	
	uint32_t *to = vdp_screen;
	uint8_t *tom = vdp_mask;
	struct VDP_SpriteCache *scache = vdp_sprite_cache;
	const int16_t *hscroll = (int16_t*)(vdp_vram + vdp_hscroll_location);
	
	if (vdp_hint_pos >= 0 && vdp_hint_pos < SCREEN_HEIGHT)
	{
		//Draw up to horizontal interrupt
		size_t y = 0;
		while (y < (size_t)vdp_hint_pos && y < SCREEN_HEIGHT)
		{
			for (; y < (size_t)vdp_hint_pos && y < SCREEN_HEIGHT; y++, scache++, hscroll += 2, to += SCREEN_PITCH, tom += SCREEN_PITCH)
				VDP_DrawScanline(y, to, tom, scache, hscroll);
			
			//Send horizontal interrupt
			vdp_hint();
			VDP_RefreshPalette();
		}
		
		//Draw rest of screen
		for (; y < SCREEN_HEIGHT; y++, scache++, hscroll += 2, to += SCREEN_PITCH, tom += SCREEN_PITCH)
			VDP_DrawScanline(y, to, tom, scache, hscroll);
	}
	else
	{
		//Draw entire screen
		for (size_t y = 0; y < SCREEN_HEIGHT; y++, scache++, hscroll += 2, to += SCREEN_PITCH, tom += SCREEN_PITCH)
			VDP_DrawScanline(y, to, tom, scache, hscroll);
	}
	
	//Send vertical interrupt
	vdp_vint();
	
	//Render screen
	Render_Screen(vdp_screen);
	
	//Handle events
	if (Input_HandleEvents())
	{
		//Game should close
		MegaDrive_Quit();
		exit(0);
	}
}
