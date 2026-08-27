#pragma once

#include "klib.hxx"
#include "x86_64/mmu.h"
#include "tree.h"

struct VM {
    struct Page_map page_map;
    struct Tree ranges;

    result<void> init();
    result<void> alloc_fixed(usize address, isize length, unsigned prot);
    result<void*> alloc_movable(usize hint, isize length, unsigned prot);
    void free(usize address, isize length);
    void print();
};
