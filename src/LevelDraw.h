#pragma once

#include <stdint.h>
#include <stddef.h>

//Level drawing globals
extern int16_t scroll_block1_size, scroll_block2_size, scroll_block3_size, scroll_block4_size;

//Level drawing functions
void DrawChunks(int16_t sx, int16_t sy, uint8_t *layout, size_t offset);
void LoadTilesFromStart();
void DrawBGScrollBlock1(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset);
void DrawBGScrollBlock2(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset);
void DrawBGScrollBlock3(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset);
void LoadTilesAsYouMove();
void LoadTilesAsYouMove_BGOnly();
void AnimateLevelGfx();
