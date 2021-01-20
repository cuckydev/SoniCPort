#pragma once

#include "Object.h"

#include <stddef.h>

//Level constants
#define RESERVED_OBJECTS 0x20
#define LEVEL_OBJECTS    0x60
#define OBJECTS          (RESERVED_OBJECTS + LEVEL_OBJECTS)

//Level macros
#define LEVEL_ID(zone, level) (((zone) << 8) | (level))

//Level types
typedef enum
{
	ZoneId_GHZ,
	ZoneId_LZ,
	ZoneId_MZ,
	ZoneId_SLZ,
	ZoneId_SYZ,
	ZoneId_SBZ,
	ZoneId_EndZ,
	ZoneId_SS,
	ZoneId_Num,
} ZoneId;

//Level globals
extern uint16_t level_id;

extern uint8_t level_map256[0xA400];
extern uint8_t level_map16[0x1800];
extern uint8_t level_layout[8][2][0x40];

extern int16_t limitleft1, limitright1, limittop1, limitbtm1;
extern int16_t limitleft2, limitright2, limittop2, limitbtm2;
extern int16_t limitleft3;

extern Object objects[OBJECTS];
extern Object *const level_objects;

//Global level resources
extern const uint8_t art_ghz1[];
extern const size_t art_ghz1_size;

//Level functions
void LoadLevelMaps();
void LoadLevelLayout();
void LoadMap16(ZoneId zone);
void LoadMap256(ZoneId zone);
void LevelSizeLoad();
