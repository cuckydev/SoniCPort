#pragma once

#include <stdint.h>

//Level deformation globals
extern uint8_t nobgscroll;

extern uint16_t fg_scroll_flags, bg1_scroll_flags, bg2_scroll_flags, bg3_scroll_flags;

extern int16_t scrposy, bgscrposy, scrposx, bgscrposx, bg2scrposx, bg3scrposx;
extern int16_t scrposy_dup, bgscrposy_dup, scrposx_dup, bgscrposx_dup, bg2scrposy, bg3scrposy;

//Level scroll functions
void DeformLayers();
