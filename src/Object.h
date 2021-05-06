#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "Types.h"
#include "Backend/VDP.h"

//Object constants
#define RESERVED_OBJECTS 0x20
#define LEVEL_OBJECTS    0x60
#define OBJECTS          (RESERVED_OBJECTS + LEVEL_OBJECTS)

//Object IDs
typedef enum
{
	/*00*/ ObjId_Null,
	/*01*/ ObjId_Sonic,
	/*02*/ ObjId_02,
	/*03*/ ObjId_03,
	/*04*/ ObjId_04,
	/*05*/ ObjId_05,
	/*06*/ ObjId_06,
	/*07*/ ObjId_07,
	/*08*/ ObjId_08,
	/*09*/ ObjId_SpecialSonic,
	/*0A*/ ObjId_0A,
	/*0B*/ ObjId_0B,
	/*0C*/ ObjId_0C,
	/*0D*/ ObjId_Signpost,
	/*0E*/ ObjId_TitleSonic,
	/*0F*/ ObjId_PSB,
	/*10*/ ObjId_10,
	/*11*/ ObjId_GHZBridge,
	/*12*/ ObjId_12,
	/*13*/ ObjId_13,
	/*14*/ ObjId_14,
	/*15*/ ObjId_15,
	/*16*/ ObjId_16,
	/*17*/ ObjId_17,
	/*18*/ ObjId_18,
	/*19*/ ObjId_19,
	/*1A*/ ObjId_1A,
	/*1B*/ ObjId_1B,
	/*1C*/ ObjId_1C,
	/*1D*/ ObjId_1D,
	/*1E*/ ObjId_1E,
	/*1F*/ ObjId_Crabmeat,
	/*20*/ ObjId_20,
	/*21*/ ObjId_HUD,
	/*22*/ ObjId_BuzzBomber,
	/*23*/ ObjId_BuzzMissile,
	/*24*/ ObjId_BuzzExplode,
	/*25*/ ObjId_Ring,
	/*26*/ ObjId_Monitor,
	/*27*/ ObjId_Explosion,
	/*28*/ ObjId_Animal,
	/*29*/ ObjId_29,
	/*2A*/ ObjId_2A,
	/*2B*/ ObjId_Chopper,
	/*2C*/ ObjId_2C,
	/*2D*/ ObjId_2D,
	/*2E*/ ObjId_MonitorItem,
	/*2F*/ ObjId_2F,
	/*30*/ ObjId_30,
	/*31*/ ObjId_31,
	/*32*/ ObjId_32,
	/*33*/ ObjId_33,
	/*34*/ ObjId_TitleCard,
	/*35*/ ObjId_35,
	/*36*/ ObjId_Spikes,
	/*37*/ ObjId_RingLoss,
	/*38*/ ObjId_ShieldInvincibility,
	/*39*/ ObjId_GameOverCard,
	/*3A*/ ObjId_GotThroughCard,
	/*3B*/ ObjId_GHZRock,
	/*3C*/ ObjId_3C,
	/*3D*/ ObjId_3D,
	/*3E*/ ObjId_3E,
	/*3F*/ ObjId_3F,
	/*40*/ ObjId_Motobug,
	/*41*/ ObjId_Spring,
	/*42*/ ObjId_Newtron,
	/*43*/ ObjId_43,
	/*44*/ ObjId_GHZEdge,
	/*45*/ ObjId_45,
	/*46*/ ObjId_46,
	/*47*/ ObjId_Bumper,
	/*48*/ ObjId_48,
	/*49*/ ObjId_49,
	/*4A*/ ObjId_4A,
	/*4B*/ ObjId_4B,
	/*4C*/ ObjId_4C,
	/*4D*/ ObjId_4D,
	/*4E*/ ObjId_4E,
	/*4F*/ ObjId_4F,
	/*50*/ ObjId_50,
	/*51*/ ObjId_51,
	/*52*/ ObjId_52,
	/*53*/ ObjId_53,
	/*54*/ ObjId_54,
	/*55*/ ObjId_55,
	/*56*/ ObjId_56,
	/*57*/ ObjId_57,
	/*58*/ ObjId_58,
	/*59*/ ObjId_59,
	/*5A*/ ObjId_5A,
	/*5B*/ ObjId_5B,
	/*5C*/ ObjId_5C,
	/*5D*/ ObjId_5D,
	/*5E*/ ObjId_5E,
	/*5F*/ ObjId_5F,
	/*60*/ ObjId_60,
	/*61*/ ObjId_61,
	/*62*/ ObjId_62,
	/*63*/ ObjId_63,
	/*64*/ ObjId_64,
	/*65*/ ObjId_65,
	/*66*/ ObjId_66,
	/*67*/ ObjId_67,
	/*68*/ ObjId_68,
	/*69*/ ObjId_69,
	/*6A*/ ObjId_6A,
	/*6B*/ ObjId_6B,
	/*6C*/ ObjId_6C,
	/*6D*/ ObjId_6D,
	/*6E*/ ObjId_6E,
	/*6F*/ ObjId_6F,
	/*70*/ ObjId_70,
	/*71*/ ObjId_71,
	/*72*/ ObjId_72,
	/*73*/ ObjId_73,
	/*74*/ ObjId_74,
	/*75*/ ObjId_75,
	/*76*/ ObjId_76,
	/*77*/ ObjId_77,
	/*78*/ ObjId_78,
	/*79*/ ObjId_79,
	/*7A*/ ObjId_7A,
	/*7B*/ ObjId_7B,
	/*7C*/ ObjId_7C,
	/*7D*/ ObjId_7D,
	/*7E*/ ObjId_7E,
	/*7F*/ ObjId_7F,
	/*80*/ ObjId_80,
	/*81*/ ObjId_81,
	/*82*/ ObjId_82,
	/*83*/ ObjId_83,
	/*84*/ ObjId_84,
	/*85*/ ObjId_85,
	/*86*/ ObjId_86,
	/*87*/ ObjId_87,
	/*88*/ ObjId_88,
	/*89*/ ObjId_89,
	/*8A*/ ObjId_Credits,
	/*8B*/ ObjId_8B,
	/*8C*/ ObjId_8C,
} ObjectId;

