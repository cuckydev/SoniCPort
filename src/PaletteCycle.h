#pragma once

#include <stdint.h>

//Palette cycle state
extern int16_t pcyc_num, pcyc_time;
extern uint16_t pcyc_buffer[0x18];

//Palette cycle routines
int PCycle_Sega();
void PCycle_Title();
void PCycle_GHZ();
void PaletteCycle();
