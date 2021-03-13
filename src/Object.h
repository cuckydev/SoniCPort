#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <Types.h>
#include <Backend/VDP.h>

//Object constants
#define RESERVED_OBJECTS 0x20
#define LEVEL_OBJECTS    0x60
#define OBJECTS          (RESERVED_OBJECTS + LEVEL_OBJECTS)

//Object IDs
typedef enum
{
	ObjId_Null,         //00
	ObjId_Sonic,        //01
	ObjId_02,           //02
	ObjId_03,           //03
	ObjId_04,           //04
	ObjId_05,           //05
	ObjId_06,           //06
	ObjId_07,           //07
	ObjId_08,           //08
	ObjId_09,           //09
	ObjId_0A,           //0A
	ObjId_0B,           //0B
	ObjId_0C,           //0C
	ObjId_0D,           //0D
	ObjId_TitleSonic,   //0E
	ObjId_PSB,          //0F
	ObjId_10,           //10
	ObjId_GHZBridge,    //11
	ObjId_12,           //12
	ObjId_13,           //13
	ObjId_14,           //14
	ObjId_15,           //15
	ObjId_16,           //16
	ObjId_17,           //17
	ObjId_18,           //18
	ObjId_19,           //19
	ObjId_1A,           //1A
	ObjId_1B,           //1B
	ObjId_1C,           //1C
	ObjId_1D,           //1D
	ObjId_1E,           //1E
	ObjId_Crabmeat,     //1F
	ObjId_20,           //20
	ObjId_21,           //21
	ObjId_BuzzBomber,   //22
	ObjId_BuzzMissile,  //23
	ObjId_BuzzExplode,  //24
	ObjId_Ring,         //25
	ObjId_26,           //26
	ObjId_Explosion,    //27
	ObjId_Animal,       //28
	ObjId_29,           //29
	ObjId_2A,           //2A
	ObjId_Chopper,      //2B
	ObjId_2C,           //2C
	ObjId_2D,           //2D
	ObjId_2E,           //2E
	ObjId_2F,           //2F
	ObjId_30,           //30
	ObjId_31,           //31
	ObjId_32,           //32
	ObjId_33,           //33
	ObjId_TitleCard,    //34
	ObjId_35,           //35
	ObjId_36,           //36
	ObjId_RingLoss,     //37
	ObjId_38,           //38
	ObjId_GameOverCard, //39
	ObjId_3A,           //3A
	ObjId_GHZRock,      //3B
	ObjId_3C,           //3C
	ObjId_3D,           //3D
	ObjId_3E,           //3E
	ObjId_3F,           //3F
	ObjId_Motobug,      //40
	ObjId_Spring,       //41
	ObjId_Newtron,      //42
	ObjId_43,           //43
	ObjId_GHZEdge,      //44
	ObjId_45,           //45
	ObjId_46,           //46
	ObjId_47,           //47
	ObjId_48,           //48
	ObjId_49,           //49
	ObjId_4A,           //4A
	ObjId_4B,           //4B
	ObjId_4C,           //4C
	ObjId_4D,           //4D
	ObjId_4E,           //4E
	ObjId_4F,           //4F
	ObjId_50,           //50
	ObjId_51,           //51
	ObjId_52,           //52
	ObjId_53,           //53
	ObjId_54,           //54
	ObjId_55,           //55
	ObjId_56,           //56
	ObjId_57,           //57
	ObjId_58,           //58
	ObjId_59,           //59
	ObjId_5A,           //5A
	ObjId_5B,           //5B
	ObjId_5C,           //5C
	ObjId_5D,           //5D
	ObjId_5E,           //5E
	ObjId_5F,           //5F
	ObjId_60,           //60
	ObjId_61,           //61
	ObjId_62,           //62
	ObjId_63,           //63
	ObjId_64,           //64
	ObjId_65,           //65
	ObjId_66,           //66
	ObjId_67,           //67
	ObjId_68,           //68
	ObjId_69,           //69
	ObjId_6A,           //6A
	ObjId_6B,           //6B
	ObjId_6C,           //6C
	ObjId_6D,           //6D
	ObjId_6E,           //6E
	ObjId_6F,           //6F
	ObjId_70,           //70
	ObjId_71,           //71
	ObjId_72,           //72
	ObjId_73,           //73
	ObjId_74,           //74
	ObjId_75,           //75
	ObjId_76,           //76
	ObjId_77,           //77
	ObjId_78,           //78
	ObjId_79,           //79
	ObjId_7A,           //7A
	ObjId_7B,           //7B
	ObjId_7C,           //7C
	ObjId_7D,           //7D
	ObjId_7E,           //7E
	ObjId_7F,           //7F
	ObjId_80,           //80
	ObjId_81,           //81
	ObjId_82,           //82
	ObjId_83,           //83
	ObjId_84,           //84
	ObjId_85,           //85
	ObjId_86,           //86
	ObjId_87,           //87
	ObjId_88,           //88
	ObjId_89,           //89
	ObjId_Credits,      //8A
	ObjId_8B,           //8B
	ObjId_8C,           //8C
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
void BuildSprites();
void AnimateSprite(Object *obj, const uint8_t *anim_script);
void DisplaySprite(Object *obj);
void ObjectDelete(Object *obj);
void SpeedToPos(Object *obj);
void ObjectFall(Object *obj);
void RememberState(Object *obj);
void MvSonicOnPtfm(Object *obj, int16_t y, int16_t prev_x);
void PlatformObject(Object *obj, uint16_t x_rad);
void Platform3(Object *obj, int16_t top);
bool ExitPlatform(Object *obj, uint16_t x_rad, uint16_t x_dia2, int16_t *x_off_p);
int SolidObject(Object *obj, uint16_t x_rad, uint16_t y_rad1, uint16_t y_rad2, int16_t prev_x, int16_t *x_off, int16_t *y_off);