//Object types
#pragma pack(push)
#pragma pack(1)

typedef union
{
	struct
	{
		unsigned int x_flip : 1;       //Horizontally flipped
		unsigned int y_flip : 1;       //Vertically flipped
		unsigned int align_fg : 1;     //Aligned to the foreground
		unsigned int align_bg : 1;     //Aligned to the background (overrides `align_fg`)
		unsigned int yrad_height : 1;  //Use y_rad as cull height instead of 32
		unsigned int raw_mappings : 1; //`mappings` member points to a single mapping
		unsigned int player_loop : 1;  //Set if we're the player object and behind a loop
		unsigned int on_screen : 1;    //Set if the object's on-screen (see BuildSprites)
	} f;
	uint8_t b;
} ObjectRender;

typedef union
{
	struct
	{
		unsigned int x_flip : 1;       //Horizontally flipped
		unsigned int y_flip : 1;       //Vertially flipped
		unsigned int flag2 : 1;        //Unused
		unsigned int player_stand : 1; //Player is standing on us
		unsigned int flag4 : 1;        //Unused
		unsigned int player_push : 1;  //Player is pushing us
		unsigned int flag6 : 1;        //Unused
		unsigned int flag7 : 1;        //Object-specific
	} f;
	uint8_t b;
} ObjectStatus;

typedef union
{
	struct
	{
		unsigned int x_flip : 1;       //Horizontally flipped
		unsigned int in_air : 1;       //In mid-air
		unsigned int in_ball : 1;      //In ball-form
		unsigned int object_stand : 1; //Standing on an object
		unsigned int roll_jump : 1;    //Set when jumping from a roll
		unsigned int pushing : 1;      //Set if we're pushing
		unsigned int underwater : 1;   //Set if we're underwater
		unsigned int flag7 : 1;        //Unused
	} f;
	uint8_t b;
} PlayerStatus;

#pragma pack(pop)

typedef struct
{
	uint8_t type;            //Object type
	ObjectRender render;     //Object render
	uint16_t tile;           //Object base tile
	const uint8_t *mappings; //Object mappings
	union
	{
		struct
		{
			dword_s x, y;
		} l; //Level
		struct
		{
			int16_t x; //X position
			int16_t y; //Y position
			uint16_t yl; //Y position (lower word for long accesses)
		} s; //Screen (VDP coordinates)
	} pos;                //Position
	int16_t xsp;          //Horizontal speed
	int16_t ysp;          //Vertical speed
	int16_t inertia;      //Speed rotated by angle
	int8_t x_rad, y_rad;  //Object radius
	uint8_t priority;     //Sprite priority (0-7, 0 drawn in front of 7)
	uint8_t width_pixels; //Culling and platform width of sprite
	uint8_t frame;        //Mapping frame
	uint8_t anim_frame;   //Frame index in animation
	uint8_t anim;         //Animation
	uint8_t prev_anim;    //Previous animation
	union
	{
		int8_t b;
		int16_t w;
	} frame_time;         //Frame duration remaining
	uint8_t col_type;     //Collision type
	uint8_t col_property; //Collision property (object-specific)
	union
	{
		ObjectStatus o;   //Object status
		PlayerStatus p;   //Player status
		uint8_t b;
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

//Object globals
extern int ExecuteObjects_i;

//Object functions
Object *FindFreeObj();
Object *FindNextFreeObj(Object *obj);
void ExecuteObjects();

void BuildSpr_Normal(uint16_t **sprite, uint8_t *sprite_i, uint16_t x, uint16_t y, uint16_t tile, const uint8_t *mappings, uint8_t pieces);
void BuildSprites(uint8_t *sprite_io);

void AnimateSprite(Object *obj, const uint8_t *anim_script);
void DisplaySprite(Object *obj);

void ObjectDelete(Object *obj);
void SpeedToPos(Object *obj);
void ObjectFall(Object *obj);

void RememberState(Object *obj);
void MvSonicOnPtfm(Object *obj, int16_t y, int16_t prev_x);
void PlatformObject(Object *obj, uint16_t x_rad);
void Platform3(Object *obj, int16_t top);
void Platform_SetStand(Object *obj);
bool ExitPlatform(Object *obj, uint16_t x_rad, uint16_t x_rad2, int16_t *x_off_p);
signed int SolidObject(Object *obj, uint16_t x_rad, uint16_t y_rad1, uint16_t y_rad2, int16_t prev_x, int16_t *x_off, int16_t *y_off);
