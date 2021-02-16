#pragma once

#include "Constants.h"

//Planes
#define MAP_PLANE(base, x, y) ((base) + (((y) * PLANE_WIDTH + (x)) << 1))

//Assertion
#define GLUE(a, b) a ## b
#define GLUE2(a, b) GLUE(a, b)

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
	#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
	#define STATIC_ASSERT(cond, msg) typedef char GLUE2(static_assertion_failed, __LINE__)[(cond) ? 1 : -1]
#endif

//Alignment
#ifdef __GNUC__
	#define ALIGNED2 __attribute__((aligned(2)))
	#define ALIGNED4 __attribute__((aligned(4)))
	#define ALIGNED8 __attribute__((aligned(8)))
	#define ALIGNED16 __attribute__((aligned(16)))
#else
	#define ALIGNED2
	#define ALIGNED4
	#define ALIGNED8
	#define ALIGNED16
#endif

//Byte-swapping
#ifdef SCP_LIL_ENDIAN
	#define LESWAP_16(x) (((x) << 8) | ((x) >> 8))
	#define LESWAP_32(x) ((((x) << 24) & 0xFF000000) | (((x) << 8) & 0x00FF0000) | (((x) >> 8) & 0x0000FF00) | (((x) >> 24) & 0x000000FF))
#else
	#define LESWAP_16(x) (x)
	#define LESWAP_32(x) (x)
#endif

//Helper macros
#define POSITIVE_MOD(x, y) (((x) % (y) + (y)) % (y))

//LevelScroll.h must be included
#define IS_OFFSCREEN(x) (uint16_t)(((x) & ~0x7F) - ((scrpos_x.f.u - 0x80) & ~0x7F)) > (((SCREEN_WIDTH + 0x80) & ~0x7F) + 0x100)

//Resource include
#ifdef SCP_REV00
	#define RES_REV(x) <Resource/x##REV00.h>
#else
	#define RES_REV(x) <Resource/x##REV01.h>
#endif
