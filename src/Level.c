#include "Level.h"

#include <Constants.h>
#include "Game.h"
#include "LevelScroll.h"
#include "LevelDraw.h"
#include "Kosinski.h"
#include "PLC.h"
#include "Palette.h"

#include <Backend/VDP.h>

#include <string.h>

//Level layouts
static const uint8_t layout_ghz1[] = {
	#include <Resource/Layout/GHZ1.h>
};
static const uint8_t layout_ghz2[] = {
	#include <Resource/Layout/GHZ2.h>
};
static const uint8_t layout_ghz3[] = {
	#include <Resource/Layout/GHZ3.h>
};
static const uint8_t layout_ghzbg[] = {
	#include <Resource/Layout/GHZBG.h>
};
static const uint8_t layout_lz1[] = {
	#include <Resource/Layout/LZ1.h>
};
static const uint8_t layout_lz2[] = {
	#include <Resource/Layout/LZ2.h>
};
static const uint8_t layout_lz3[] = {
	#include <Resource/Layout/LZ3.h>
};
static const uint8_t layout_lzbg[] = {
	#include <Resource/Layout/LZBG.h>
};
static const uint8_t layout_mz1[] = {
	#include <Resource/Layout/MZ1.h>
};
static const uint8_t layout_mz1bg[] = {
	#include <Resource/Layout/MZ1BG.h>
};
static const uint8_t layout_mz2[] = {
	#include <Resource/Layout/MZ2.h>
};
static const uint8_t layout_mz2bg[] = {
	#include <Resource/Layout/MZ2BG.h>
};
static const uint8_t layout_mz3[] = {
	#include <Resource/Layout/MZ3.h>
};
static const uint8_t layout_mz3bg[] = {
	#include <Resource/Layout/MZ3BG.h>
};
static const uint8_t layout_slz1[] = {
	#include <Resource/Layout/SLZ1.h>
};
static const uint8_t layout_slz2[] = {
	#include <Resource/Layout/SLZ2.h>
};
static const uint8_t layout_slz3[] = {
	#include <Resource/Layout/SLZ3.h>
};
static const uint8_t layout_slzbg[] = {
	#include <Resource/Layout/SLZBG.h>
};
static const uint8_t layout_syz1[] = {
	#include <Resource/Layout/SYZ1.h>
};
static const uint8_t layout_syz2[] = {
	#include <Resource/Layout/SYZ2.h>
};
static const uint8_t layout_syz3[] = {
	#include <Resource/Layout/SYZ3.h>
};
#ifdef SCP_REV00
	static const uint8_t layout_syzbg[] = {
		#include <Resource/Layout/SYZBGREV00.h>
	};
#else
	static const uint8_t layout_syzbg[] = {
		#include <Resource/Layout/SYZBGREV01.h>
	};
#endif
static const uint8_t layout_sbz1[] = {
	#include <Resource/Layout/SBZ1.h>
};
static const uint8_t layout_sbz1bg[] = {
	#include <Resource/Layout/SBZ1BG.h>
};
static const uint8_t layout_sbz2[] = {
	#include <Resource/Layout/SBZ2.h>
};
static const uint8_t layout_sbz2bg[] = {
	#include <Resource/Layout/SBZ2BG.h>
};
static const uint8_t layout_sbz3[] = {
	#include <Resource/Layout/SBZ3.h>
};
static const uint8_t layout_ending[] = {
	#include <Resource/Layout/Ending.h>
};

//256x256 mappings
static const uint8_t map256_ghz[] = {
	#include <Resource/Map256/GHZ.h>
};
static const uint8_t map256_lz[] = {
	#include <Resource/Map256/LZ.h>
};
#ifdef SCP_REV00
	static const uint8_t map256_mz[] = {
		#include <Resource/Map256/MZREV00.h>
	};
#else
	static const uint8_t map256_mz[] = {
		#include <Resource/Map256/MZREV01.h>
	};
#endif
static const uint8_t map256_slz[] = {
	#include <Resource/Map256/SLZ.h>
};
static const uint8_t map256_syz[] = {
	#include <Resource/Map256/SYZ.h>
};
#ifdef SCP_REV00
	static const uint8_t map256_sbz[] = {
		#include <Resource/Map256/SBZREV00.h>
	};
#else
	static const uint8_t map256_sbz[] = {
		#include <Resource/Map256/SBZREV01.h>
	};
#endif

