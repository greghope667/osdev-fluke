#pragma once

#include "kdef.h"

typedef struct Physical {
    usize address;
} physical_t;

extern usize hhdm_offset;

inline void*
phys_to_virt(physical_t p) { return (void*)(p.address + hhdm_offset); }

inline physical_t
hhdm_virt_to_phys(void* v) { return (physical_t){ (usize)v - hhdm_offset }; }

#define MEM_LOW_HALF_MAX    0x0000'8000'0000'0000ull
#define MEM_HIGH_HALF_MIN   0xffff'8000'0000'0000ull
#define PAGE_SIZE 0x1000

inline bool
is_user_pointer(const void* v) { return (usize)v < MEM_LOW_HALF_MAX; }

inline bool
is_kernel_pointer(const void* v) { return (usize)v >= MEM_HIGH_HALF_MIN; }

inline bool
is_page_aligned(usize a) { return (a & (PAGE_SIZE-1)) == 0; }

bool copy_to_user(void* dest, const void* src, isize size);
bool copy_from_user(void* dest, const void* src, isize size);
