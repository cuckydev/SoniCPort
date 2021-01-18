#include "Palette.h"

#include "Video.h"

#include <Backend/VDP.h>

#include <stdlib.h>

//Palette state
uint16_t dry_palette[4][16];
uint16_t dry_palette_dup[4][16];
uint16_t wet_palette[4][16];
uint16_t wet_palette_dup[4][16];

int16_t pcyc_num, pcyc_time;
uint16_t pcyc_buffer[0x18];

static struct
{
	uint8_t ind, len;
} palette_fade;

//Palettes
static const uint8_t pal_sega_background[] = {
	#include <Resource/Palette/SegaBackground.h>
};

static struct PalettePointer
{
	const uint16_t *palette;
	uint16_t *target;
	size_t colours;
} palette_pointers[] = {
	/* PalId_SegaBG */ {(const uint16_t*)pal_sega_background, &dry_palette[0][0], 0x40},
};

//Palette interface
void PalLoad1(PaletteId id)
{
	//Load given palette
	struct PalettePointer *palload = &palette_pointers[id];
	const uint16_t *inp = palload->palette;
	uint16_t *outp = palload->target + (dry_palette_dup - dry_palette);
	
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
	uint16_t *outp = palload->target + (wet_palette - dry_palette);
	
	for (size_t i = 0; i < palload->colours; i++, inp++)
		*outp++ = LESWAP_16(*inp);
}

void PalLoad4_Water(PaletteId id)
{
	//Load given palette
	struct PalettePointer *palload = &palette_pointers[id];
	const uint16_t *inp = palload->palette;
	uint16_t *outp = palload->target + (wet_palette_dup - dry_palette);
	
	for (size_t i = 0; i < palload->colours; i++, inp++)
		*outp++ = LESWAP_16(*inp);
}

//Palette fading
void FadeOut_DecColour(uint16_t *col)
{
	uint16_t v = *col;
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
