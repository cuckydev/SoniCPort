//This file was given by Clownacy

#include "Nemesis.h"

#include <stdbool.h>
#include <stddef.h>

#include <Backend/VDP.h>

uint8_t nemesis_buffer[0x200];

static size_t nemesis_off;

void NemDecPrepare(NemesisState *state)
{
	uint8_t d0;
	uint8_t d7;
	
	d7 = *state->source++;
	
	if (d7 == 0xFF)
		return;
	
	for (;;)
	{
		for (;;)
		{
			d0 = *state->source++;
			
			if (d0 < 0x80)
				break;
			
			if (d0 == 0xFF)
				return;
			
			d7 = d0;
		}
		
		d7 &= 0xF;
		d7 |= d0 & 0x70;
		d0 &= 0xF;
		
		uint8_t d1 = 8 - d0;
		
		if (d1 == 0)
		{
			size_t index = *state->source++ * 2;
			
			state->dictionary[index] = d0;
			state->dictionary[index+1] = d7;
		}
		else
		{
			size_t index = (*state->source++ << d1) * 2;
			uint16_t d5 = (1 << d1) - 1;
			
			do
			{
				state->dictionary[index++] = d0;
				state->dictionary[index++] = d7;
			} while (d5-- != 0);
		}
	}
}

//This function needed a lot of restructuring to look good in C
void NemDecRun(NemesisState *state)
{
	for (;;)
	{
		while (state->d0-- != 0)
		{
			state->d4 <<= 4;
			state->d4 |= state->d1;
			
			if (--state->d3 == 0)
			{
				if (state->xor_mode)
				{
					state->d2 ^= state->d4;
					
					if (state->vram_mode)
					{
						uint32_t wdw = LESWAP_32(state->d2);
						VDP_WriteVRAM(nemesis_off, (const uint8_t*)&wdw, 4);
						nemesis_off += 4;
					}
					else
					{
						// NemDec_WriteAndAdvance_XOR
						*state->destination++ = (state->d2 >> 8 * 3) & 0xFF;
						*state->destination++ = (state->d2 >> 8 * 2) & 0xFF;
						*state->destination++ = (state->d2 >> 8 * 1) & 0xFF;
						*state->destination++ = (state->d2 >> 8 * 0) & 0xFF;
					}
				}
				else
				{
					if (state->vram_mode)
					{
						uint32_t wdw = LESWAP_32(state->d4);
						VDP_WriteVRAM(nemesis_off, (const uint8_t*)&wdw, 4);
						nemesis_off += 4;
					}
					else
					{
						// NemDec_WriteAndAdvance
						*state->destination++ = (state->d4 >> 8 * 3) & 0xFF;
						*state->destination++ = (state->d4 >> 8 * 2) & 0xFF;
						*state->destination++ = (state->d4 >> 8 * 1) & 0xFF;
						*state->destination++ = (state->d4 >> 8 * 0) & 0xFF;
					}
				}
				
				if (--state->remaining == 0)
					return;
				
				state->d4 = 0;
				state->d3 = 8;
			}
		}
		
		size_t index = (state->d5 >> (state->d6 - 8)) & 0xFF;
		
		if (index < 0xFC)
		{
			index *= 2;
			
			state->d6 -= state->dictionary[index];
			
			if (state->d6 < 9)
			{
				state->d6 += 8;
				
				state->d5 <<= 8;
				state->d5 |= *state->source++;
			}
			
			state->d0 = state->d1 = state->dictionary[index + 1];
			
			state->d1 &= 0xF;
			state->d0 &= 0xF0;
		}
		else
		{
			state->d6 -= 6;
			
			if (state->d6 < 9)
			{
				state->d6 += 8;
				
				state->d5 <<= 8;
				state->d5 |= *state->source++;
			}
			
			state->d6 -= 7;
			
			state->d0 = state->d1 = state->d5 >> state->d6;
			
			state->d1 &= 0xF;
			state->d0 &= 0x70;
			
			if (state->d6 < 9)
			{
				state->d6 += 8;
				
				state->d5 <<= 8;
				state->d5 |= *state->source++;
			}
		}
		
		state->d0 >>= 4;
		
		++state->d0;
	}
}

static void NemDecMain(NemesisState *state)
{
	state->dictionary = nemesis_buffer;
	
	uint16_t header = (state->source[0] << 8) | state->source[1];
	state->source += 2;
	
	state->xor_mode = header & 0x8000;
	
	state->remaining = (header << 3) & 0xFFFF;
	
	state->d3 = 8;
	state->d2 = 0;
	state->d4 = 0;
	
	NemDecPrepare(state);
	
	state->d5 = (state->source[0] << 8) | state->source[1];
	state->source += 2;
	
	state->d6 = 0x10;
	
	// These lines are new, to suit the restructured NemDecRun
	state->d0 = 0;
	//state->d1 = 0; // This line isn't actually necessary
	
	NemDecRun(state);
}

void NemDecSeek(size_t off)
{
	nemesis_off = off;
}

void NemDec(size_t off, const uint8_t *source)
{
	NemesisState state;
	
	nemesis_off = off;
	state.source = source;
	state.vram_mode = true;
	
	NemDecMain(&state);
}

void NemDecToRAM(const uint8_t *source, uint8_t *destination)
{
	NemesisState state;
	
	state.source = source;
	state.destination = destination;
	state.vram_mode = false;
	
	NemDecMain(&state);
}