//16x16 mappings
static const uint8_t map16_ghz[] = {
	#include <Resource/Map16/GHZ.h>
};
static const uint8_t map16_lz[] = {
	#include <Resource/Map16/LZ.h>
};
static const uint8_t map16_mz[] = {
	#include <Resource/Map16/MZ.h>
};
static const uint8_t map16_slz[] = {
	#include <Resource/Map16/SLZ.h>
};
static const uint8_t map16_syz[] = {
	#include <Resource/Map16/SYZ.h>
};
static const uint8_t map16_sbz[] = {
	#include <Resource/Map16/SBZ.h>
};

//Collision indices
static const uint8_t coli_ghz[] = {
	#include <Resource/CollisionIndex/GHZ.h>
};
static const uint8_t coli_lz[] = {
	#include <Resource/CollisionIndex/LZ.h>
};
static const uint8_t coli_mz[] = {
	#include <Resource/CollisionIndex/MZ.h>
};
static const uint8_t coli_slz[] = {
	#include <Resource/CollisionIndex/SLZ.h>
};
static const uint8_t coli_syz[] = {
	#include <Resource/CollisionIndex/SYZ.h>
};
static const uint8_t coli_sbz[] = {
	#include <Resource/CollisionIndex/SBZ.h>
};

//Level definitions
static const struct
{
	const uint8_t *layout_fg;
	const uint8_t *layout_bg;
	const uint8_t *layout_3;
} level_layouts[ZoneId_Num][4] = {
	{ //ZoneId_GHZ
		{layout_ghz1,   layout_ghzbg,  NULL},
		{layout_ghz2,   layout_ghzbg,  NULL},
		{layout_ghz3,   layout_ghzbg,  NULL},
		{NULL,          NULL,          NULL},
	},
	{ //ZoneId_LZ
		{layout_lz1,    layout_lzbg,   NULL},
		{layout_lz2,    layout_lzbg,   NULL},
		{layout_lz3,    layout_lzbg,   NULL},
		{layout_sbz3,   layout_lzbg,   NULL},
	},
	{ //ZoneId_MZ
		{layout_mz1,    layout_mz1bg,  layout_mz1},
		{layout_mz2,    layout_mz2bg,  NULL},
		{layout_mz3,    layout_mz3bg,  NULL},
		{NULL,          NULL,          NULL},
	},
	{ //ZoneId_SLZ
		{layout_slz1,   layout_slzbg,  NULL},
		{layout_slz2,   layout_slzbg,  NULL},
		{layout_slz3,   layout_slzbg,  NULL},
		{NULL,          NULL,          NULL},
	},
	{ //ZoneId_SYZ
		{layout_syz1,   layout_syzbg,  NULL},
		{layout_syz2,   layout_syzbg,  NULL},
		{layout_syz3,   layout_syzbg,  NULL},
		{NULL,          NULL,          NULL},
	},
	{ //ZoneId_SBZ
		{layout_sbz1,   layout_sbz1bg, layout_sbz1bg},
		{layout_sbz2,   layout_sbz2bg, layout_sbz2bg},
		{layout_sbz2,   layout_sbz2bg, NULL},
		{NULL,          NULL,          NULL},
	},
	{ //ZoneId_EndZ
		{layout_ending, layout_ghzbg,  NULL},
		{layout_ending, layout_ghzbg,  NULL},
		{NULL,          NULL,          NULL},
		{NULL,          NULL,          NULL},
	},
};

static const int16_t ldef_size[ZoneId_Num][4][6] = {
	{ //ZoneId_GHZ
		{0x0004, 0x0000, 0x24BF, 0x0000, 0x0300, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x1EBF, 0x0000, 0x0300, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x2960, 0x0000, 0x0300, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x2ABF, 0x0000, 0x0300, 96 + SCREEN_TALLADD2},
	},
	{ //ZoneId_LZ
		{0x0004, 0x0000, 0x19BF, 0x0000, 0x0530, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x10AF, 0x0000, 0x0720, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x202F,-0x0100, 0x0800, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x20BF, 0x0000, 0x0720, 96 + SCREEN_TALLADD2},
	},
	{ //ZoneId_MZ
		{0x0004, 0x0000, 0x17BF, 0x0000, 0x01D0, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x17BF, 0x0000, 0x0520, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x1800, 0x0000, 0x0720, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x16BF, 0x0000, 0x0720, 96 + SCREEN_TALLADD2},
	},
	{ //ZoneId_SLZ
		{0x0004, 0x0000, 0x1FBF, 0x0000, 0x0640, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x1FBF, 0x0000, 0x0640, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x2000, 0x0000, 0x06C0, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x3EC0, 0x0000, 0x0720, 96 + SCREEN_TALLADD2},
	},
	{ //ZoneId_SYZ
		{0x0004, 0x0000, 0x22C0, 0x0000, 0x0420, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x28C0, 0x0000, 0x0520, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x2C00, 0x0000, 0x0620, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x2EC0, 0x0000, 0x0620, 96 + SCREEN_TALLADD2},
	},
	{ //ZoneId_SBZ
		{0x0004, 0x0000, 0x21C0, 0x0000, 0x0720, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x1E40,-0x0100, 0x0800, 96 + SCREEN_TALLADD2},
		{0x0004, 0x2080, 0x2460, 0x0510, 0x0510, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x3EC0, 0x0000, 0x0720, 96 + SCREEN_TALLADD2},
	},
	{ //ZoneId_EndZ
		{0x0004, 0x0000, 0x0500, 0x0110, 0x0110, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x0DC0, 0x0110, 0x0110, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x2FFF, 0x0000, 0x0320, 96 + SCREEN_TALLADD2},
		{0x0004, 0x0000, 0x2FFF, 0x0000, 0x0320, 96 + SCREEN_TALLADD2},
	}
};

