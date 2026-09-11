#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/time.h.html

#include <fluke/types.h>

typedef __clock_t       clock_t;
typedef __size_t        size_t;
typedef __time_t        time_t;
typedef __pid_t         pid_t;

struct timespec {
    time_t  tv_sec;
    long    tv_nsec;
};

#define NULL ((void*)0)
#define CLOCKS_PER_SEC ((clock_t)1000000)
#define CLOCK_MONOTONIC 1

#ifdef __cplusplus
extern "C" {
#endif

clock_t clock(void);

#ifdef __cplusplus
} // extern C
#endif
