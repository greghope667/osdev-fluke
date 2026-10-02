#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/sys_mman.h.html

#include <fluke/defs/virtual.h>
#include <fluke/types.h>

// TODO: make map anon/map shared an actual thing

#define MAP_ANONYMOUS           0
#define MAP_ANON                0
#define MAP_PRIVATE             0

#define MAP_FAILED              ((void*)-1)

typedef __size_t    size_t;
typedef __off_t     off_t;

void*   mmap(void* __hint, size_t __len, int __prot, int __mflags, int __fd, off_t __off);
int     mprotect(void* __addr, size_t __len, int __prot);
int     munmap(void* __addr, size_t __len);
