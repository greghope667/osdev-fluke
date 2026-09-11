#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/stddef.h.html

#include <fluke/types.h>

#define NULL ((void*)0)
#define offsetof(type, member) __builtin_offsetof(type, member)

typedef __ptrdiff_t ptrdiff_t;
typedef __size_t size_t;
