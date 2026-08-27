#include "vm.hxx"

#include "fluke.h"
#include "klib.h"
#include "mem/memory.h"
#include "x86_64/mmu.h"

struct VM_area {
    struct Tree_node node;
    usize start;
    usize end;
    usize free_after;
    unsigned flags;
    struct VM_area* prev;
    struct VM_area* next;
};

result<void>
VM::init()
{
    page_map = mmu_create_address_space();
    ranges.root = nullptr;
    return {};
}

result<void>
VM::alloc_fixed(usize address, isize length, unsigned flags)
{

    puts(__FILE__ " TODO: replace with proper implementation");
    assert(is_page_aligned(address));
    assert(is_page_aligned(length));

    enum mmu_mode mode = MMU_MODE_DEFAULT;
    if (flags)
        mode |= MMU_MODE_USER;
    if (flags & PROT_EXEC)
        mode |= MMU_MODE_EXEC;
    if (flags & PROT_WRITE)
        mode |= MMU_MODE_WRITE;

    mmu_assign(page_map, address, length, mode, MMU_CACHE_DEFAULT);
    return {};
}

result<void*>
VM::alloc_movable(usize hint, isize length, unsigned flags)
{
    puts(__FILE__ " TODO: replace with proper implementation");
    TRY(alloc_fixed(hint, length, flags));
    return (void*)hint;
}
