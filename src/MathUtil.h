#pragma once

#include "Types.h"

//Random seed
extern dword_u random_seed;

//Math utility functions
void CalcSine(uint8_t angle, int16_t *sin, int16_t *cos);
int16_t GetSin(uint8_t angle);
int16_t GetCos(uint8_t angle);
uint16_t CalcAngle(int16_t x, int16_t y);
uint32_t RandomNumber();
