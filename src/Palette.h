#pragma once

#include <stdint.h>

//Palette types
typedef enum
{
	PalId_SegaBG,
	PalId_Title,
	PalId_LevelSel,
	PalId_Sonic,
	PalId_GHZ,
	PalId_LZ,
	PalId_MZ,
	PalId_SYZ,
	PalId_SLZ,
	PalId_SBZ1,
	PalId_Special,
	PalId_LZWater,
	PalId_SBZ3,
	PalId_SBZ3Water,
	PalId_SBZ2,
	PalId_SonicLZ,
	PalId_SonicSBZ,
	PalId_SSResults,
	PalId_Continue,
	PalId_Ending,
} PaletteId;

typedef struct
{
	uint8_t ind, len;
} PaletteFade;

//Palette globals
extern int16_t pal_chgspeed;

extern uint16_t dry_palette[4][16];
extern uint16_t dry_palette_dup[4][16];
extern uint16_t wet_palette[4][16];
extern uint16_t wet_palette_dup[4][16];

extern PaletteFade palette_fade;

//Palette interface
void PalLoad1(PaletteId id);
void PalLoad2(PaletteId id);
void PalLoad3_Water(PaletteId id);
void PalLoad4_Water(PaletteId id);

//Palette fading
void FadeIn_FromBlack();
void PaletteFadeIn();
void PaletteFadeIn_At(uint8_t ind, uint8_t len);
void FadeOut_ToBlack();
void PaletteFadeOut();
void PaletteFadeOut_At(uint8_t ind, uint8_t len);

void WhiteIn_FromWhite();
void PaletteWhiteIn();
void PaletteWhiteIn_At(uint8_t ind, uint8_t len);
void WhiteOut_ToWhite();
void PaletteWhiteOut();
void PaletteWhiteOut_At(uint8_t ind, uint8_t len);
