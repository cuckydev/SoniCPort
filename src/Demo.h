#pragma once

#include <stdint.h>

//Demo state
extern uint16_t btn_pushtime1;
extern uint8_t btn_pushtime2;

//Demos
extern const uint8_t *intro_demo_ptr[];
extern const uint8_t *ending_demo_ptr[];

//Demo playback
void MoveSonicInDemo();
