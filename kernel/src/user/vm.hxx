#pragma once

#include "klib.hxx"
#include "x86_64/mmu.h"
#include "fluke/virtual.h" // IWYU pragma: export

struct VM {
    Page_map page_map;
    // Tree ranges;
    // usize first_region;
    struct VM_area* first;

    result<void> init();
    result<void> alloc_fixed_overwrite(usize address, isize length, unsigned prot);
    result<void> alloc_fixed_noreplace(usize address, isize length, unsigned prot);
    result<char*> alloc_movable(usize hint, isize length, unsigned prot);
    result<void> free(usize address, isize length);
    void print();
};
