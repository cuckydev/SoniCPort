#pragma once

#define GLUE(a, b) a ## b
#define GLUE2(a, b) GLUE(a, b)

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
	#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
	#define STATIC_ASSERT(cond, msg) typedef char GLUE2(static_assertion_failed, __LINE__)[(cond) ? 1 : -1]
#endif

#ifdef SCP_LIL_ENDIAN
	#define LESWAP_16(x) (((x) << 8) | ((x) >> 8))
#else
	#define LESWAP_16(x) (x)
#endif
