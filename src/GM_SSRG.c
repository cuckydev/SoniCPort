//Ported from MarkeyJester's 'SSRG Splash Screen'

#include "GM_SSRG.h"

#include "Game.h"
#include "Video.h"
#include "Palette.h"
#include "Level.h"
#include "Nemesis.h"
#include "Kosinski.h"
#include "PLC.h"

#include <string.h>

//SSRG memory
#define ssrg_memory level_map256 //RAM 0xFFFF0000

#define ssrg_scroll_fg (*(int16_t*)(ssrg_memory + 0x7800))
#define ssrg_scroll_bg (*(int16_t*)(ssrg_memory + 0x7808))

//SSRG assets
static const uint8_t art_link[] = {
	#include <Resource/SSRG/ArtLink.h>
	,0,
};
static const uint8_t art_main[] = {
	#include <Resource/SSRG/ArtMain.h>
	,0,
};
static const uint8_t art_sonic[] = {
	#include <Resource/SSRG/ArtSonic.h>
	,0,
};
static const uint8_t art_square[] = {
	#include <Resource/SSRG/ArtSquare.h>
	,0,
};
static const uint8_t map_link[] = {
	#include <Resource/SSRG/MapLink.h>
};
static const uint8_t map_main[] = {
	#include <Resource/SSRG/MapMain.h>
};
static const uint8_t map_square[] = {
	#include <Resource/SSRG/MapSquare.h>
};

static const uint16_t pal_ssrg[] = {
	0x0000,0x0400,0x0800,0x0C00,0x0E00,0x0EEE,0x0AAA,0x0888,0x0666,0x0444,0x0222,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0444,0x0888,0x0CCC,0x0EEE,0x0888,0x0666,0x0444,0x0222,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0422,0x0844,0x0C66,0x0E88,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0404,0x0808,0x0E0E,0x00E0,0x00C0,0x00A0,0x0080,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};

//SSRG planes
static void CopyTilemap_Single(uint16_t v, size_t offset, size_t width, size_t height)
{
	while (height-- > 0)
	{
		VDP_SeekVRAM(offset);
		for (size_t x = 0; x < width; x++)
			VDP_WriteVRAM((const uint8_t*)&v, 2);
		offset += PLANE_WIDTH * 2;
	}
}


static void CopyTilemap_Add(const uint8_t *tilemap, size_t offset, size_t width, size_t height, uint16_t add)
{
	while (height-- > 0)
	{
		VDP_SeekVRAM(offset);
		for (size_t x = 0; x < width; x++)
		{
			uint16_t v = ((*tilemap++ << 8) | (*tilemap++ << 0)) + add;
			VDP_WriteVRAM((const uint8_t*)&v, 2);
		}
		offset += PLANE_WIDTH * 2;
	}
}

static void SRG_ScrollFG()
{
	int16_t *bufp;
	
	//Scroll FG
	bufp = &hscroll_buffer[115][0];
	
	int16_t scroll_fg = ssrg_scroll_fg + 0x30;
	if (scroll_fg < 0xF7)
	{
		scroll_fg *= -2;
		scroll_fg += (PLANE_WIDTH - 64) * 8;
		for (int i = 0; i < 0x20; i++)
		{ *bufp++ = scroll_fg; *bufp++ = 0; }
	}
	
	//Scroll BG
	bufp = &hscroll_buffer[0][0];
	
	int16_t scroll_bg = ssrg_scroll_bg;
	for (int i = 0; i < SCREEN_HEIGHT; i++)
	{ bufp++; *bufp++ = scroll_bg; }
}

