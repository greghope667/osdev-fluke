#pragma once

#include "kdef.h"
#include "mem/memory.h"

enum mmu_mode {
    MMU_MODE_DEFAULT    = 0,
    MMU_MODE_EXEC       = 1 << 0,
    MMU_MODE_WRITE      = 1 << 1,
    MMU_MODE_USER       = 1 << 2,
};

enum mmu_cache {
    MMU_CACHE_DEFAULT   = 0,
    MMU_CACHE_NONE      = 1 << 0,
    // MMU_CACHE_FB        = 1 << 1,
};

struct Page_map {
    physical_t top_address;
};

void mmu_assign(struct Page_map pm, usize address, isize length, enum mmu_mode mode, enum mmu_cache cache);
void mmu_clear(struct Page_map pm, usize address, isize length);
void mmu_point1(struct Page_map pm, usize address, physical_t target, enum mmu_mode mode, enum mmu_cache cache);

physical_t virt_to_phys(struct Page_map pm, void* ptr);

void mmu_set_address_space(struct Page_map pm);
struct Page_map mmu_get_address_space();
void mmu_reload_address_space();

// Creates a new address space with kernel space mapped, and user space empty
struct Page_map mmu_create_address_space();
void mmu_destroy_address_space(struct Page_map pm);
