#pragma once

#include <stdint.h>
#include <stddef.h>

//Level drawing globals
extern int16_t scroll_block_1_size;
extern int16_t scroll_block_2_size;
extern int16_t scroll_block_3_size;
extern int16_t scroll_block_4_size;

//Level drawing functions
void DrawChunks(int16_t sx, int16_t sy, uint8_t *layout, size_t offset);
void DrawBGScrollBlock1(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset);
void DrawBGScrollBlock2(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset);
void DrawBGScrollBlock3(int16_t sx, int16_t sy, uint16_t *flag, uint8_t *layout, size_t offset);
