#include "Palette.h"

#include "Video.h"
#include "PLC.h"

#include "Backend/VDP.h"

#include <stdlib.h>

//Palette state
int16_t pal_chgspeed;

uint16_t dry_palette[4][16];
uint16_t dry_palette_dup[4][16];
uint16_t wet_palette[4][16];
uint16_t wet_palette_dup[4][16];

PaletteFade palette_fade;

//Palettes
static ALIGNED2 const uint8_t pal_sega_bg[] = {
	#include "Resource/Palette/SegaBG.h"
};
static ALIGNED2 const uint8_t pal_title[] = {
	#include "Resource/Palette/Title.h"
};
static ALIGNED2 const uint8_t pal_level_sel[] = {
	#include "Resource/Palette/LevelSel.h"
};
static ALIGNED2 const uint8_t pal_sonic[] = {
	#include "Resource/Palette/Sonic.h"
};
static ALIGNED2 const uint8_t pal_ghz[] = {
	#include "Resource/Palette/GHZ.h"
};
static ALIGNED2 const uint8_t pal_lz[] = {
	#include "Resource/Palette/LZ.h"
};
static ALIGNED2 const uint8_t pal_mz[] = {
	#include "Resource/Palette/MZ.h"
};
static ALIGNED2 const uint8_t pal_slz[] = {
	#include "Resource/Palette/SLZ.h"
};
static ALIGNED2 const uint8_t pal_syz[] = {
	#include "Resource/Palette/SYZ.h"
};
static ALIGNED2 const uint8_t pal_sbz1[] = {
	#include "Resource/Palette/SBZ1.h"
};
static ALIGNED2 const uint8_t pal_special[] = {
	#include "Resource/Palette/Special.h"
};
static ALIGNED2 const uint8_t pal_lz_water[] = {
	#include "Resource/Palette/LZWater.h"
};
static ALIGNED2 const uint8_t pal_sbz3[] = {
	#include "Resource/Palette/SBZ3.h"
};
static ALIGNED2 const uint8_t pal_sbz3_water[] = {
	#include "Resource/Palette/SBZ3Water.h"
};
static ALIGNED2 const uint8_t pal_sbz2[] = {
	#include "Resource/Palette/SBZ2.h"
};
static ALIGNED2 const uint8_t pal_sonic_lz[] = {
	#include "Resource/Palette/SonicLZ.h"
};
static ALIGNED2 const uint8_t pal_sonic_sbz[] = {
	#include "Resource/Palette/SonicSBZ.h"
};
static ALIGNED2 const uint8_t pal_ss_results[] = {
	#include "Resource/Palette/SSResults.h"
};
static ALIGNED2 const uint8_t pal_continue[] = {
	#include "Resource/Palette/Continue.h"
};
static ALIGNED2 const uint8_t pal_ending[] = {
	#include "Resource/Palette/Ending.h"
};

static struct PalettePointer
{
	const uint16_t *palette;
	uint16_t *target;
	size_t colours;
} palette_pointers[] = {
	/* PalId_SegaBG    */ {(const uint16_t*)pal_sega_bg,    &dry_palette[0][0], 0x40},
	/* PalId_Title     */ {(const uint16_t*)pal_title,      &dry_palette[0][0], 0x40},
	/* PalId_LevelSel  */ {(const uint16_t*)pal_level_sel,  &dry_palette[0][0], 0x40},
	/* PalId_Sonic     */ {(const uint16_t*)pal_sonic,      &dry_palette[0][0], 0x10},
	/* PalId_GHZ       */ {(const uint16_t*)pal_ghz,        &dry_palette[1][0], 0x30},
	/* PalId_LZ        */ {(const uint16_t*)pal_lz,         &dry_palette[1][0], 0x30},
	/* PalId_MZ        */ {(const uint16_t*)pal_mz,         &dry_palette[1][0], 0x30},
	/* PalId_SYZ       */ {(const uint16_t*)pal_syz,        &dry_palette[1][0], 0x30},
	/* PalId_SLZ       */ {(const uint16_t*)pal_slz,        &dry_palette[1][0], 0x30},
	/* PalId_SBZ1      */ {(const uint16_t*)pal_sbz1,       &dry_palette[1][0], 0x30},
	/* PalId_Special   */ {(const uint16_t*)pal_special,    &dry_palette[0][0], 0x40},
	/* PalId_LZWater   */ {(const uint16_t*)pal_lz_water,   &dry_palette[0][0], 0x40},
	/* PalId_SBZ3      */ {(const uint16_t*)pal_sbz3,       &dry_palette[1][0], 0x30},
	/* PalId_SBZ3Water */ {(const uint16_t*)pal_sbz3_water, &dry_palette[0][0], 0x40},
	/* PalId_SBZ2      */ {(const uint16_t*)pal_sbz2,       &dry_palette[1][0], 0x30},
	/* PalId_SonicLZ   */ {(const uint16_t*)pal_sonic_lz,   &dry_palette[0][0], 0x10},
	/* PalId_SonicSBZ  */ {(const uint16_t*)pal_sonic_sbz,  &dry_palette[0][0], 0x10},
	/* PalId_SSResults */ {(const uint16_t*)pal_ss_results, &dry_palette[0][0], 0x40},
	/* PalId_Continue  */ {(const uint16_t*)pal_continue,   &dry_palette[0][0], 0x20},
	/* PalId_Ending    */ {(const uint16_t*)pal_ending,     &dry_palette[0][0], 0x40},
};

