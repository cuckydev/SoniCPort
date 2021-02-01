#pragma once

#include "Object.h"

#include <stdint.h>

void FloorLog_Unk();
const uint8_t *FindNearestTile(Object *obj, int16_t x, int16_t y);
int16_t FindFloor(Object *obj, int16_t x, int16_t y, uint16_t solid, uint16_t flip, int16_t inc, uint8_t *angle);
int16_t FindWall(Object *obj, int16_t x, int16_t y, uint16_t solid, uint16_t flip, int16_t inc, uint8_t *angle);
