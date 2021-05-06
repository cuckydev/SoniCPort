#pragma once

#include "Types.h"

//Special Stage state
extern word_u ss_angle;
extern uint16_t ss_rotate;
extern uint16_t palss_num, palss_time;

extern uint8_t last_special;

extern uint8_t emeralds;
extern uint8_t emerald_list[8];

//Special Stage functions
void SS_ShowLayout(uint8_t sprite_i);
void SS_Load();
