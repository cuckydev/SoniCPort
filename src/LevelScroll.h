#pragma once

#include <Types.h>

//Scroll flags
#define SCROLL_FLAG_UP     (1 << 0)
#define SCROLL_FLAG_DOWN   (1 << 1)
#define SCROLL_FLAG_LEFT   (1 << 2)
#define SCROLL_FLAG_RIGHT  (1 << 3)
#define SCROLL_FLAG_UP2    (1 << 4)
#define SCROLL_FLAG_DOWN2  (1 << 5)
#define SCROLL_FLAG_LEFT2  (1 << 0) //scroll blocks 2 and 3
#define SCROLL_FLAG_RIGHT2 (1 << 1) //scroll blocks 2 and 3

//Level deformation globals
extern uint8_t nobgscroll, bgscrollvert;

extern uint16_t fg_scroll_flags, bg1_scroll_flags, bg2_scroll_flags, bg3_scroll_flags;
extern uint16_t fg_scroll_flags_dup, bg1_scroll_flags_dup, bg2_scroll_flags_dup, bg3_scroll_flags_dup;

extern dword_s scrposx,     scrposy,     bgscrposx,     bgscrposy,     bg2scrposx,     bg2scrposy,     bg3scrposx,     bg3scrposy;
extern dword_s scrposx_dup, scrposy_dup, bgscrposx_dup, bgscrposy_dup, bg2scrposx_dup, bg2scrposy_dup, bg3scrposx_dup, bg3scrposy_dup;

extern int16_t scrshiftx, scrshifty;

extern uint8_t fg_xblock, bg1_xblock, bg2_xblock, bg3_xblock;
extern uint8_t fg_yblock, bg1_yblock, bg2_yblock, bg3_yblock;

extern int16_t lookshift;

//Level scroll functions
void BgScrollSpeed(int16_t x, int16_t y);
void DeformLayers();
void LoadTilesAsYouMove_BGOnly();
