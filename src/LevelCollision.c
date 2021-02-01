#include "LevelCollision.h"

#include "Level.h"

//Collision maps
static const uint8_t angle_map[] = {
	#include <Resource/Collision/AngleMap.h>
};

static const uint8_t height_map[] = {
	#include <Resource/Collision/HeightMap.h>
};

static const uint8_t width_map[] = {
	#include <Resource/Collision/WidthMap.h>
};

//Collision interface
void FloorLog_Unk()
{
	//Some debug function
}

const uint8_t *FindNearestTile(Object *obj, int16_t x, int16_t y)
{
	//Get chunk position
	int16_t cx = x >> 8;
	int16_t cy = y >> 8;
	
	if (cx < 0 || cy < 0 || cx >= 0x40 || cy >= 8)
		return level_map256;
	
	//Get chunk
	uint8_t chunk = level_layout[cy][0][cx];
	if (chunk == 0)
		return level_map256;
	
	if (!(chunk & 0x80))
	{
		//Return chunk
		uint8_t tx = (x >> 4) & 0xF;
		uint8_t ty = (y >> 4) & 0xF;
		return (level_map256 - 0x200) + (chunk << 9) + (ty << 5) + (tx << 1);
	}
	else
	{
		//Get chunk id
		chunk &= 0x7F;
		if (obj->render.f.player_loop)
		{
			if (chunk == 0x29)
				chunk = 0x51;
		}
		
		//Return chunk
		uint8_t tx = (x >> 4) & 0xF;
		uint8_t ty = (y >> 4) & 0xF;
		return (level_map256 - 0x200) + (chunk << 9) + (ty << 5) + (tx << 1);
	}
}

static int16_t FindFloor2(Object *obj, int16_t x, int16_t y, uint16_t solid, uint16_t flip, uint8_t *angle)
{
	//Check tile at given position
	const uint8_t *tile = FindNearestTile(obj, x, y);
	uint16_t tilev = (tile[0] << 8) | (tile[1] << 0);
	
	uint16_t tilei = tilev & META_TILE;
	
	if (tilei != 0 && (tilev & solid))
	{
		//Get collision tile
		uint16_t ctile = coll_index[tilei];
		if (ctile != 0)
		{
			//Get angle and height map index
			if (angle != NULL)
				*angle = angle_map[ctile];
			ctile <<= 4;
			
			uint8_t ind_x = x & 0xF;
			if (tilev & META_X_FLIP)
			{
				ind_x ^= 0xF;
				if (angle != NULL)
					*angle = -*angle;
			}
			if (tilev & META_Y_FLIP)
			{
				if (angle != NULL)
					*angle = (-(*angle + 0x40)) - 0x40;
			}
			
			//Get height
			int16_t height = (int8_t)height_map[ctile + ind_x];
			if ((tilev ^= flip) & META_Y_FLIP)
				height = -height;
			
			//Handle hit tile
			if (height > 0)
			{
				//Clip to floor
				return 0xF - (height + (y & 0xF));
			}
			else if (height < 0)
			{
				//Clip to ceiling?
				if ((height += (y & 0xF)) < 0)
					return -height;
			}
		}
	}
	
	//No tile found
	return 0xF - (y & 0xF);
}

int16_t FindFloor(Object *obj, int16_t x, int16_t y, uint16_t solid, uint16_t flip, int16_t inc, uint8_t *angle)
{
	//Check tile at given position
	const uint8_t *tile = FindNearestTile(obj, x, y);
	uint16_t tilev = (tile[0] << 8) | (tile[1] << 0);
	
	uint16_t tilei = tilev & META_TILE;
	
	if (tilei != 0 && (tilev & solid))
	{
		//Get collision tile
		uint16_t ctile = coll_index[tilei];
		if (ctile != 0)
		{
			//Get angle and height map index
			if (angle != NULL)
				*angle = angle_map[ctile];
			ctile <<= 4;
			
			uint8_t ind_x = x & 0xF;
			if (tilev & META_X_FLIP)
			{
				ind_x ^= 0xF;
				if (angle != NULL)
					*angle = -*angle;
			}
			if (tilev & META_Y_FLIP)
			{
				if (angle != NULL)
					*angle = (-(*angle + 0x40)) - 0x40;
			}
			
			//Get height
			int16_t height = (int8_t)height_map[ctile + ind_x];
			if ((tilev ^= flip) & META_Y_FLIP)
				height = -height;
			
			//Handle hit tile
			if (height > 0)
			{
				if (height != 0x10)
				{
					//Clip to floor
					return 0xF - (height + (y & 0xF));
				}
				else
				{
					//Check tile above
					return FindFloor2(obj, x, y - inc, solid, flip, angle) - 0x10;
				}
			}
			else
			{
				//Check tile above
				if ((height += (y & 0xF)) < 0)
					return FindFloor2(obj, x, y - inc, solid, flip, angle) - 0x10;
			}
		}
	}
	
	//No tile found, check tile below
	return FindFloor2(obj, x, y + inc, solid, flip, angle) + 0x10;
}

