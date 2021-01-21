#pragma once

#include <stdint.h>

//Fixed point ints
typedef union
{
	struct
	{
		#ifdef SCP_BIG_ENDIAN
			int8_t u;
			uint8_t l;
		#else
			uint8_t l;
			int8_t u;
		#endif
	} f;
	int16_t v;
} word_s;

typedef union
{
	struct
	{
		#ifdef SCP_BIG_ENDIAN
			uint8_t u;
			uint8_t l;
		#else
			uint8_t l;
			uint8_t u;
		#endif
	} f;
	uint16_t v;
} word_u;

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
