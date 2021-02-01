#include "LevelDraw.h"

#include <Constants.h>
#include "Level.h"
#include "LevelScroll.h"

#include <Backend/VDP.h>

//Scroll dimensions (hack so that dimensions that aren't a multiple of 16 work)
#define SCROLL_WIDTH  ((SCREEN_WIDTH  + 15) & ~15)
#define SCROLL_HEIGHT ((SCREEN_HEIGHT + 15) & ~15)

//Scroll blocks
int16_t scroll_block1_size, scroll_block2_size, scroll_block3_size, scroll_block4_size;

//Block drawing functions
size_t CalcVRAMPos(int16_t sx, int16_t sy, int16_t x, int16_t y)
{
	//Convert coordinates to plane coordinates
	uint16_t px = ((x + sx) >> 2) & ~3;
	uint16_t py = ((y + sy) >> 2) & ~3;
	return (POSITIVE_MOD(py, PLANE_HEIGHT << 1) * PLANE_WIDTH) + POSITIVE_MOD(px, PLANE_WIDTH << 1);
}

void GetBlockData(const uint8_t **meta, const uint8_t **block, int16_t sx, int16_t sy, int16_t x, int16_t y, uint8_t *layout)
{
	//Offset coordinates by screen coordinates
	x += sx;
	y += sy;
	
	//Get chunk position
	int16_t cx = x >> 8;
	int16_t cy = y >> 8;
	
	if (cx < 0 || cy < 0 || cx >= 0x40 || cy >= 8)
	{
		*meta = level_map256;
		*block = level_map16;
		return;
	}
	
	//Get chunk
	uint8_t chunk = layout[(cy << 7) + cx] & 0x7F;
	if (chunk == 0)
	{
		*meta = level_map256;
		*block = level_map16;
		return;
	}
	
	//Get 256x256 map pointer
	uint8_t tx = (x >> 4) & 0xF;
	uint8_t ty = (y >> 4) & 0xF;
	const uint8_t *metap = (level_map256 - 0x200) + (chunk << 9) + (ty << 5) + (tx << 1);
	*meta = metap;
	
	//Get 16x16 map pointer
	size_t tile = (metap[0] << 8) | (metap[1] << 0);
	tile = tile & 0x3FF;
	
	*block = level_map16 + (tile << 3);
}

#define WRITE_TILE(off, xor)                                 \
{                                                            \
	uint16_t v = ((*block++ << 8) | (*block++ << 0)) ^ xor;  \
	VDP_Tile tile = TILE_TO_STRUCT(v);                       \
	VDP_WriteVRAM(offset + (off), (const uint8_t*)&tile, 2); \
}

void DrawBlock(const uint8_t *meta, const uint8_t *block, size_t offset)
{
	uint8_t flag = meta[0];
	
	if (flag & 0x08) //X flip
	{
		if (flag & 0x10) //Y flip
		{
			WRITE_TILE((PLANE_WIDTH << 1) + 2, 0x1800)
			WRITE_TILE((PLANE_WIDTH << 1) + 0, 0x1800)
			WRITE_TILE(                     2, 0x1800)
			WRITE_TILE(                     0, 0x1800)
		}
		else
		{
			WRITE_TILE(                     2, 0x0800)
			WRITE_TILE(                     0, 0x0800)
			WRITE_TILE((PLANE_WIDTH << 1) + 2, 0x0800)
			WRITE_TILE((PLANE_WIDTH << 1) + 0, 0x0800)
		}
	}
	else if (flag & 0x10) //Y flip
	{
		WRITE_TILE((PLANE_WIDTH << 1) + 0, 0x1000)
		WRITE_TILE((PLANE_WIDTH << 1) + 2, 0x1000)
		WRITE_TILE(                     0, 0x1000)
		WRITE_TILE(                     2, 0x1000)
	}
	else
	{
		WRITE_TILE(                     0, 0x0000)
		WRITE_TILE(                     2, 0x0000)
		WRITE_TILE((PLANE_WIDTH << 1) + 0, 0x0000)
		WRITE_TILE((PLANE_WIDTH << 1) + 2, 0x0000)
	}
}

