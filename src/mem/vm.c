#include "vm.h"

#include "x86_64/mmu.h"
#include "klib.h"

void vm_init(struct VM* vm)
{
    vm->page_map = mmu_create_address_space();
}

void
vm_alloc(struct VM *vm, usize address, isize length, enum VM_FLAG flags)
{
    enum mmu_mode mode = MMU_MODE_USER;
    if (flags & VM_EXEC)
        mode |= MMU_MODE_EXEC;
    if (flags & VM_WRITE)
        mode |= MMU_MODE_WRITE;

    // TODO: check alignment

    mmu_assign(vm->page_map, address, length, mode, MMU_CACHE_DEFAULT);
}