static void SRG_DrawFG()
{
	//Get scroll value
	uint16_t scroll_fg = (ssrg_scroll_fg * 2) - 0x40;
	
	if (scroll_fg <= 0x110)
	{
		//Get scroll offset
		uint16_t scroll_off = ((scroll_fg & 0x01F8) >> 2);
		
		//Get scroll offsets
		size_t offset = MAP_PLANE(VRAM_FG, 2, 14) + PLANE_WIDEADD + PLANE_TALLADD + scroll_off;
		const uint8_t *mapp = ssrg_memory + scroll_off;
		
		//Write plane data
		for (int i = 0; i < 3; i++)
		{
			uint16_t v = ((mapp[0] << 8) | (mapp[1] << 0)) + 0x2000;
			VDP_SeekVRAM(offset);
			VDP_WriteVRAM((const uint8_t*)&v, 2);
			offset += PLANE_WIDTH << 1;
			mapp += 0x46;
		}
	}
	else
	{
		//Make sure we've reached the starting point
		scroll_fg -= 0x170;
		if (scroll_fg & 0x8000)
			return;
		
		//Have we reached PASSED the ending point?
		if (scroll_fg >= 0x60)
			return;
		
		//Flashing
		uint16_t add;
		if (scroll_fg >= 0x50 || (scroll_fg & 8) == 0)
			add = 0x0000; //White
		else
			add = 0x2000; //Grey
		CopyTilemap_Add(ssrg_memory, 0xC704 + PLANE_WIDEADD, 35, 3, add);
	}
}

static void UpdateScrollPositions(Object *obj)
{
	ssrg_scroll_bg = obj->pos.s.x;
	vid_bg_scrpos_y_dup = -obj->pos.s.y;
}

//SSRG objects
static void SpeedToPosHud(Object *obj)
{
	uint32_t xadd = (int32_t)obj->xsp << 8;
	obj->pos.s.x += xadd >> 16;
	uint32_t y = ((obj->pos.s.y << 16) | obj->pos.s.yl) + (obj->ysp << 8);
	obj->pos.s.y = y >> 16;
	obj->pos.s.yl = y;
}

//Letters object
typedef struct
{
	uint16_t pad; //0x28-0x29
	uint16_t timer; //0x2A
} Scratch_Letters;

static void Obj_Letters(Object *obj)
{
	Scratch_Letters *scratch = (Scratch_Letters*)&obj->scratch;
	
	static const uint16_t data[4][8] = {
		//0xXXXX, 0xYYYY, 0xVRAM, 0xTIME, 0xXSPD, 0xYSPD, 0xMAPI, 0x????
		{0x0182,0x0180,0x0001,0x0001,0xFF00,0xF400,0x0000,0x0000},
		{0x01A8,0x0180,0x0001,0x0011,0xFF00,0xF400,0x0000,0x0000},
		{0x01D0,0x0180,0x0011,0x0021,0xFF00,0xF400,0x0000,0x0000},
		{0x01F8,0x0180,0x0021,0x0031,0xFF00,0xF400,0x0000,0x0000},
	};
	
	static const uint8_t mappings[] = {
		0x00,0x02,
		0x01,0xF0,0x0F,0x00,0x00,0xF0,
	};
	
	switch (obj->routine)
	{
		case 0: //Initialization
		{
			//Increment routine counter and read object data
			obj->routine += 2;
			
			const uint16_t *datap = data[obj->type - 1];
			obj->pos.s.x = *datap++ + PLANE_WIDEADD * 4;
			obj->pos.s.y = *datap++;
			
			obj->tile = *datap++;
			
			scratch->timer = *datap++;
			obj->xsp = *datap++;
			obj->ysp = *datap++;
			obj->frame = *datap++;
			
			//Set object drawing information
			obj->mappings = mappings;
			obj->priority = 0x20; //What the hell?
			obj->width_pixels = 32;
			
			obj->y_rad = 32;
		}
	//Fallthrough
		case 2: //Wait for timer to expire
			if (--scratch->timer == 0)
				obj->routine += 2;
			DisplaySprite(obj);
			break;
		case 4: //Fly up
			//Fall and draw
			SpeedToPosHud(obj);
			if ((obj->ysp += 0x40) >= 0 && 0xF0 < obj->pos.s.y)
				obj->routine += 2;
			DisplaySprite(obj);
			break;
		case 6: //Spring
			//Spring back upwards and draw
			SpeedToPosHud(obj);
			if ((obj->ysp -= 0x80) < 0 && 0xE8 >= obj->pos.s.y)
			{
				obj->ysp = 0;
				scratch->timer = 0;
				obj->routine += 2;
			}
			DisplaySprite(obj);
			break;
		case 8: //Slow down
			if (obj->xsp < 0)
			{
				//Slow down then draw
				SpeedToPosHud(obj);
				obj->xsp += 0x20;
				DisplaySprite(obj);
				break;
			}
			else if ((++scratch->timer) & 1)
			{
				//Handle flashing
				if ((obj->tile >> TILE_PALETTE_SHIFT) >= 2)
				{
					//Reset palette and state, increment routine
					obj->tile &= ~TILE_PALETTE_AND;
					obj->routine += 2;
					obj->xsp = 0;
					obj->ysp = 0;
					obj->pos.s.y &= 0x1FF; //Immediately overwritten
					obj->pos.s.y = 0xE4;
					DisplaySprite(obj);
					break;
				}
				else
				{
					//Increment palette
					obj->tile += TILE_MAP(0, 1, 0, 0, 0);
				}
			}
	//Fallthrough
		case 10: //Idle
			//Check if we've been knocked by the square
			SpeedToPosHud(obj);
			
			if (obj->pos.s.y != 0xE4)
			{
				if (obj->pos.s.y < 0xE4)
				{
					//Gravity
					obj->ysp += 0x80;
				}
				else
				{
					//Reset Y speed and position
					obj->ysp = 0;
					obj->pos.s.y = 0xE4;
				}
			}
			
			//Decrease X speed
			if (obj->xsp < 0)
				obj->xsp += 0x40;
			else if (obj->xsp > 0)
				obj->xsp -= 0x40;
			
			DisplaySprite(obj);
			break;
	}
}

