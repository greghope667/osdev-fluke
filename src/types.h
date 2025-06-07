#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef ptrdiff_t isize;
typedef size_t usize;

#define EOF -1

#define ARRAY_LENGTH(x) ((isize)((sizeof(x))/(sizeof(x[0]))))
