#include "Level.h"

#include "Constants.h"

#include <Backend/VDP.h>

#include <string.h>

//Level state
uint16_t level_id;

//Loaded level data
ALIGNED2 uint8_t level_map256[0xA400];
ALIGNED2 uint8_t level_map16[0x1800];
uint8_t level_layout[8][2][0x40];

int16_t limitleft1, limitright1, limittop1, limitbtm1;
int16_t limitleft2, limitright2, limittop2, limitbtm2;
int16_t limitleft3;

//Objects
Object objects[OBJECTS];
Object *const level_objects = objects + RESERVED_OBJECTS;

//Level art
const uint8_t art_ghz1[] = {
	#include <Resource/Art/GHZ1.h>
};
const size_t art_ghz1_size = sizeof(art_ghz1);
static const uint8_t art_ghz2[] = {
	#include <Resource/Art/GHZ2.h>
};

//Level layouts
static const uint8_t layout_ghz1[] = {
	#include <Resource/Layout/GHZ1.h>
};
static const uint8_t layout_ghzbg[] = {
	#include <Resource/Layout/GHZBG.h>
};

//256x256 mappings
static const uint8_t map256_ghz[] = {
	#include <Resource/Map256/GHZ.h>
};

//16x16 mappings
static const uint8_t map16_ghz[] = {
	#include <Resource/Map16/GHZ.h>
};

//Level definitions
static const uint8_t *level_layouts[][2] = {
	{layout_ghz1, layout_ghzbg},
};

static const struct
{
	const uint8_t *map256; size_t map256_size;
	const uint8_t *map16; size_t map16_size;
} level_maps[] = {
	/* ZoneId_GHZ */ {map256_ghz, sizeof(map256_ghz), map16_ghz, sizeof(map16_ghz)},
};

static const uint16_t level_size[ZoneId_Num - 1][4][6] = {
	{ //ZoneId_GHZ
		{0x0004, 0x0000, 0x24BF, 0x0000, 0x0300, 0x0060},
		{0x0004, 0x0000, 0x1EBF, 0x0000, 0x0300, 0x0060},
		{0x0004, 0x0000, 0x2960, 0x0000, 0x0300, 0x0060},
		{0x0004, 0x0000, 0x2ABF, 0x0000, 0x0300, 0x0060},
	},
	{ //ZoneId_LZ
		{0x0004, 0x0000, 0x19BF, 0x0000, 0x0530, 0x0060},
		{0x0004, 0x0000, 0x10AF, 0x0000, 0x0720, 0x0060},
		{0x0004, 0x0000, 0x202F, 0xFF00, 0x0800, 0x0060},
		{0x0004, 0x0000, 0x20BF, 0x0000, 0x0720, 0x0060},
	},
	{ //ZoneId_MZ
		{0x0004, 0x0000, 0x17BF, 0x0000, 0x01D0, 0x0060},
		{0x0004, 0x0000, 0x17BF, 0x0000, 0x0520, 0x0060},
		{0x0004, 0x0000, 0x1800, 0x0000, 0x0720, 0x0060},
		{0x0004, 0x0000, 0x16BF, 0x0000, 0x0720, 0x0060},
	},
	{ //ZoneId_SLZ
		{0x0004, 0x0000, 0x1FBF, 0x0000, 0x0640, 0x0060},
		{0x0004, 0x0000, 0x1FBF, 0x0000, 0x0640, 0x0060},
		{0x0004, 0x0000, 0x2000, 0x0000, 0x06C0, 0x0060},
		{0x0004, 0x0000, 0x3EC0, 0x0000, 0x0720, 0x0060},
	},
	{ //ZoneId_SYZ
		{0x0004, 0x0000, 0x22C0, 0x0000, 0x0420, 0x0060},
		{0x0004, 0x0000, 0x28C0, 0x0000, 0x0520, 0x0060},
		{0x0004, 0x0000, 0x2C00, 0x0000, 0x0620, 0x0060},
		{0x0004, 0x0000, 0x2EC0, 0x0000, 0x0620, 0x0060},
	},
	{ //ZoneId_SBZ
		{0x0004, 0x0000, 0x21C0, 0x0000, 0x0720, 0x0060},
		{0x0004, 0x0000, 0x1E40, 0xFF00, 0x0800, 0x0060},
		{0x0004, 0x2080, 0x2460, 0x0510, 0x0510, 0x0060},
		{0x0004, 0x0000, 0x3EC0, 0x0000, 0x0720, 0x0060},
	},
	{ //ZoneId_EndZ
		{0x0004, 0x0000, 0x0500, 0x0110, 0x0110, 0x0060},
		{0x0004, 0x0000, 0x0DC0, 0x0110, 0x0110, 0x0060},
		{0x0004, 0x0000, 0x2FFF, 0x0000, 0x0320, 0x0060},
		{0x0004, 0x0000, 0x2FFF, 0x0000, 0x0320, 0x0060},
	}
};

//Level loading
void LoadLevelMaps()
{
	ZoneId zone = level_id >> 8;
	memcpy(level_map256, level_maps[zone].map256, level_maps[zone].map256_size);
	memcpy(level_map16, level_maps[zone].map16, level_maps[zone].map16_size);
}

void LoadLayout(const uint8_t *from, uint8_t *to)
{
	uint8_t width  = *from++;
	uint8_t height = *from++;
	
	do
	{
		for (size_t i = 0; i <= width; i++)
			*to++ = *from++;
		to += 0x80 - width;
	} while (height-- > 0);
}

void LoadLevelLayout()
{
	uint16_t index = ((level_id & 0xFF00) >> 6) | (level_id & 0x0003);
	LoadLayout(level_layouts[index][0], level_layout[0][0]);
	LoadLayout(level_layouts[index][1], level_layout[0][1]);
}

void LevelSizeLoad()
{
	
}

