#pragma once

#include <stdint.h>

//Multi-size ints
typedef union
{
	struct
	{
		#ifdef SCP_BIG_ENDIAN
			int16_t u;
			uint16_t l;
		#else
			uint16_t l;
			int16_t u;
		#endif
	} f;
	int32_t v;
} dword_s;

typedef union
{
	struct
	{
		#ifdef SCP_BIG_ENDIAN
			uint16_t u;
			uint16_t l;
		#else
			uint16_t l;
			uint16_t u;
		#endif
	} f;
	uint32_t v;
} dword_u;
