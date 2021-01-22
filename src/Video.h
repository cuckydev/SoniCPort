#pragma once

#include <Backend/VDP.h>

//Video constants
#define BUFFER_SPRITES 0x50

//Video globals
extern uint8_t vbla_routine;

extern uint8_t wtr_state;

extern int16_t vid_scrposy_dup, vid_bgscrposy_dup, vid_scrposx_dup, vid_bgscrposx_dup, vid_bg3scrposy_dup, vid_bg3scrposx_dup;

extern VDP_Sprite sprite_buffer[BUFFER_SPRITES];
extern int16_t hscroll_buffer[SCREEN_HEIGHT][2];

//Video interface
void VDPSetupGame();
int WaitForVBla();
void ClearScreen();
void CopyTilemap(const uint8_t *tilemap, size_t offset, size_t width, size_t height);
