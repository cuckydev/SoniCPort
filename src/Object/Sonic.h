#pragma once

#include <stdint.h>
#include <Types.h>

//Sonic globals
extern int16_t sonspeed_max, sonspeed_acc, sonspeed_dec;
extern uint8_t sonframe_num, sonframe_chg;

extern uint8_t angle_buffer0;
extern uint8_t angle_buffer1;

extern int16_t track_sonic[0x40][2];
extern word_u track_pos;

//Sonic types
typedef enum
{
	SonAnimId_Walk,
	SonAnimId_Run,
	SonAnimId_Roll,
	SonAnimId_Roll2,
	SonAnimId_Push,
	SonAnimId_Wait,
	SonAnimId_Balance,
	SonAnimId_LookUp,
	SonAnimId_Duck,
	SonAnimId_Warp1,
	SonAnimId_Warp2,
	SonAnimId_Warp3,
	SonAnimId_Warp4,
	SonAnimId_Stop,
	SonAnimId_Float1,
	SonAnimId_Float2,
	SonAnimId_Spring,
	SonAnimId_Hang,
	SonAnimId_Leap1,
	SonAnimId_Leap2,
	SonAnimId_Surf,
	SonAnimId_GetAir,
	SonAnimId_Burnt,
	SonAnimId_Drown,
	SonAnimId_Death,
	SonAnimId_Shrink,
	SonAnimId_Hurt,
	SonAnimId_WaterSlid,
	SonAnimId_Null,
	SonAnimId_Float3,
	SonAnimId_Float4,
} SonAnimId;
