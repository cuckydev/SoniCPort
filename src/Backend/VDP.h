#pragma once

#include "MegaDrive.h"

#include <Constants.h>
#include <Util/Macros.h>

//VDP constants
#define VRAM_SIZE    0x10000
#define PLANE_SIZE   0x2000
#define SPRITES      80
#define SPRITES_SIZE (SPRITES * 8)
#define COLOURS      (4 * 16)

//VDP macros
//Sprite structure
#define SPRITE_R_Y(s)        ((uint16_t)((s) >> 48))
#define SPRITE_R_WIDTH(s)    ((uint8_t)(((s) >> 42) & 0x3))
#define SPRITE_R_HEIGHT(s)   ((uint8_t)(((s) >> 40) & 0x3))
#define SPRITE_R_LINK(s)     ((uint8_t)(((s) >> 32) & 0x7F))
#define SPRITE_R_PRIORITY(s) ((uint8_t)(((s) >> 31) & 0x1))
#define SPRITE_R_PALETTE(s)  ((uint8_t)(((s) >> 29) & 0x3))
#define SPRITE_R_YFLIP(s)    ((uint8_t)(((s) >> 28) & 0x1))
#define SPRITE_R_XFLIP(s)    ((uint8_t)(((s) >> 28) & 0x1))
#define SPRITE_R_PATTERN(s)  ((uint16_t)(((s) >> 16) & 0x7FF))
#define SPRITE_R_X(s)        ((uint16_t)(s))

//Plane structure
#define PLANE_R_PRIORITY(p)  ((uint8_t)(((p) >> 15) & 0x1))
#define PLANE_R_PALETTE(p)   ((uint8_t)(((p) >> 13) & 0x3))
#define PLANE_R_YFLIP(p)     ((uint8_t)(((p) >> 12) & 0x1))
#define PLANE_R_XFLIP(p)     ((uint8_t)(((p) >> 11) & 0x1))
#define PLANE_R_PATTERN(p)   ((uint16_t)((p) & 0x7FF))

//VDP interface
int VDP_Init(const MD_Header *header);
void VDP_Quit();

void VDP_WriteVRAM(size_t offset, const uint8_t *data, size_t len);
void VDP_FillVRAM(size_t offset, uint8_t data, size_t len);

void VDP_WriteCRAM(size_t offset, const uint16_t *data, size_t len);
void VDP_FillCRAM(size_t offset, uint16_t data, size_t len);

void VDP_SetPlaneALocation(size_t loc);
void VDP_SetPlaneBLocation(size_t loc);
void VDP_SetSpriteLocation(size_t loc);
void VDP_SetHScrollLocation(size_t loc);
void VDP_SetPlaneSize(size_t w, size_t h);
void VDP_SetBackgroundColour(uint8_t index);
void VDP_SetVScroll(int16_t scroll_a, int16_t scroll_b);

int VDP_Render();