//Player start positions
static const int16_t ldef_start[ZoneId_Num][4][2] = {
	{ //ZoneId_GHZ
		{0x0050, 0x03B0},
		{0x0050, 0x00FC},
		{0x0050, 0x03B0},
		{0x0080, 0x00A8},
	},
	{ //ZoneId_LZ
		{0x0060, 0x006C},
		{0x0050, 0x00EC},
		{0x0050, 0x02EC},
		{0x0B80, 0x0000},
	},
	{ //ZoneId_MZ
		{0x0030, 0x0266},
		{0x0030, 0x0266},
		{0x0030, 0x0166},
		{0x0080, 0x00A8},
	},
	{ //ZoneId_SLZ
		{0x0040, 0x02CC},
		{0x0040, 0x014C},
		{0x0040, 0x014C},
		{0x0080, 0x00A8},
	},
	{ //ZoneId_SYZ
		{0x0030, 0x03BD},
		{0x0030, 0x01BD},
		{0x0030, 0x00EC},
		{0x0080, 0x00A8},
	},
	{ //ZoneId_SBZ
		{0x0030, 0x048C},
		{0x0030, 0x074C},
		{0x2140, 0x05AC},
		{0x0080, 0x00A8},
	},
	{ //ZoneId_EndZ
		{0x0620, 0x016B},
		{0x0EE0, 0x016C},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
	},
};

//Level loop (and S-tube) chunks
static const uint8_t ldef_schunks[ZoneId_Num][2][2] = {
	{{0xB5, 0x7F}, {0x1F, 0x20}}, //ZoneId_GHZ
	{{0x7F, 0x7F}, {0x7F, 0x7F}}, //ZoneId_LZ
	{{0x7F, 0x7F}, {0x7F, 0x7F}}, //ZoneId_MZ
	{{0xAA, 0xB4}, {0x7F, 0x7F}}, //ZoneId_SLZ
	{{0x7F, 0x7F}, {0x7F, 0x7F}}, //ZoneId_SYZ
	{{0x7F, 0x7F}, {0x7F, 0x7F}}, //ZoneId_SBZ
	{{0x7F, 0x7F}, {0x7F, 0x7F}}, //ZoneId_EndZ
};

//Level scroll block sizes
static const int16_t ldef_scrollsize[ZoneId_Num][4] = {
	{ 0x70, 0x100, 0x100, 0x100},
	{0x800, 0x100, 0x100, 0},
	{0x800, 0x100, 0x100, 0},
	{0x800, 0x100, 0x100, 0},
	{0x800, 0x100, 0x100, 0},
	{0x800, 0x100, 0x100, 0},
	{ 0x70, 0x100, 0x100, 0x100},
};

