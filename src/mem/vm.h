#pragma once

#include "kdef.h"
#include "x86_64/mmu.h"

struct VM {
    struct Page_map page_map;
};

enum VM_FLAG {
    VM_EXEC = 1 << 0,
    VM_WRITE = 1 << 1,
};

// struct VM* vm_create();
void vm_init(struct VM* vm);
void vm_alloc(struct VM* vm, usize address, isize length, enum VM_FLAG flags);
void vm_alloc_movable(struct VM*, usize hint, isize length, enum VM_FLAG flags);
void vm_free(struct VM* vm, usize address, isize length);
