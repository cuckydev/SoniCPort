#pragma once

#include "Object.h"

//Collision angle buffer
extern uint8_t angle_buffer0, angle_buffer1;

//Level collision interface
void FloorLog_Unk();
const uint8_t *FindNearestTile(Object *obj, int16_t x, int16_t y);
int16_t FindFloor(Object *obj, int16_t x, int16_t y, uint16_t solid, uint16_t flip, int16_t inc, uint8_t *angle);
int16_t FindWall(Object *obj, int16_t x, int16_t y, uint16_t solid, uint16_t flip, int16_t inc, uint8_t *angle);

//Object collision functions
int16_t GetDistance2_Down(Object *obj, int16_t x, int16_t y, uint8_t *hit_angle);
int16_t GetDistance2_Left(Object *obj, int16_t x, int16_t y, uint8_t *hit_angle);
int16_t GetDistance2_Up(Object *obj, int16_t x, int16_t y, uint8_t *hit_angle);
int16_t GetDistance2_Right(Object *obj, int16_t x, int16_t y, uint8_t *hit_angle);
int16_t GetDistanceBelowAngle2(Object *obj, uint8_t angle, uint8_t *hit_angle);

int16_t GetDistance_Down(Object *obj, uint8_t *hit_angle);
int16_t GetDistance_Left(Object *obj, uint8_t *hit_angle);
int16_t GetDistance_Up(Object *obj, uint8_t *hit_angle);
int16_t GetDistance_Right(Object *obj, uint8_t *hit_angle);
int16_t GetDistanceBelowAngle(Object *obj, uint8_t angle, uint8_t *hit_angle);

int16_t ObjFloorDist(Object *obj, int16_t x);
