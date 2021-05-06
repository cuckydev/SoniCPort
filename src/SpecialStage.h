#pragma once

#include "Types.h"

//Special Stage constants
#define SS_SRCDIM 64
#define SS_DIM 128
#define SS_PAD (SS_DIM - SS_SRCDIM)
#define SS_PAD2 (SS_PAD >> 1)

//Special Stage state
extern word_u ss_angle;
extern uint16_t ss_rotate;
extern uint16_t palss_num, palss_time;

extern uint8_t last_special;

extern uint8_t emeralds;
extern uint8_t emerald_list[8];

extern uint8_t ss_layout[SS_DIM * SS_DIM];

//Special Stage functions
void SS_ShowLayout(uint8_t sprite_i);
void SS_Load();