//Palette interface
void PalLoad1(PaletteId id)
{
	//Load given palette
	struct PalettePointer *palload = &palette_pointers[id];
	const uint16_t *inp = palload->palette;
	uint16_t *outp = &dry_palette_dup[0][0] + (palload->target - &dry_palette[0][0]);
	
	for (size_t i = 0; i < palload->colours; i++, inp++)
		*outp++ = LESWAP_16(*inp);
}

void PalLoad2(PaletteId id)
{
	//Load given palette
	struct PalettePointer *palload = &palette_pointers[id];
	const uint16_t *inp = palload->palette;
	uint16_t *outp = palload->target;
	
	for (size_t i = 0; i < palload->colours; i++, inp++)
		*outp++ = LESWAP_16(*inp);
}

void PalLoad3_Water(PaletteId id)
{
	//Load given palette
	struct PalettePointer *palload = &palette_pointers[id];
	const uint16_t *inp = palload->palette;
	uint16_t *outp = &wet_palette[0][0] + (palload->target - &dry_palette[0][0]);
	
	for (size_t i = 0; i < palload->colours; i++, inp++)
		*outp++ = LESWAP_16(*inp);
}

void PalLoad4_Water(PaletteId id)
{
	//Load given palette
	struct PalettePointer *palload = &palette_pointers[id];
	const uint16_t *inp = palload->palette;
	uint16_t *outp = &wet_palette_dup[0][0] + (palload->target - &dry_palette[0][0]);
	
	for (size_t i = 0; i < palload->colours; i++, inp++)
		*outp++ = LESWAP_16(*inp);
}

//Fade in from black
void FadeIn_AddColour(uint16_t *col, uint16_t ref)
{
	uint16_t v = *col;
	if (v == ref)
		return;
	if ((v + 0x200) <= ref)
		v += 0x200;
	else if ((v + 0x020) <= ref)
		v += 0x020;
	else if ((v + 0x002) <= ref)
		v += 0x002;
	*col = v;
}

void FadeIn_FromBlack()
{
	uint16_t *col, *ref;
	
	//Fade dry palette
	col = (&dry_palette[0][0]) + palette_fade.ind;
	ref = (&dry_palette_dup[0][0]) + palette_fade.ind;
	for (int i = 0; i < palette_fade.len; i++)
		FadeIn_AddColour(col++, *ref++);
	
	//Fade wet palette
	col = (&wet_palette[0][0]) + palette_fade.ind;
	ref = (&wet_palette_dup[0][0]) + palette_fade.ind;
	for (int i = 0; i < palette_fade.len; i++)
		FadeIn_AddColour(col++, *ref++);
}

void PaletteFadeIn()
{
	PaletteFadeIn_At(0x00, 0x40);
}

void PaletteFadeIn_At(uint8_t ind, uint8_t len)
{
	//Initialize fade
	palette_fade.ind = ind;
	palette_fade.len = len;
	
	//Fill palette with black
	uint16_t *col = (&dry_palette[0][0]) + palette_fade.ind;
	for (int i = 0; i < palette_fade.len; i++)
		*col++ = 0x000;
	
	//Fade for 22 frames
	for (int i = 0; i < 22; i++)
	{
		vbla_routine = 0x12;
		WaitForVBla();
		FadeIn_FromBlack();
		RunPLC();
	}
}

//Fade out to black
void FadeOut_DecColour(uint16_t *col)
{
	uint16_t v = *col;
	if (v == 0)
		return;
	if (v & 0x00E)
		v -= 0x002;
	else if (v & 0x0E0)
		v -= 0x020;
	else if (v & 0xE00)
		v -= 0x200;
	*col = v;
}

void FadeOut_ToBlack()
{
	uint16_t *col;
	
	//Fade dry palette
	col = (&dry_palette[0][0]) + palette_fade.ind;
	for (int i = 0; i < palette_fade.len; i++)
		FadeOut_DecColour(col++);
	
	//Fade wet palette
	col = (&wet_palette[0][0]) + palette_fade.ind;
	for (int i = 0; i < palette_fade.len; i++)
		FadeOut_DecColour(col++);
}

void PaletteFadeOut()
{
	PaletteFadeOut_At(0x00, 0x40);
}

void PaletteFadeOut_At(uint8_t ind, uint8_t len)
{
	//Initialize fade
	palette_fade.ind = ind;
	palette_fade.len = len;
	
	//Fade for 22 frames
	for (int i = 0; i < 22; i++)
	{
		vbla_routine = 0x12;
		WaitForVBla();
		FadeOut_ToBlack();
		RunPLC();
	}
}
