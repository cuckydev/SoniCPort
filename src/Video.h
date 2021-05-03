#pragma once

#include "Backend/VDP.h"

//Video constants
#define BUFFER_SPRITES 0x50

//Video globals
extern uint8_t vbla_routine;

extern uint8_t sprite_count;

extern uint8_t hbla_pal;
extern int16_t hbla_pos;

extern int16_t vid_scrpos_y_dup, vid_bg_scrpos_y_dup, vid_scrpos_x_dup, vid_bg_scrpos_x_dup, vid_bg3_scrpos_y_dup, vid_bg3_scrpos_x_dup;

extern uint16_t sprite_buffer[BUFFER_SPRITES][4];
extern int16_t hscroll_buffer[SCREEN_HEIGHT][2];

//Video interface
void VDPSetupGame();
void WaitForVBla();
void ClearScreen();
void CopyTilemap(const uint8_t *tilemap, size_t offset, size_t width, size_t height);
