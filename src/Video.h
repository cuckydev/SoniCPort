#pragma once

#include <Backend/VDP.h>

//Video globals
extern uint8_t vbla_routine;

extern uint8_t wtr_state;

extern VDP_Sprite sprite_buffer[0x50];
extern int16_t hscroll_buffer[SCREEN_HEIGHT][2];

//Video interface
void VDPSetupGame();
void VDPSetupFrame();
int WaitForVBla();
void ClearScreen();
void CopyTilemap(const uint8_t *tilemap, size_t offset, size_t width, size_t height);
