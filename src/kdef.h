#pragma once

/* Common macros and typedefs, as well as all (worthwile) freestanding c headers
 * This file is included by almost every header */

#include <stdint.h> // IWYU pragma: export
#include <stddef.h> // IWYU pragma: export
#include <limits.h> // IWYU pragma: export
#include <stdarg.h> // IWYU pragma: export

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef __uint128_t u128;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef __int128_t i128;

typedef ptrdiff_t isize;
typedef size_t usize;

#define EOF (-1)

#define ARRAY_LENGTH(x) ((isize)((sizeof(x))/(sizeof(x[0]))))

#define MAX(a, b) ({        \
    auto _x = (a);          \
    auto _y = (b);          \
    _x > _y ? _x : _y;      \
})

#define MIN(a, b) ({        \
    auto _x = (a);          \
    auto _y = (b);          \
    _x < _y ? _x : _y;      \
})

#define CLAMP(v, min, max) ({   \
    auto _v = (v);              \
    auto _min = (min);          \
    auto _max = (max);          \
    _v < _min ? _min : (_v > _max ? _max : _v); \
})

#define CLAMP_T(T, v, min, max) ({  \
    T _v = (v);                     \
    T _min = (min);                 \
    T _max = (max);                 \
    _v < _min ? _min : (_v > _max ? _max : _v); \
})

#define PACKED __attribute__((packed))

#define PAGE_SIZE 0x1000

#define container_of(ptr, type, member) ((type*)((char*)ptr - offsetof(type, member)))
