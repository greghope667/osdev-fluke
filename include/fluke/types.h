#pragma once

typedef __INT8_TYPE__       __int8_t;
typedef __INT16_TYPE__      __int16_t;
typedef __INT32_TYPE__      __int32_t;
typedef __INT64_TYPE__      __int64_t;
typedef __PTRDIFF_TYPE__    __ssize_t;
typedef __PTRDIFF_TYPE__    __isize_t;
typedef __PTRDIFF_TYPE__    __ptrdiff_t;
typedef __INTPTR_TYPE__     __intptr_t;

typedef __UINT8_TYPE__      __uint8_t;
typedef __UINT16_TYPE__     __uint16_t;
typedef __UINT32_TYPE__     __uint32_t;
typedef __UINT64_TYPE__     __uint64_t;
typedef __SIZE_TYPE__       __size_t;
typedef __UINTPTR_TYPE__    __uintptr_t;

#ifdef __cplusplus
static_assert(sizeof(__int8_t) == 1);
static_assert(sizeof(__int16_t) == 2);
static_assert(sizeof(__int32_t) == 4);
static_assert(sizeof(__int64_t) == 8);
static_assert(sizeof(__size_t) == sizeof(void*));
#endif

typedef __uint64_t              __blkcnt_t;
typedef __uint64_t              __blksize_t;
typedef __isize_t               __clock_t;
typedef __uint64_t              __dev_t;
typedef __uint32_t              __gid_t;
typedef __uint64_t              __ino_t;
typedef __uint32_t              __mode_t;
typedef __uint64_t              __nlink_t;
typedef __isize_t               __off_t;
typedef __int32_t               __pid_t;
typedef volatile int            __sig_atomic_t;
typedef __uint64_t              __sigset_t;
typedef __int64_t               __time_t;
typedef __uint32_t              __uid_t;
typedef __builtin_va_list       __va_list;

#define __NOTHROW __attribute__((__nothrow__))
#define __NORETURN __attribute__((__noreturn__))
#define __CONST __attribute__((__const__))
