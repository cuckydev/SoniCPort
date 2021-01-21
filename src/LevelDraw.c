#include "LevelDraw.h"

#include <Constants.h>
#include "Level.h"
#include "LevelScroll.h"

#include <Backend/VDP.h>

//Scroll blocks
int16_t scroll_block_1_size;
int16_t scroll_block_2_size;
int16_t scroll_block_3_size;
int16_t scroll_block_4_size;

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
	
	//Get chunk
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
	if (chunk == 0xFF)
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

#define WRITE_TILE(off, xor) \
{ \
	uint16_t tile = ((*block++ << 8) | (*block++ << 0)) ^ xor; \
	VDP_WriteVRAM(offset + (off), (const uint8_t*)&tile, 2); \
}

void DrawBlock(const uint8_t *meta, const uint8_t *block, size_t offset)
{
	uint8_t flag = meta[0];
	
	if (flag & 0x08) //X flip
	{
		if (flag & 0x10) //Y flip
		{
			WRITE_TILE((PLANE_WIDTH * 2) + 2, 0x1800)
			WRITE_TILE((PLANE_WIDTH * 2) + 0, 0x1800)
			WRITE_TILE(                    2, 0x1800)
			WRITE_TILE(                    0, 0x1800)
		}
		else
		{
			WRITE_TILE(                    2, 0x0800)
			WRITE_TILE(                    0, 0x0800)
			WRITE_TILE((PLANE_WIDTH * 2) + 2, 0x0800)
			WRITE_TILE((PLANE_WIDTH * 2) + 0, 0x0800)
		}
	}
	else if (flag & 0x10) //Y flip
	{
		WRITE_TILE((PLANE_WIDTH * 2) + 0, 0x1000)
		WRITE_TILE((PLANE_WIDTH * 2) + 2, 0x1000)
		WRITE_TILE(                    0, 0x1000)
		WRITE_TILE(                    2, 0x1000)
	}
	else
	{
		WRITE_TILE(                    0, 0x0000)
		WRITE_TILE(                    2, 0x0000)
		WRITE_TILE((PLANE_WIDTH * 2) + 0, 0x0000)
		WRITE_TILE((PLANE_WIDTH * 2) + 2, 0x0000)
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
		pos = ((((pos >> 1) / PLANE_WIDTH) * PLANE_WIDTH) +
			((((pos >> 1) % PLANE_WIDTH) + 2) % PLANE_WIDTH)) << 1;
		x += 16;
	}
}

void DrawBlocks_LR(size_t offset, size_t pos, int16_t sx, int16_t sy, int16_t x, int16_t y, uint8_t *layout)
{
	DrawBlocks_LR_2(offset, pos, sx, sy, x, y, layout, (SCREEN_WIDTH + 16 + 16) / 16);
}

void DrawBlocks_TB_2(size_t offset, size_t pos, int16_t sx, int16_t sy, int16_t x, int16_t y, uint8_t *layout, size_t height)
{
	puts("DrawBlocks_TB_2");
}

void DrawBlocks_TB(size_t offset, size_t pos, int16_t sx, int16_t sy, int16_t x, int16_t y, uint8_t *layout)
{
	DrawBlocks_TB_2(offset, pos, sx, sy, x, y, layout, (SCREEN_HEIGHT + 16 + 16) / 16);
}

//Level drawing functions
void DrawChunks(int16_t sx, int16_t sy, uint8_t *layout, size_t offset)
{
	int16_t y = -16;
	for (size_t i = 0; i < (SCREEN_HEIGHT + 16 + 16) / 16; i++)
	{
		DrawBlocks_LR_2(offset, CalcVRAMPos(sx, sy, 0, y), sx, sy, 0, y, layout, 512 / 16);
		y += 16;
	}
}

void DrawBGScrollBlock1(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset)
{
	//Check if any flags have been set
	if (*flag == 0)
		return;
	
	//Handle flags
	if (*flag != (*flag &= ~SCROLL_FLAG_UP))
		DrawBlocks_LR_2(offset, CalcVRAMPos(sx, sy, -16, -16), sx, sy, -16, -16, layout, 512 / 16);
	if (*flag != (*flag &= ~SCROLL_FLAG_DOWN))
		DrawBlocks_LR_2(offset, CalcVRAMPos(sx, sy, -16, SCREEN_HEIGHT), sx, sy, -16, SCREEN_HEIGHT, layout, 512 / 16);
	if (*flag != (*flag &= ~SCROLL_FLAG_LEFT))
		DrawBlocks_TB(offset, CalcVRAMPos(sx, sy, -16, -16), sx, sy, -16, -16, layout);
	if (*flag != (*flag &= ~SCROLL_FLAG_RIGHT))
		DrawBlocks_TB(offset, CalcVRAMPos(sx, sy, SCREEN_WIDTH, -16), sx, sy, SCREEN_WIDTH, -16, layout);
	if (*flag != (*flag &= ~SCROLL_FLAG_UP2))
		;//DrawBlocks_LB_3(offset, CalcVRAMPos_2(sx, sy, 0, -16), sx, sy, 0, -16, layout);
	if (*flag != (*flag &= ~SCROLL_FLAG_DOWN2))
		;//DrawBlocks_LB_3(offset, CalcVRAMPos_2(sx, sy, 0, SCREEN_HEIGHT), sx, sy, 0, SCREEN_HEIGHT, layout);
}

void DrawBGScrollBlock2(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset)
{
}

void DrawBGScrollBlock3(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset)
{
	
}
