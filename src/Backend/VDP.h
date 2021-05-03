#pragma once

#include "MegaDrive.h"

#include "Constants.h"
#include "Macros.h"
#include <stdbool.h>

//VDP constants
#define VDP_INTERNAL_PAD 32

#define VRAM_SIZE    0x10000
#define PLANE_SIZE   0x2000
#define SPRITES      80
#define SPRITES_SIZE (SPRITES * 8)
#define COLOURS      (4 * 16)

//Tile structure
#define TILE_PRIORITY_AND   0x8000
#define TILE_PRIORITY_SHIFT 15
#define TILE_PALETTE_AND    0x6000
#define TILE_PALETTE_SHIFT  13
#define TILE_Y_FLIP_AND     0x1000
#define TILE_Y_FLIP_SHIFT   12
#define TILE_X_FLIP_AND     0x0800
#define TILE_X_FLIP_SHIFT   11
#define TILE_PATTERN_AND    0x07FF
#define TILE_PATTERN_SHIFT  0

#define TILE_MAP(priority, palette, y_flip, x_flip, pattern)      \
	(                                                             \
		((priority << TILE_PRIORITY_SHIFT) & TILE_PRIORITY_AND) | \
		((palette  << TILE_PALETTE_SHIFT)  & TILE_PALETTE_AND)  | \
		((y_flip   << TILE_Y_FLIP_SHIFT)   & TILE_Y_FLIP_AND)   | \
		((x_flip   << TILE_X_FLIP_SHIFT)   & TILE_X_FLIP_AND)   | \
		((pattern  << TILE_PATTERN_SHIFT)  & TILE_PATTERN_AND)    \
	)

//Sprite structure
//word y 000000YYYYYYYYYY
#define SPRITE_Y_AND   0x3FF
#define SPRITE_Y_SHIFT 0
//word sizelink 0000WWHH00LLLLLL
#define SPRITE_SL_W_AND   0x0C00
#define SPRITE_SL_W_SHIFT 10
#define SPRITE_SL_H_AND   0x0300
#define SPRITE_SL_H_SHIFT 8
#define SPRITE_SL_L_AND   0x003F
#define SPRITE_SL_L_SHIFT 0
//word tile
//word x
#define SPRITE_X_AND   0x1FF
#define SPRITE_X_SHIFT 0

//VDP interface
int VDP_Init(const MD_Header *header);
void VDP_Quit();

void VDP_SeekVRAM(size_t offset);
void VDP_WriteVRAM(const uint8_t *data, size_t len);
void VDP_FillVRAM(uint8_t data, size_t len);

void VDP_SeekCRAM(size_t offset);
void VDP_WriteCRAM(const uint16_t *data, size_t len);
void VDP_FillCRAM(uint16_t data, size_t len);

void VDP_SetPlaneALocation(size_t loc);
void VDP_SetPlaneBLocation(size_t loc);
void VDP_SetSpriteLocation(size_t loc);
void VDP_SetHScrollLocation(size_t loc);
void VDP_SetPlaneSize(size_t w, size_t h);
void VDP_SetBackgroundColour(uint8_t index);
void VDP_SetVScroll(int16_t scroll_a, int16_t scroll_b);
void VDP_SetHIntPosition(int16_t pos);

void VDP_Render();