void DrawBlocks_LR_2(size_t offset, size_t pos, int16_t sx, int16_t sy, int16_t x, int16_t y, uint8_t *layout, size_t width)
{
	const uint8_t *meta;
	const uint8_t *block;
	while (width-- > 0)
	{
		GetBlockData(&meta, &block, sx, sy, x, y, layout);
		DrawBlock(meta, block, offset + pos);
		size_t tx = pos % (PLANE_WIDTH << 1);
		size_t ty = pos / (PLANE_WIDTH << 1);
		pos = (ty * (PLANE_WIDTH << 1)) + ((tx + 4) % (PLANE_WIDTH << 1));
		x += 16;
	}
}

void DrawBlocks_LR(size_t offset, size_t pos, int16_t sx, int16_t sy, int16_t x, int16_t y, uint8_t *layout)
{
	DrawBlocks_LR_2(offset, pos, sx, sy, x, y, layout, (SCROLL_WIDTH + 16 + 16) / 16);
}

void DrawBlocks_TB_2(size_t offset, size_t pos, int16_t sx, int16_t sy, int16_t x, int16_t y, uint8_t *layout, size_t height)
{
	const uint8_t *meta;
	const uint8_t *block;
	while (height-- > 0)
	{
		GetBlockData(&meta, &block, sx, sy, x, y, layout);
		DrawBlock(meta, block, offset + pos);
		size_t tx = pos % (PLANE_WIDTH << 1);
		size_t ty = pos / (PLANE_WIDTH << 1);
		pos = (((ty + 2) % PLANE_HEIGHT) * (PLANE_WIDTH << 1)) + tx;
		y += 16;
	}
}

void DrawBlocks_TB(size_t offset, size_t pos, int16_t sx, int16_t sy, int16_t x, int16_t y, uint8_t *layout)
{
	DrawBlocks_TB_2(offset, pos, sx, sy, x, y, layout, (SCROLL_HEIGHT + 16 + 16) / 16);
}

void DrawBlocks_BG(size_t offset, int16_t sx, int16_t sy, int16_t y, uint8_t *layout, const uint8_t *array)
{
	static const dword_s *bg_pos[] = {&bg_scrpos_x, &bg_scrpos_x, &bg2_scrpos_x, &bg3_scrpos_y};
	uint8_t bg_pos_i = array[y >> 4];
	if (bg_pos_i != 0)
	{
		sx = bg_pos[bg_pos_i >> 1]->f.u;
		sy = (sy & ~0xF) % SCROLL_HEIGHT;
		DrawBlocks_LR(offset, CalcVRAMPos(sx, sy, 0, y), sx, sy, 0, y, layout);
	}
	else
	{
		DrawBlocks_LR_2(offset, CalcVRAMPos(sx, sy, 0, y), sx, sy, 0, y, layout, PLANE_WIDTH);
	}
}

