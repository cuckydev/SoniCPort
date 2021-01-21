#pragma once

#include <Types.h>

//Scroll flags
#define SCROLL_FLAG_LEFT  (1 << 2)
#define SCROLL_FLAG_RIGHT (1 << 3)

//Level deformation globals
extern uint8_t nobgscroll, bgscrollvert;

extern uint16_t fg_scroll_flags, bg1_scroll_flags, bg2_scroll_flags, bg3_scroll_flags;

extern dword_s scrposx, scrposy, bgscrposx, bgscrposy, bg2scrposx, bg2scrposy, bg3scrposx, bg3scrposy;
extern int16_t scrposy_dup, bgscrposy_dup, scrposx_dup, bgscrposx_dup, bg3scrposy_dup, bg3scrposx_dup;

extern int16_t scrshiftx, scrshifty;

extern uint8_t fg_xblock, bg1_xblock, bg2_xblock, bg3_xblock;
extern uint8_t fg_yblock, bg1_yblock, bg2_yblock, bg3_yblock;

extern int16_t lookshift;

//Level scroll functions
void BgScrollSpeed(int16_t x, int16_t y);
void DeformLayers();
