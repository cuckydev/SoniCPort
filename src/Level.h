#pragma once

#include "Object.h"

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

extern uint8_t dle_routine;

extern int16_t limit_left1, limit_right1, limit_top1, limit_btm1;
extern int16_t limit_left2, limit_right2, limit_top2, limit_btm2;
extern int16_t limit_left3;

extern uint8_t last_lamp;

extern uint8_t level_map256[0xA400];
extern uint8_t level_map16[0x1800];
extern uint8_t level_layout[8][2][0x40];

extern uint8_t loopchunks[2][2];

extern Object objects[OBJECTS];
extern Object *const player;
extern Object *const level_objects;

//Global level resources
extern const uint8_t art_ghz1[];

//Level functions
void LoadLevelMaps();
void LoadLevelLayout();
void LoadMap16(ZoneId zone);
void LoadMap256(ZoneId zone);
void LevelSizeLoad();
