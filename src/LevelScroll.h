#pragma once

#include <stdint.h>

//Scroll flags
#define SCROLL_FLAG_LEFT  (1 << 2)
#define SCROLL_FLAG_RIGHT (1 << 3)

//Level deformation globals
extern uint8_t nobgscroll, bgscrollvert;

extern uint16_t fg_scroll_flags, bg1_scroll_flags, bg2_scroll_flags, bg3_scroll_flags;

extern int32_t scrposx, scrposy, bgscrposy;
extern int32_t scrposx_dup, scrposy_dup, bgscrposy_dup;
extern int32_t bgscrposx, bg2scrposx, bg3scrposx;
extern int32_t bgscrposx_dup, bg2scrposx_dup, bg3scrposx_dup;
extern int32_t bgscrposy_dup, bg2scrposy_dup, bg3scrposy_dup;

extern int16_t scrshiftx, scrshifty;

extern uint8_t fg_xblock, bg1_xblock, bg2_xblock, bg3_xblock;
extern uint8_t fg_yblock, bg1_yblock, bg2_yblock, bg3_yblock;

//Level scroll functions
void DeformLayers();
