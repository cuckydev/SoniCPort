#pragma once

#include <stdint.h>

//Joypad bitmask
#define JPAD_START (1 << 7)
#define JPAD_A     (1 << 6)
#define JPAD_C     (1 << 5)
#define JPAD_B     (1 << 4)
#define JPAD_RIGHT (1 << 3)
#define JPAD_LEFT  (1 << 2)
#define JPAD_DOWN  (1 << 1)
#define JPAD_UP    (1 << 0)

//Joupad interface
uint8_t Joypad_GetState1();
uint8_t Joypad_GetState2();
