//This file was given by Clownacy

#include "Kosinski.h"

#include <stdbool.h>

static uint16_t descriptor_field;
static uint32_t descriptor_bits_remaining;
const uint8_t *source;

static void RefreshDescriptorField()
{
	descriptor_field = source[0] | (source[1] << 8);
	source += 2;
	
	descriptor_bits_remaining = 16;
}

static bool GetDescriptorBit()
{
	bool bit = descriptor_field & 1;
	
	descriptor_field >>= 1;
	
	if (--descriptor_bits_remaining == 0)
		RefreshDescriptorField();
	
	return bit;
}

uint8_t* KosDec(const uint8_t *_source, void *_destination)
{
	source = _source;
	uint8_t *destination = _destination;
	
	RefreshDescriptorField();
	
	for (;;)
	{
		if (GetDescriptorBit())
		{
			*destination++ = *source++;
		}
		else
		{
			uint32_t length = 0;
			int32_t offset;
			
			if (!GetDescriptorBit())
			{
				if (GetDescriptorBit())
					length += 2;
				
				if (GetDescriptorBit())
					++length;
				
				++length;
				
				offset = -0x100 + *source++;
			}
			else
			{
				uint8_t d0 = *source++;
				uint8_t d1 = *source++;
				
				offset = -0x2000 + (((d1 & 0xF8) << 5) | d0);
				length = d1 & 7;
				
				if (length != 0)
				{
					++length;
				}
				else
				{
					length = *source++;
					
					if (length == 0)
						break;
					
					if (length == 1)
						continue;
				}
			}
			
			do
			{
				*destination = destination[offset];
				++destination;
			} while (length-- != 0);
		}
	}
	
	return destination;
}
