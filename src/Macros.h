#pragma once

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
