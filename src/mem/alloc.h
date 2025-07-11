#pragma once

#include "kdef.h"

#define ALLOC_MAX PAGE_SIZE

inline int __attribute__((const))
alloc_calc_size_class(usize allocation)
{
    if (allocation < 8) return 0;
    if (allocation > ALLOC_MAX) return -1;
    return 61 - __builtin_clzl(allocation-1);
}

void* alloc_page();
void free_page(void*);

void* kalloc_class(int size_class);
void kfree_class(void* ptr, int size_class);

inline void*
kalloc(usize size)
{
    return kalloc_class(alloc_calc_size_class(size));
}

inline void
kfree(void* ptr, usize size)
{
    return kfree_class(ptr, alloc_calc_size_class(size));
}

void alloc_print_info();

#define xmalloc(x) ({                                                \
    extern void panic(const char* reason) __attribute__((noreturn)); \
    void* ptr = (kalloc(x));                                         \
    if (!ptr) panic("allocation failure");                           \
    ptr;                                                             \
})