//Level headers
const LevelHeader level_header[ZoneId_Num] = {
	{PlcId_GHZ, art_ghz2, PlcId_GHZ2, map16_ghz, map256_ghz, 0, 0, PalId_GHZ,  PalId_GHZ,  sizeof(map16_ghz)},
	{PlcId_LZ,  art_lz,   PlcId_LZ2,  map16_lz,  map256_lz,  0, 0, PalId_LZ,   PalId_LZ,   sizeof(map16_lz)},
	{PlcId_MZ,  art_mz,   PlcId_MZ2,  map16_mz,  map256_mz,  0, 0, PalId_MZ,   PalId_MZ,   sizeof(map16_mz)},
	{PlcId_SLZ, art_slz,  PlcId_SLZ2, map16_slz, map256_slz, 0, 0, PalId_SLZ,  PalId_SLZ,  sizeof(map16_slz)},
	{PlcId_SYZ, art_syz,  PlcId_SYZ2, map16_syz, map256_syz, 0, 0, PalId_SYZ,  PalId_SYZ,  sizeof(map16_syz)},
	{PlcId_SBZ, art_sbz,  PlcId_SBZ2, map16_sbz, map256_sbz, 0, 0, PalId_SBZ1, PalId_SBZ1, sizeof(map16_sbz)},
	{0,         art_ghz2, 0,          map16_ghz, map256_ghz, 0, 0, PalId_GHZ,  PalId_GHZ,  sizeof(map16_ghz)},
};

//Level collision indices
const uint8_t *level_coli[ZoneId_Num - 1] = {
	coli_ghz,
	coli_lz,
	coli_mz,
	coli_slz,
	coli_syz,
	coli_sbz,
};

//Level state
uint16_t level_id;

uint8_t dle_routine;

uint16_t limit_left1, limit_right1, limit_top1, limit_btm1;
uint16_t limit_left2, limit_right2, limit_top2, limit_btm2;
uint16_t limit_left3;
uint16_t limit_top_db, limit_btm_db;

LevelAnim level_anim[6];

uint8_t last_lamp;

uint16_t restart;
uint16_t pause;
uint8_t time_over;

uint16_t frame_count;

//Player state
uint32_t score;
uint32_t time;
uint16_t rings;
uint8_t lives;
uint8_t continues;

uint32_t score_life;

uint16_t air;
uint8_t last_special;

uint8_t life_num;
uint8_t life_count;
uint8_t ring_count;
uint8_t time_count;
uint8_t score_count;

uint8_t shield;
uint8_t invincibility;
uint8_t shoes;
uint8_t debug_use;

//Water state
int16_t wtr_pos1, wtr_pos2, wtr_pos3;
uint8_t water;
uint8_t wtr_routine;
uint8_t wtr_state;

//Loaded level data
ALIGNED2 uint8_t level_map256[0xA400];
ALIGNED2 uint8_t level_map16[0x1800];
uint8_t level_layout[8][2][0x40];
uint8_t level_schunks[2][2];
const uint8_t *coll_index;

//Object state
Object objects[OBJECTS];
Object *const player = objects;
Object *const level_objects = objects + RESERVED_OBJECTS;

int16_t obj31_ypos;
uint8_t boss_status;
uint8_t lock_screen;
uint16_t gfx_big_ring;
uint8_t convey_rev;
uint8_t obj63[6];
uint8_t tunnel_mode;
uint8_t lock_multi;
uint8_t tunnel_allow;
uint8_t jump_only;
uint8_t obj6B;
uint8_t lock_ctrl;
uint8_t big_ring;
uint16_t item_bonus;
uint16_t time_bonus;
uint16_t ring_bonus;
uint8_t endact_bonus;
uint8_t sonicend;
uint16_t lz_deform;
uint8_t f_switch[0x10];

LevelAnim sprite_anim[4];

Oscillatory oscillatory;

uint16_t opl_routine, opl_screen;
uint8_t opl_data[0x10];

//Level loading
void LoadLevelMaps()
{
	//Get header
	const LevelHeader *header = &level_header[LEVEL_ZONE(level_id)];
	
	//Load chunk maps and tile map
	KosDec(header->map256, level_map256);
	memcpy(level_map16, header->map16, header->map16_size);
}

void LoadLayout(const uint8_t *from, uint8_t *to)
{
	//Read layout header (dimensions - 1)
	uint8_t width  = *from++;
	uint8_t height = *from++;
	
	//Read layout data
	do
	{
		for (size_t i = 0; i <= width; i++)
			*to++ = *from++;
		to += 0x80 - (width + 1);
	} while (height-- > 0);
}

void LoadLevelLayout()
{
	//Load foreground and background layers
	LoadLayout(
		level_layouts[LEVEL_ZONE(level_id)][LEVEL_ACT(level_id)].layout_fg,
		level_layout[0][0]);
	LoadLayout(
		level_layouts[LEVEL_ZONE(level_id)][LEVEL_ACT(level_id)].layout_bg,
		level_layout[0][1]);
}