void Draw_GHZ_Bg(int16_t sy, uint8_t *layout, size_t offset)
{
	int16_t y = 0;
	for (size_t i = 0; i < (SCROLL_HEIGHT + 16 + 16) / 16; i++)
	{
		static const uint8_t bg_array[] = {0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x06, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		DrawBlocks_BG(offset, bg_scrpos_y.f.u, sy, y, layout, bg_array);
		y += 16;
	}
}

//Level drawing functions
void DrawChunks(int16_t sx, int16_t sy, uint8_t *layout, size_t offset)
{
	int16_t y = -16;
	for (size_t i = 0; i < (SCROLL_HEIGHT + 16 + 16) / 16; i++)
	{
		DrawBlocks_LR_2(offset, CalcVRAMPos(sx, sy, 0, y), sx, sy, 0, y, layout, 512 / 16);
		y += 16;
	}
}

void LoadTilesFromStart()
{
	DrawChunks(scrpos_x.f.u, scrpos_y.f.u, level_layout[0][0], VRAM_FG);
	#ifndef SCP_REV00
		if (LEVEL_ZONE(level_id) == ZoneId_GHZ)
			Draw_GHZ_Bg(bg_scrpos_y.f.u, level_layout[0][1], VRAM_BG);
		else if (LEVEL_ZONE(level_id) == ZoneId_MZ)
			{;}//Draw_MZ_Bg(bg_scrpos_y.f.u, level_layout[0][1], VRAM_BG);
		else if (level_id == LEVEL_ID(ZoneId_SBZ, 0))
			{;}//Draw_SBZ_Bg(bg_scrpos_y.f.u, level_layout[0][1], VRAM_BG);
		else if (LEVEL_ZONE(level_id) == ZoneId_EndZ)
			Draw_GHZ_Bg(bg_scrpos_y.f.u, level_layout[0][1], VRAM_BG);
		else
	#endif
	DrawChunks(bg_scrpos_x.f.u, bg_scrpos_y.f.u, level_layout[0][1], VRAM_BG);
}

void DrawBGScrollBlock1(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset)
{
	//TODO: REV00
	//Check if any flags have been set
	if (*flag == 0)
		return;
	
	//Handle flags
	if (*flag & SCROLL_FLAG_UP)
	{
		DrawBlocks_LR_2(offset, CalcVRAMPos(sx, sy, -16, -16), sx, sy, -16, -16, layout, 512 / 16);
		*flag &= ~SCROLL_FLAG_UP;
	}
	if (*flag & SCROLL_FLAG_DOWN)
	{
		DrawBlocks_LR_2(offset, CalcVRAMPos(sx, sy, -16, SCROLL_HEIGHT), sx, sy, -16, SCROLL_HEIGHT, layout, 512 / 16);
		*flag &= ~SCROLL_FLAG_DOWN;
	}
	if (*flag & SCROLL_FLAG_LEFT)
	{
		DrawBlocks_TB(offset, CalcVRAMPos(sx, sy, -16, -16), sx, sy, -16, -16, layout);
		*flag &= ~SCROLL_FLAG_LEFT;
	}
	if (*flag & SCROLL_FLAG_RIGHT)
	{
		DrawBlocks_TB(offset, CalcVRAMPos(sx, sy, SCROLL_WIDTH, -16), sx, sy, SCROLL_WIDTH, -16, layout);
		*flag &= ~SCROLL_FLAG_RIGHT;
	}
	if (*flag & SCROLL_FLAG_UP2)
	{
		DrawBlocks_LR_2(offset, CalcVRAMPos(0, sy, 0, -16), 0, sy, 0, -16, layout, 512 / 16);
		*flag &= ~SCROLL_FLAG_UP2;
	}
	if (*flag & SCROLL_FLAG_DOWN2)
	{
		DrawBlocks_LR_2(offset, CalcVRAMPos(0, sy, 0, SCROLL_HEIGHT), 0, sy, 0, SCROLL_HEIGHT, layout, 512 / 16);
		*flag &= ~SCROLL_FLAG_DOWN2;
	}
}

void DrawBGScrollBlock2(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset)
{
	//TODO: REV00
	//Check if any flags have been set
	if (*flag == 0)
		return;
	
	//Run completely different code if in Scrap Brain Zone (what)
	if (LEVEL_ZONE(level_id) != ZoneId_SBZ)
	{
		if (*flag & SCROLL_FLAG_LEFT2)
		{
			DrawBlocks_TB_2(offset, CalcVRAMPos(sx, sy, -16, SCROLL_HEIGHT / 2), sx, sy, -16, SCROLL_HEIGHT / 2, layout, 3);
			*flag &= ~SCROLL_FLAG_LEFT2;
		}
		if (*flag & SCROLL_FLAG_RIGHT2)
		{
			DrawBlocks_TB_2(offset, CalcVRAMPos(sx, sy, SCROLL_WIDTH, SCROLL_HEIGHT / 2), sx, sy, SCROLL_WIDTH, SCROLL_HEIGHT / 2, layout, 3);
			*flag &= ~SCROLL_FLAG_RIGHT2;
		}
	}
	else
	{
		//TODO
	}
}

void DrawBGScrollBlock3(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset)
{
	//TODO: REV00
	//Check if any flags have been set
	if (*flag == 0)
		return;
	
	//Run completely different code if in Marble Zone (what)
	if (LEVEL_ZONE(level_id) != ZoneId_MZ)
	{
		if (*flag & SCROLL_FLAG_LEFT2)
		{
			DrawBlocks_TB_2(offset, CalcVRAMPos(sx, sy, -16, 64), sx, sy, -16, 64, layout, 3);
			*flag &= ~SCROLL_FLAG_LEFT2;
		}
		if (*flag & SCROLL_FLAG_RIGHT2)
		{
			DrawBlocks_TB_2(offset, CalcVRAMPos(sx, sy, SCROLL_WIDTH, 64), sx, sy, SCROLL_WIDTH, 64, layout, 3);
			*flag &= ~SCROLL_FLAG_RIGHT2;
		}
	}
	else
	{
		//TODO
	}
}

void LoadTilesAsYouMove()
{
	//Scroll background
	DrawBGScrollBlock1(bg_scrpos_x.f.u,  bg_scrpos_y.f.u,  &bg1_scroll_flags, level_layout[0][1], VRAM_BG);
	DrawBGScrollBlock2(bg2_scrpos_x.f.u, bg2_scrpos_y.f.u, &bg2_scroll_flags, level_layout[0][1], VRAM_BG);
	DrawBGScrollBlock3(bg3_scrpos_x.f.u, bg3_scrpos_y.f.u, &bg3_scroll_flags, level_layout[0][1], VRAM_BG);
	
	//Scroll foreground
	int16_t sx = scrpos_x_dup.f.u;
	int16_t sy = scrpos_y_dup.f.u;
	uint8_t *layout = level_layout[0][0];
	
	if (fg_scroll_flags == 0)
		return;
	
	if (fg_scroll_flags & SCROLL_FLAG_UP)
	{
		DrawBlocks_LR(VRAM_FG, CalcVRAMPos(sx, sy, -16, -16), sx, sy, -16, -16, layout);
		fg_scroll_flags &= ~SCROLL_FLAG_UP;
	}
	if (fg_scroll_flags & SCROLL_FLAG_DOWN)
	{
		DrawBlocks_LR(VRAM_FG, CalcVRAMPos(sx, sy, -16, SCROLL_HEIGHT), sx, sy, -16, SCROLL_HEIGHT, layout);
		fg_scroll_flags &= ~SCROLL_FLAG_DOWN;
	}
	if (fg_scroll_flags & SCROLL_FLAG_LEFT)
	{
		DrawBlocks_TB(VRAM_FG, CalcVRAMPos(sx, sy, -16, -16), sx, sy, -16, -16, layout);
		fg_scroll_flags &= ~SCROLL_FLAG_LEFT;
	}
	if (fg_scroll_flags & SCROLL_FLAG_RIGHT)
	{
		DrawBlocks_TB(VRAM_FG, CalcVRAMPos(sx, sy, SCROLL_WIDTH, -16), sx, sy, SCROLL_WIDTH, -16, layout);
		fg_scroll_flags &= ~SCROLL_FLAG_RIGHT;
	}
}

void LoadTilesAsYouMove_BGOnly()
{
	DrawBGScrollBlock1(bg_scrpos_x.f.u,  bg_scrpos_y.f.u,  &bg1_scroll_flags, level_layout[0][1], VRAM_BG);
	DrawBGScrollBlock2(bg2_scrpos_x.f.u, bg2_scrpos_y.f.u, &bg2_scroll_flags, level_layout[0][1], VRAM_BG);
	//No scroll block 3, even in REV01... odd
}
