#pragma once

typedef signed char     __int8_t;
typedef short           __int16_t;
typedef int             __int32_t;
typedef long            __int64_t;
typedef long            __ssize_t;
typedef long            __isize_t;
typedef long            __ptrdiff_t;
typedef long            __intptr_t;

typedef unsigned char   __uint8_t;
typedef unsigned short  __uint16_t;
typedef unsigned int    __uint32_t;
typedef unsigned long   __uint64_t;
typedef unsigned long   __size_t;
typedef unsigned long   __uintptr_t;

#ifdef __cplusplus
static_assert(sizeof(__int8_t) == 1);
static_assert(sizeof(__int16_t) == 2);
static_assert(sizeof(__int32_t) == 4);
static_assert(sizeof(__int64_t) == 8);
static_assert(sizeof(__size_t) == sizeof(void*));
#endif

typedef __isize_t               __clock_t;
typedef __isize_t               __off_t;
typedef __int32_t               __pid_t;
typedef volatile int            __sig_atomic_t;
typedef __uint64_t              __sigset_t;
typedef __int64_t               __time_t;
typedef __builtin_va_list       __va_list;
