#pragma once

#include "Object.h"
#include "PLC.h"

//Level macros
#define LEVEL_ID(zone, level) (((zone) << 8) | (level))
#define LEVEL_ZONE(id)        ((id) >> 8)
#define LEVEL_ACT(id)         ((id) & 0x3)
#define LEVEL_INDEX(id)       ((LEVEL_ZONE(id) << 2) | LEVEL_ACT(id))

//Level bitfield structures
#define META_SOLID_LRB 0x4000
#define META_SOLID_TOP 0x2000
#define META_Y_FLIP    0x1000
#define META_X_FLIP    0x0800
#define META_TILE      0x07FF

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
	uint8_t frame;
	int8_t time;
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

typedef struct
{
	uint8_t pad, min, sec, frame;
} LevelTime;

//Level headers
extern const LevelHeader level_header[ZoneId_Num];

//Level globals
extern uint16_t level_id;

extern uint8_t dle_routine;

extern uint16_t limit_left1, limit_right1, limit_top1, limit_btm1;
extern uint16_t limit_left2, limit_right2, limit_top2, limit_btm2;
extern uint16_t limit_left3;
extern uint16_t limit_top_db, limit_btm_db;

extern LevelAnim level_anim[6];

extern uint8_t last_lamp;

extern uint16_t restart;
extern uint16_t pause;
extern uint8_t time_over;

extern uint16_t frame_count;

extern uint32_t score;
extern LevelTime time;
extern uint16_t rings;
extern uint8_t lives;
extern uint8_t continues;

extern uint32_t score_life;

extern uint16_t air;
extern uint8_t last_special;

extern uint8_t life_num;
extern uint8_t life_count;
extern uint8_t ring_count;
extern uint8_t time_count;
extern uint8_t score_count;

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

extern uint16_t opl_routine;
extern int16_t opl_screen;
extern const uint8_t *opl_ptr0;
extern const uint8_t *opl_ptr4;
extern const uint8_t *opl_ptr8;
extern const uint8_t *opl_ptrC;

extern uint8_t objstate_left;
extern uint8_t objstate_right;
extern uint8_t objstate[0x100];

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

extern Oscillatory oscillatory;

extern LevelAnim sprite_anim[4];
extern uint16_t sprite_anim_3buf;

//Game functions
void AddPoints(uint16_t points);

//Level functions
void LoadLevelMaps();
void LoadLevelLayout();
void LoadMap16(ZoneId zone);
void LoadMap256(ZoneId zone);
void LevelSizeLoad();
void LevelDataLoad();
void ColIndexLoad();
void DynamicLevelEvents();
void SynchroAnimate();
void ObjPosLoad();
