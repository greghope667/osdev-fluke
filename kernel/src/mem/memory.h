#pragma once

#include "kdef.h"

#ifdef __cplusplus
extern "C" {
#endif

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

error_code copy_to_user(void* dest, const void* src, isize size);
error_code copy_to_user8(void* dest, u64 value);
error_code copy_from_user(void* dest, const void* src, isize size);
error_code check_user_range(const void* v, isize len);

#define MEMCPY_FAULT_READ 1
#define MEMCPY_FAULT_WRITE 2
[[nodiscard]] int memcpy_catch_fault(
    void* dst, const void* src, isize size, int catch_fault);

struct Thread_context;

[[nodiscard]] int memcpy_user_user_catch_fault(
    struct Thread_context* dest_thread,
    void* dest_addr,
    struct Thread_context* source_thread,
    const void* source_addr,
    isize size
);

#ifdef __cplusplus
}
#endif