void LevelSizeLoad()
{
	//Reset level state
	dle_routine = 0;
	
	//Get sizes to load
	const int16_t *sizes = ldef_size[LEVEL_ZONE(level_id)][LEVEL_ACT(level_id)];
	
	//Load sizes and other stuff
	/* FFFFF730 = */ sizes++;
	limit_left2 = *sizes;
	limit_left1 = *sizes++;
	limit_right2 = *sizes;
	limit_right1 = *sizes++;
	limit_top2 = *sizes;
	limit_top1 = *sizes++;
	limit_btm2 = *sizes;
	limit_btm1 = *sizes++;
	limit_left3 = limit_left2 + 0x240;
	look_shift = *sizes++;
	
	//Load player start
	int16_t x, y;
	if (last_lamp)
	{
		//TODO
		//Lamp_LoadInfo();
		x = player->pos.l.x.f.u;
		y = player->pos.l.y.f.u;
	}
	else
	{
		if (demo < 0)
		{
			//TODO - in an ending demo
			x = 0x80;
			y = 0xA8;
		}
		else
		{
			//Level
			x = ldef_start[LEVEL_ZONE(level_id)][LEVEL_ACT(level_id)][0];
			y = ldef_start[LEVEL_ZONE(level_id)][LEVEL_ACT(level_id)][1];
		}
		
		player->pos.l.x.f.u = x;
		player->pos.l.y.f.u = y;
	}
	
	//Clip camera position against left and right
	if ((x -= (SCREEN_WIDTH / 2)) < 0) //0 instead of limit_left
		x = 0;
	if (x >= limit_right2)
		x = limit_right2;
	scrpos_x.f.u = x;
	
	//Clip camera position against top and bottom
	if ((y -= (96 + SCREEN_TALLADD2)) < 0) //0 instead of limit_top
		y = 0;
	if (y >= limit_btm2)
		y = limit_btm2;
	scrpos_y.f.u = y;
	
	//Load other level stuff
	BgScrollSpeed(x, y);
	memcpy(&level_schunks[0][0], &ldef_schunks[LEVEL_ZONE(level_id)][0][0], 4);
	
	const int16_t *scroll_size = ldef_scrollsize[LEVEL_ZONE(level_id)];
	scroll_block1_size = *scroll_size++;
	scroll_block2_size = *scroll_size++;
	scroll_block3_size = *scroll_size++;
	scroll_block4_size = *scroll_size++;
}

void LevelDataLoad()
{
	//Get header
	const LevelHeader *header = &level_header[LEVEL_ZONE(level_id)];
	
	//Load chunk maps and tile map
	KosDec(header->map256, level_map256);
	memcpy(level_map16, header->map16, header->map16_size);
	
	//Load level layout
	LoadLevelLayout();
	
	//Load level palette
	PaletteId pal = header->pal;
	if (level_id == LEVEL_ID(ZoneId_LZ, 3))
		pal = PalId_SBZ3;
	if (level_id == LEVEL_ID(ZoneId_SBZ, 1) || level_id == LEVEL_ID(ZoneId_SBZ, 2))
		pal = PalId_SBZ2;
	PalLoad1(pal);
	
	//Load level art
	if (header->plc2 != 0)
		AddPLC(header->plc2);
}

void ColIndexLoad()
{
	//Use zone's collision indices
	coll_index = level_coli[LEVEL_ZONE(level_id)];
}

//Dynamic level events
void DynamicLevelEvents()
{
	//Update target scroll limits
	switch (LEVEL_ZONE(level_id))
	{
		case ZoneId_GHZ:
			switch (LEVEL_ACT(level_id))
			{
				case 0: //Act 1
					if ((uint16_t)scrpos_x.f.u >= (0x1780 - SCREEN_WIDEADD2))
						limit_btm1 = 0x400 - SCREEN_TALLADD;
					else
						limit_btm1 = 0x300 - SCREEN_TALLADD;
					break;
				case 1: //Act 2
					break;
				case 2: //Act 3
					break;
			}
			break;
		default:
			break;
	}
	
	//Update scroll limits
	int16_t scroll_diff = limit_btm1 - limit_btm2;
	int16_t spd = 2;
	
	if (scroll_diff < 0)
	{
		if ((uint16_t)scrpos_y.f.u > limit_btm1)
			limit_btm2 = scrpos_y.f.u & ~1;
		limit_btm2 -= spd;
		bgscrollvert = true;
	}
	else if (scroll_diff > 0)
	{
		if (((uint16_t)scrpos_y.f.u + 8) >= limit_btm2 && player->status.p.f.in_air)
			spd *= 4;
		limit_btm2 += spd;
		bgscrollvert = true;
	}
}