static int16_t FindWall2(Object *obj, int16_t x, int16_t y, uint16_t solid, uint16_t flip, uint8_t *angle)
{
	//Check tile at given position
	const uint8_t *tile = FindNearestTile(obj, x, y);
	uint16_t tilev = (tile[0] << 8) | (tile[1] << 0);
	
	uint16_t tilei = tilev & META_TILE;
	
	if (tilei != 0 && (tilev & solid))
	{
		//Get collision tile
		uint16_t ctile = coll_index[tilei];
		if (ctile != 0)
		{
			//Get angle and width map index
			if (angle != NULL)
				*angle = angle_map[ctile];
			ctile <<= 4;
			
			uint8_t ind_y = y & 0xF;
			if (tilev & META_Y_FLIP)
			{
				ind_y ^= 0xF;
				if (angle != NULL)
					*angle = (-(*angle + 0x40)) - 0x40;
			}
			if (tilev & META_X_FLIP)
			{
				if (angle != NULL)
					*angle = -*angle;
			}
			
			//Get width
			int16_t width = (int8_t)width_map[ctile + ind_y];
			if ((tilev ^= flip) & META_X_FLIP)
				width = -width;
			
			//Handle hit tile
			if (width > 0)
			{
				//Clip to wall
				return 0xF - (width + (x & 0xF));
			}
			else if (width < 0)
			{
				//Clip to other wall
				if ((width += (x & 0xF)) < 0)
					return -width;
			}
		}
	}
	
	//No tile found
	return 0xF - (x & 0xF);
}

int16_t FindWall(Object *obj, int16_t x, int16_t y, uint16_t solid, uint16_t flip, int16_t inc, uint8_t *angle)
{
	//Check tile at given position
	const uint8_t *tile = FindNearestTile(obj, x, y);
	uint16_t tilev = (tile[0] << 8) | (tile[1] << 0);
	
	uint16_t tilei = tilev & META_TILE;
	
	if (tilei != 0 && (tilev & solid))
	{
		//Get collision tile
		uint16_t ctile = coll_index[tilei];
		if (ctile != 0)
		{
			//Get angle and width map index
			if (angle != NULL)
				*angle = angle_map[ctile];
			ctile <<= 4;
			
			uint8_t ind_y = y & 0xF;
			if (tilev & META_Y_FLIP)
			{
				ind_y ^= 0xF;
				if (angle != NULL)
					*angle = (-(*angle + 0x40)) - 0x40;
			}
			if (tilev & META_X_FLIP)
			{
				if (angle != NULL)
					*angle = -*angle;
			}
			
			//Get width
			int16_t width = (int8_t)width_map[ctile + ind_y];
			if ((tilev ^= flip) & META_X_FLIP)
				width = -width;
			
			//Handle hit tile
			if (width > 0)
			{
				if (width != 0x10)
				{
					//Clip to wall
					return 0xF - (width + (x & 0xF));
				}
				else
				{
					//Check tile behind
					return FindWall2(obj, x - inc, y, solid, flip, angle) - 0x10;
				}
			}
			else
			{
				//Check tile behind
				if ((width += (x & 0xF)) < 0)
					return FindWall2(obj, x - inc, y, solid, flip, angle) - 0x10;
			}
		}
	}
	
	//No tile found, check tile ahead
	return FindWall2(obj, x + inc, y, solid, flip, angle) + 0x10;
}
