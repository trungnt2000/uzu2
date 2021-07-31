#ifndef COMMON_H
#define COMMON_H
#include "SDL.h"
#include <stdbool.h>

typedef Sint8  s8;
typedef Uint8  u8;
typedef Sint16 s16;
typedef Uint16 u16;
typedef Sint32 s32;
typedef Uint32 u32;

typedef void (*Func)();
typedef void* pointer_t;

typedef int (*CompareFunc)(const void* lhs, const void* rhs);
typedef void (*FreeFunc)(void* p);
typedef SDL_bool (*EqualFunc)(const void* lhs, const void* rhs);
typedef u32 (*HashFunc)(const void* v);

#define IN
#define OUT
#define INOUT

#define BOOL SDL_bool
#define TRUE SDL_TRUE
#define FALSE SDL_FALSE

#define ASSERT(cd) SDL_assert(cd)
#define ASSERT_MSG(cd, msg) ASSERT((cd) && (msg))

#ifdef __GNUC__
#define STATIC_ASSERT(con, msg) _Static_assert(con, #msg)
#else
#define STATIC_ASSERT(con, msg) typedef static_assert_##msg[(con) ? 1 : -1]
#endif

#define INLINE static inline

#define BIT(x) (1 << (x))

// clang-format off
INLINE int
max(int a, int b)
{ return a > b ? a : b; }

INLINE int
min(int a, int b)
{ return a < b ? a : b; }

INLINE int
sign(int x)
{ return (x > 0) - (x < 0); }

INLINE int
clamp(int lower, int upper, int x)
{ return max(lower, min(upper, x)); }

INLINE int
signf(float x)
{ return (x > 0) - (x < 0); }

INLINE float
maxf(float a, float b)
{ return a > b ? a : b; }

INLINE float
minf(float a, float b)
{ return a < b ? a : b; }

INLINE float
clampf(float lower, float upper, float x)
{ return maxf(lower, minf(upper, x)); }

INLINE float 
lerpf(float a, float b, float t)
{ return a + (b - a) * t; }
// clang-format on
#endif // COMMON_H