//Square object
typedef struct
{
	uint16_t pad; //0x28-0x29
	uint16_t timer; //0x2A
	int16_t speed; //0x2C
} Scratch_Square;

static void Obj_Square(Object *obj)
{
	Scratch_Square *scratch = (Scratch_Square*)&obj->scratch;
	
	static const struct MapRamData
	{
		uint8_t *map;
		size_t offset;
		uint16_t width, height;
		uint32_t pad;
	} map_ram_data[] = {
		{&ssrg_memory[0x4000], MAP_PLANE(VRAM_BG, 2, 2), 0x000B, 0x000B, 0},
		{&ssrg_memory[0x4120], MAP_PLANE(VRAM_BG, 0, 0), 0x000F, 0x000F, 0},
		{&ssrg_memory[0x4320], MAP_PLANE(VRAM_BG, 0, 0), 0x0010, 0x0010, 0},
		{&ssrg_memory[0x4562], MAP_PLANE(VRAM_BG, 0, 0), 0x000F, 0x000F, 0},
	};
	
	switch (obj->routine)
	{
		case 0: //Initialization
			//Increment routine and initialize state
			obj->routine += 2;
			
			obj->pos.s.x = -0xC0;
			obj->pos.s.y = 0x20;
			scratch->timer = 0xA0 - (PLANE_WIDEADD / 2);
			obj->xsp = 0x800;
			obj->ysp = 0;
			
			UpdateScrollPositions(obj);
			return;
		case 2: //Wait for timer to expire
			if (--scratch->timer == 0)
			{
				obj->routine += 2;
				scratch->timer = (PLANE_WIDEADD / 2) * -2;
			}
			DisplaySprite(obj); //Null mappings, thank you Markey!
			return;
		case 4: //Play sound
			obj->routine += 2;
			//moveq	#0xFFFFFFBC,d0				; set to play spin release SFX
			//jsr	PlaySound_Special			; play SFX //TODO
	//Fallthrough
		case 6: //Spin in
			scratch->timer += 2;
			if (obj->pos.s.x >= 0x60 + (PLANE_WIDEADD * 4))
			{
				//Hit the 'SSRG' text
				obj->routine += 2;
				//moveq	#0xFFFFFFBD,d0				; set to play spiked chandelier SFX
				//jsr	PlaySound_Special			; play SFX //TODO
				obj->xsp = -0x100;
				obj->ysp = -0x400;
				
				//Knock 'SSRG' text
				objects[0].xsp = 0x300;
				objects[0].ysp = -0x400;
				objects[1].xsp = 0x300;
				objects[1].ysp = -0x300;
				objects[2].xsp = 0x300;
				objects[2].ysp = -0x200;
				objects[3].xsp = 0x300;
				objects[3].ysp = -0x100;
			}
			break;
		case 8: //Knocked back and landing
			scratch->timer -= 8;
			
			//Fall
			if ((obj->ysp += 0x20) >= 0 && obj->pos.s.y >= 0x20)
			{
				//Landed
				obj->pos.s.y = 0x20;
				obj->xsp = 0;
				obj->ysp = 0;
				obj->routine += 2;
				scratch->speed = 0xA40;
			}
			break;
		case 10: //Finish spinning
		{
			int16_t speed = scratch->speed;
			if (speed < 0)
			{
				//Invert palette brightness
				if (dry_palette[3][1] != 0xE0E)
					dry_palette[3][1] += 0x202;
				if (dry_palette[3][3] != 0x404)
					dry_palette[3][3] -= 0x202;
			}
			else
			{
				//Spin
				speed -= 0x18;
				scratch->timer -= speed >> 8;
				scratch->speed = speed;
			}
			break;
		}
	}
	
	//Move object and plane
	SpeedToPosHud(obj);
	UpdateScrollPositions(obj);
	
	//Clear previous plane art
	CopyTilemap_Single(0, VRAM_BG, 0x11, 0x11);
	
	//Copy new plane art
	const struct MapRamData *data = &map_ram_data[(scratch->timer & 0x18) >> 3];
	CopyTilemap(data->map, data->offset, data->width + 1, data->height + 1);
	
	#if (SCREEN_WIDTH > 320)
		//Clear unwanted tiles
		int16_t clip_tiles = -(obj->pos.s.x >> 3);
		if (clip_tiles > 0)
			CopyTilemap_Single(0, VRAM_BG, clip_tiles, 0x11);
	#endif
}

