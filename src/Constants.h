#pragma once

//Screen dimensions
#define SCREEN_SCALE 2 //TODO: make screen scale a variable

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 224

#define SCREEN_ADD  (SCREEN_WIDTH - 2)
#define SCREEN_ADD2 (SCREEN_ADD / 2)

#define PLANE_WIDEADD (((SCREEN_WIDTH - 320) / 8 + 1) & ~1)

//VRAM data
#define VRAM_FG      0xC000 //foreground namespace
#define VRAM_BG      0xE000 //background namespace
#define VRAM_SONIC   0xF000 //Sonic graphics
#define VRAM_SPRITES 0xF800 //sprite table
#define VRAM_HSCROLL 0xFC00 //horizontal scroll table

#define PLANE_WIDTH  64
#define PLANE_HEIGHT 32
