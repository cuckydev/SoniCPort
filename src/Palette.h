#pragma once

#include <stdint.h>

//Palette globals
extern uint16_t pal_chgspeed;

extern uint16_t dry_palette[4][16];
extern uint16_t dry_palette_dup[4][16];
extern uint16_t wet_palette[4][16];
extern uint16_t wet_palette_dup[4][16];

//Palette types
typedef enum
{
	PalId_SegaBG,
	PalId_Title,
	PalId_Sonic,
	PalId_GHZ,
	PalId_SonicLZ,
	PalId_SonicSBZ,
} PaletteId;

//Palette interface
void PalLoad1(PaletteId id);
void PalLoad2(PaletteId id);
void PalLoad3_Water(PaletteId id);
void PalLoad4_Water(PaletteId id);

//Palette fading
void PaletteFadeIn();
void PaletteFadeOut();
