#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/stdint.h.html

#include <fluke/types.h>

typedef __int8_t        int8_t;
typedef __int16_t       int16_t;
typedef __int32_t       int32_t;
typedef __int64_t       int64_t;
typedef __uint8_t       uint8_t;
typedef __uint16_t      uint16_t;
typedef __uint32_t      uint32_t;
typedef __uint64_t      uint64_t;

typedef __int8_t        int_least8_t;
typedef __int16_t       int_least16_t;
typedef __int32_t       int_least32_t;
typedef __int64_t       int_least64_t;
typedef __uint8_t       uint_least8_t;
typedef __uint16_t      uint_least16_t;
typedef __uint32_t      uint_least32_t;
typedef __uint64_t      uint_least64_t;

typedef __int32_t       int_fast8_t;
typedef __int32_t       int_fast16_t;
typedef __int32_t       int_fast32_t;
typedef __int64_t       int_fast64_t;
typedef __uint32_t      uint_fast8_t;
typedef __uint32_t      uint_fast16_t;
typedef __uint32_t      uint_fast32_t;
typedef __uint64_t      uint_fast64_t;

typedef __isize_t       intmax_t;
typedef __size_t        uintmax_t;

/*
#define INT8_C(v)       v
#define INT16_C(v)      v
#define INT32_C(v)      v
#define INT64_C(v)      v ## L
#define INTMAX_C(v)     v ## L
#define UINT8_C(v)      v
#define UINT16_C(v)     v
#define UINT32_C(v)     v ## U
#define UINT64_C(v)     v ## UL
#define UINTMAX_C(v)    v ## UL
*/

#define PTRDIFF_MAX __PTRDIFF_MAX__
