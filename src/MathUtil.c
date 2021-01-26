#include "MathUtil.h"

//Random number generation
dword_u random_seed;

uint32_t RandomNumber()
{
	//Re-seed if 0
	if (random_seed.v == 0)
		random_seed.v = 0x2A6D365A;
	
	//Scramble our current seed
	dword_u result = {.v = random_seed.v};
	random_seed.v <<= 2;
	random_seed.v += result.v;
	random_seed.v <<= 3;
	random_seed.v += result.v;
	result.f.l = random_seed.f.l;
	
	//switch to random_seed.f.u because of a swap
	result.f.l += random_seed.f.u;
	random_seed.f.u = result.f.l;
	
	return result.v;
}
