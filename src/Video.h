#pragma once

#include <Backend/VDP.h>

//Video globals
extern uint8_t vbla_routine;

extern int16_t scrposy_dup, bgscrposy_dup, scrposx_dup, bgscrposx_dup;

extern uint8_t wtr_state;

extern uint64_t sprite_buffer[0x50];
extern uint16_t hscroll_buffer[SCREEN_HEIGHT][2];

//Video interface
void VDPSetupGame();
void VDPSetupFrame();
int WaitForVBla();
void ClearScreen();
void CopyTilemap(const uint8_t *tilemap, size_t offset, size_t width, size_t height);