//Sonic neon object
typedef struct
{
	uint16_t pad; //0x28-0x29
	uint16_t timer; //0x2A
	uint16_t speed; //0x2C
} Scratch_SonicNeon;

static void Obj_SonicNeon(Object *obj)
{
	//For some reason this object is written completely differently to the others
	
	Scratch_SonicNeon *scratch = (Scratch_SonicNeon*)&obj->scratch;
	
	static const uint8_t mappings[] = {
		0x00,0x0c,
		0x00,0x21,
		0x00,0x2c,
		0x00,0x37,
		0x00,0x4c,
		0x00,0x57,
		0x04,
		0xeb,0x0d,0x00,0x00,0xec,
		0xfb,0x09,0x00,0x08,0xec,
		0xfb,0x06,0x00,0x0e,0x04,
		0x0b,0x04,0x00,0x14,0xec,
		0x02,
		0xec,0x0d,0x00,0x00,0xed,
		0xfc,0x0e,0x00,0x16,0xf5,
		0x02,
		0xed,0x09,0x00,0x22,0xf3,
		0xfd,0x0a,0x00,0x28,0xf3,
		0x04,
		0xeb,0x09,0x00,0x22,0xf4,
		0xfb,0x09,0x00,0x31,0xec,
		0xfb,0x06,0x00,0x37,0x04,
		0x0b,0x04,0x00,0x3d,0xec,
		0x02,
		0xec,0x09,0x00,0x22,0xf3,
		0xfc,0x0e,0x00,0x3f,0xeb,
		0x03,
		0xed,0x0d,0x00,0x00,0xec,
		0xfd,0x0c,0x00,0x4b,0xf4,
		0x05,0x09,0x00,0x4f,0xf4,
	};
	
	if (obj->routine == 0)
	{
		obj->type = 1; //'Engine doesn't like the ID being null'
		
		//Set object drawing information
		obj->tile = TILE_MAP(0, 3, 0, 0, 0x400);
		obj->mappings = mappings;
		
		//Initialize state
		obj->pos.s.x = 0;
		obj->pos.s.y = 0xDE;
		obj->priority = 0x20; //What the hell?
		
		obj->width_pixels = 32;
		obj->y_rad = 32;
		
		scratch->timer = 192 - PLANE_WIDEADD;
		obj->xsp = 0x400;
		
		//Increment routine
		obj->routine += 2;
	}
	
	//Wait for timer to expire
	if (scratch->timer != 0)
	{
		scratch->timer--;
		return;
	}
	
	//Check if we should stop
	if (obj->routine <= 2 && obj->pos.s.x >= 0xDE + (PLANE_WIDEADD * 4))
	{
		//Stop running and increment routine
		obj->pos.s.x = 0xDE + (PLANE_WIDEADD * 4);
		obj->xsp = 0;
		//moveq	#$FFFFFFBE,d0				; set to play spinning SFX
		//jsr	PlaySound_Special			; play SFX //TODO
		obj->routine += 2;
	}
	
	//Update mapping frame
	scratch->speed += 0x40;
	
	uint8_t frame = scratch->speed >> 8;
	if (frame >= 6)
	{
		frame = 0;
		scratch->speed = 0;
	}
	
	obj->frame = frame;
	
	//Palette cycle
	if (!(ssrg_scroll_fg & 0x7))
	{
		uint16_t *fromp = &dry_palette[3][4];
		uint16_t *top = fromp;
		uint16_t temp = *fromp++;
		*top++ = *fromp++;
		*top++ = *fromp++;
		*top++ = *fromp++;
		*top = temp;
	}
	
	//Move and draw
	SpeedToPosHud(obj);
	DisplaySprite(obj);
}

