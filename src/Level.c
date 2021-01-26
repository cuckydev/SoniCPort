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
		{0x0060, 0x00C0},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
	},
	{ //ZoneId_MZ
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
	},
	{ //ZoneId_SLZ
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
	},
	{ //ZoneId_SYZ
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
	},
	{ //ZoneId_SBZ
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
	},
	{ //ZoneId_EndZ
		{0x0080, 0x00A8},
		{0x0080, 0x00A8},
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
	{PlcId_GHZ, art_ghz2, PlcId_GHZ2, map16_ghz, map256_ghz, 0, 0, PalId_GHZ, PalId_GHZ, sizeof(map16_ghz)},
};

//Level state
uint16_t level_id;

uint8_t dle_routine;

int16_t limit_left1, limit_right1, limit_top1, limit_btm1;
int16_t limit_left2, limit_right2, limit_top2, limit_btm2;
int16_t limit_left3;
int16_t limit_top_db, limit_btm_db;

LevelAnim level_anim[6];
const uint8_t *coll_index;

uint8_t last_lamp;

uint16_t restart;
uint16_t pause;

uint16_t frame_count;

//Player state
uint8_t lives;
uint16_t air;
uint8_t last_special;
uint8_t continues;

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

//Object state
Object objects[OBJECTS];
Object *const player = objects;
Object *const level_objects = objects + RESERVED_OBJECTS;

uint8_t btn_pushtime1, btn_pushtime2;
int16_t obj31_ypos;
uint8_t boss_status;
word_u track_pos;
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
	uint16_t index = LEVEL_INDEX(level_id);
	LoadLayout(level_layouts[index][0], level_layout[0][0]);
	LoadLayout(level_layouts[index][1], level_layout[0][1]);
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
	//if (level_id == LEVEL_ID(ZoneId_LZ, 3))
	//	pal = PalId_SBZ3;
	//if (level_id == LEVEL_ID(ZoneId_SBZ, 1) || level_id == LEVEL_ID(ZoneId_SBZ, 2))
	//	pal = PalId_SBZ2;
	PalLoad1(pal);
	
	//Load level art
	if (header->plc2 != PlcId_Main)
		AddPLC(header->plc2);
}
