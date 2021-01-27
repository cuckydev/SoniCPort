#pragma once

#include "Object.h"
#include "PLC.h"

//Level constants
#define RESERVED_OBJECTS 0x20
#define LEVEL_OBJECTS    0x60
#define OBJECTS          (RESERVED_OBJECTS + LEVEL_OBJECTS)

//Level macros
#define LEVEL_ID(zone, level) (((zone) << 8) | (level))
#define LEVEL_ZONE(id)        ((id) >> 8)
#define LEVEL_ACT(id)         ((id) & 0x3)
#define LEVEL_INDEX(id)       ((LEVEL_ZONE(id) << 2) | LEVEL_ACT(id))

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

typedef struct
{
	uint8_t frame, time;
} LevelAnim;

typedef struct
{
	uint16_t direction;
	uint16_t state[16][2];
} Oscillatory;

typedef struct
{
	uint8_t plc1;
	const uint8_t *art;
	uint8_t plc2;
	const uint8_t *map16;
	const uint8_t *map256;
	uint8_t pad;
	uint8_t music;
	uint8_t pal_dup;
	uint8_t pal;
	size_t map16_size; //TEMP
} LevelHeader;

//Level headers
extern const LevelHeader level_header[ZoneId_Num];

//Level globals
extern uint16_t level_id;

extern uint8_t dle_routine;

extern int16_t limit_left1, limit_right1, limit_top1, limit_btm1;
extern int16_t limit_left2, limit_right2, limit_top2, limit_btm2;
extern int16_t limit_left3;
extern int16_t limit_top_db, limit_btm_db;

extern LevelAnim level_anim[6];

extern uint8_t last_lamp;

extern uint16_t restart;
extern uint16_t pause;
extern uint8_t time_over;

extern uint16_t frame_count;

extern uint32_t score;
extern uint32_t time;
extern uint16_t rings;
extern uint8_t lives;
extern uint8_t continues;

extern uint16_t air;
extern uint8_t last_special;

extern uint8_t lifecount;
extern uint8_t hud_life;
extern uint8_t hud_ring;
extern uint8_t hud_time;
extern uint8_t hud_score;

extern uint8_t shield;
extern uint8_t invincibility;
extern uint8_t shoes;
extern uint8_t debug_use;

extern int16_t wtr_pos1, wtr_pos2, wtr_pos3;
extern uint8_t water;
extern uint8_t wtr_routine;
extern uint8_t wtr_state;

extern uint8_t level_map256[0xA400];
extern uint8_t level_map16[0x1800];
extern uint8_t level_layout[8][2][0x40];
extern uint8_t level_schunks[2][2];
extern const uint8_t *coll_index;

extern Object objects[OBJECTS];
extern Object *const player;
extern Object *const level_objects;

extern uint8_t btn_pushtime1, btn_pushtime2;
extern int16_t obj31_ypos;
extern uint8_t boss_status;
extern uint8_t lock_screen;
extern uint16_t gfx_big_ring;
extern uint8_t convey_rev;
extern uint8_t obj63[6];
extern uint8_t tunnel_mode;
extern uint8_t lock_multi;
extern uint8_t tunnel_allow;
extern uint8_t jump_only;
extern uint8_t obj6B;
extern uint8_t lock_ctrl;
extern uint8_t big_ring;
extern uint16_t item_bonus;
extern uint16_t time_bonus;
extern uint16_t ring_bonus;
extern uint8_t endact_bonus;
extern uint8_t sonicend;
extern uint16_t lz_deform;
extern uint8_t f_switch[0x10];

extern LevelAnim sprite_anim[4];

extern Oscillatory oscillatory;

extern uint16_t opl_routine, opl_screen;
extern uint8_t opl_data[0x10];

//Level functions
void LoadLevelMaps();
void LoadLevelLayout();
void LoadMap16(ZoneId zone);
void LoadMap256(ZoneId zone);
void LevelSizeLoad();
void LevelDataLoad();
void ColIndexLoad();
