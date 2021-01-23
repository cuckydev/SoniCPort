#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <Types.h>
#include <Backend/VDP.h>

//Object IDs
typedef enum
{
	ObjId_Null,       //00
	ObjId_Sonic,      //01
	ObjId_02,         //02
	ObjId_03,         //03
	ObjId_04,         //04
	ObjId_05,         //05
	ObjId_06,         //06
	ObjId_07,         //07
	ObjId_08,         //08
	ObjId_09,         //09
	ObjId_0A,         //0A
	ObjId_0B,         //0B
	ObjId_0C,         //0C
	ObjId_0D,         //0D
	ObjId_TitleSonic, //0E
	ObjId_0F,         //0F
} ObjectId;

//Object types
#pragma pack(push)
#pragma pack(1)

typedef union
{
	struct
	{
		uint8_t on_screen : 1;     //Set if the object's on-screen (see BuildSprites)
		uint8_t player_loop : 1;   //Set if we're the player object and behind a loop
		uint8_t raw_mappings : 1;  //`mappings` member points to a single mapping
		uint8_t assume_height : 1; //Cull height is assumed to be 32
		uint8_t align_bg : 1;      //Aligned to the background (overrides `align_fg`)
		uint8_t align_fg : 1;      //Aligned to the foreground
		uint8_t y_flip : 1;        //Vertically flipped
		uint8_t x_flip : 1;        //Horizontally flipped
	} f;
	uint8_t b;
} ObjectRender;

typedef union
{
	struct
	{
		uint8_t flag7 : 1;        //Object-specific
		uint8_t flag6 : 1;        //Unused
		uint8_t player_push : 1;  //Player is pushing us
		uint8_t flag5 : 1;        //Unused
		uint8_t player_stand : 1; //Player is standing on us
		uint8_t y_flip : 1;       //Vertially flipped
		uint8_t x_flip : 1;       //Horizontally flipped
	} f;
	uint8_t b;
} ObjectStatus;

typedef union
{
	struct
	{
		uint8_t flag7 : 1;        //Unused
		uint8_t underwater : 1;   //Set if we're underwater
		uint8_t pushing : 1;      //Set if we're pushing
		uint8_t roll_jump : 1;    //Set when jumping from a roll
		uint8_t object_stand : 1; //Standing on an object
		uint8_t in_ball : 1;      //In ball-form
		uint8_t in_air : 1;       //In mid-air
		uint8_t x_flip : 1;       //Horizontally flipped
	} f;
	uint8_t b;
} PlayerStatus;

#pragma pack(pop)

typedef struct
{
	uint8_t type;            //Object type
	ObjectRender render;     //Object render
	VDP_Tile tile;           //Object base tile
	const uint8_t *mappings; //Object mappings
	union
	{
		struct
		{
			dword_s x, y;
		} l; //Level
		struct
		{
			uint16_t x; //X position
			uint16_t y; //Y position
		} s; //Screen (VDP coordinates)
	} pos;                //Position
	int16_t xsp;          //Horizontal speed
	int16_t ysp;          //Vertical speed
	int16_t inertia;      //Speed rotated by angle
	uint8_t x_rad, y_rad; //Object radius
	uint8_t priority;     //Sprite priority (0-7, 0 drawn in front of 7)
	uint8_t width_pixels; //Culling and platform width of sprite
	uint8_t frame;        //Mapping frame
	uint8_t anim_frame;   //Frame index in animation
	uint8_t anim;         //Animation
	uint8_t prev_anim;    //Previous animation
	int8_t frame_time;    //Frame duration remaining
	uint8_t col_response; //Collision response
	uint8_t col_property; //Collision property (object-specific)
	union
	{
		ObjectStatus o;   //Object status
		PlayerStatus p;   //Player status
	} status;
	uint8_t respawn_index; //Respawn index reference number
	uint8_t routine;       //Routine
	uint8_t routine_sec;   //Secondary routine
	uint8_t angle;         //Angle
	union
	{
		uint8_t  u8[0x18];
		int8_t   s8[0x18];
		uint16_t u16[0xC];
		int16_t  s16[0xC];
		uint32_t u32[0x6];
		int32_t  s32[0x6];
	} scratch;             //Scratch memory
} Object;

//Object functions
void ExecuteObjects();
void BuildSprites();
void AnimateSprite(Object *obj, const uint8_t *anim_script);
void DisplaySprite(Object *obj);
