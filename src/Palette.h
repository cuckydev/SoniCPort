#pragma once

#include <stdint.h>

//Palette globals
extern uint16_t dry_palette[4][16];
extern uint16_t dry_palette_dup[4][16];
extern uint16_t wet_palette[4][16];
extern uint16_t wet_palette_dup[4][16];

extern int16_t pcyc_num, pcyc_time;
extern uint16_t pcyc_buffer[0x18];

//Palette types
typedef enum
{
	PalId_SegaBG,
	PalId_Title,
	PalId_Sonic,
} PaletteId;

//Palette interface
void PalLoad1(PaletteId id);
void PalLoad2(PaletteId id);
void PalLoad3_Water(PaletteId id);
void PalLoad4_Water(PaletteId id);

//Palette fading
int PaletteFadeIn();
int PaletteFadeOut();