//SSRG splash game mode
void GM_SSRG()
{
	//moveq	#$FFFFFFE4,d0				; set music ID to "stop music"
	//jsr	PlaySound_Special			; play ID //TODO
	
	//Clear the pattern load queue and fade out
	ClearPLC();
	PaletteFadeOut();
	
	//Clear screen
	ClearScreen();
	
	//Clear object memory
	memset(objects, 0, sizeof(objects));
	
	//Initialize VDP and video state
	VDP_SetBackgroundColour(0);
	vid_scrpos_y_dup = -8;
	vid_bg_scrpos_y_dup = -44;
	
	//Decompress art into VRAM
	VDP_SeekVRAM(0x0020);
	NemDec(art_main);
	VDP_SeekVRAM(0x4000);
	NemDec(art_square);
	VDP_SeekVRAM(0x8000);
	NemDec(art_sonic);
	VDP_SeekVRAM(0x9000);
	NemDec(art_link);
	
	//Decompress mappings
	KosDec(map_link, ssrg_memory);
	CopyTilemap(ssrg_memory, MAP_PLANE(VRAM_FG, 4, 24) + PLANE_WIDEADD + (PLANE_TALLADD * 2), 32, 1);
	
	KosDec(map_main, &ssrg_memory[0x0000]);
	KosDec(map_square, &ssrg_memory[0x4000]);
	
	//Copy palette
	memcpy(&dry_palette_dup[0][0], pal_ssrg, sizeof(pal_ssrg));
	
	//Load objects and fade in
	objects[0].type = 1; //S
	objects[1].type = 2; //S
	objects[2].type = 3; //R
	objects[3].type = 4; //G
	PaletteFadeIn();
	memset(&ssrg_memory[0x7800], 0, 4*3);
	
	//Run loop
	do
	{
		//Render frame
		vbla_routine = 0x04;
		WaitForVBla();
		ssrg_scroll_fg++;
		
		//Run objects
		Object *objectp = objects;
		Obj_Letters(objectp++); //S
		Obj_Letters(objectp++); //S
		Obj_Letters(objectp++); //R
		Obj_Letters(objectp++); //G
		Obj_Square(objectp++); //Square
		Obj_SonicNeon(&objects[5]); //Neon Sonic (not sure why it directly addresses here)
		
		//Draw screen
		SRG_ScrollFG();
		SRG_DrawFG();
		BuildSprites();
	} while (!(jpad1_press1 & JPAD_START) && ssrg_scroll_fg < 0x200);
	
	//Go to title gamemode
	gamemode = GameMode_Title;
}
