#include "Palette.h"

#include "Video.h"

#include <Backend/VDP.h>

#include <stdlib.h>

//Palette state
uint16_t dry_palette[4][16];
uint16_t dry_palette_dup[4][16];
uint16_t wet_palette[4][16];
uint16_t wet_palette_dup[4][16];

static struct
{
	uint8_t ind, len;
} palette_fade;

//Palettes
static ALIGNED2 const uint8_t pal_sega_background[] = {
	#include <Resource/Palette/SegaBackground.h>
};
static ALIGNED2 const uint8_t pal_title[] = {
	#include <Resource/Palette/Title.h>
};
static ALIGNED2 const uint8_t pal_sonic[] = {
	#include <Resource/Palette/Sonic.h>
};

static struct PalettePointer
{
	const uint16_t *palette;
	uint16_t *target;
	size_t colours;
} palette_pointers[] = {
	/* PalId_SegaBG */ {(const uint16_t*)pal_sega_background, &dry_palette[0][0], 0x40},
	/* PalId_Title  */ {(const uint16_t*)pal_title,           &dry_palette[0][0], 0x40},
	/* PalId_Sonic  */ {(const uint16_t*)pal_sonic,           &dry_palette[0][0], 0x10},
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

int PaletteFadeIn()
{
	int result;
	
	//Initialize fade
	palette_fade.ind = 0x00;
	palette_fade.len = 0x40;
	
	//Fill palette with black
	VDP_FillCRAM(0x0000, 0x0000, 0x40);
	
	//Fade for 22 frames
	for (int i = 0; i < 22; i++)
	{
		vbla_routine = 0x12;
		if ((result = WaitForVBla()))
			return result;
		FadeIn_FromBlack();
	}
	
	return 0;
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

int PaletteFadeOut()
{
	int result;
	
	//Initialize fade
	palette_fade.ind = 0x00;
	palette_fade.len = 0x40;
	
	//Fade for 22 frames
	for (int i = 0; i < 22; i++)
	{
		vbla_routine = 0x12;
		if ((result = WaitForVBla()))
			return result;
		FadeOut_ToBlack();
	}
	
	return 0;
}
