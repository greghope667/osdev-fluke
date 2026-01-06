#include "process.h"

#include "mem/alloc.h"
#include "x86_64/cpu.h"

struct Process*
process_create()
{
    struct Process* process = kalloc(sizeof(*process));
    if (!process)
        return nullptr;

    memset(process, 0, sizeof(*process));
    vm_init(&process->vm);

    return process;
}

void
process_load_flat_binary(struct Process* process, const void* binary, usize size)
{
    assert(process->state == SPAWNING);

    usize prog = 0x200000;
    usize program_size = ROUND_UP_P2(size, PAGE_SIZE) + 4 * PAGE_SIZE;

    cpu_context_initialise_user(&process->saved_context, prog, prog + program_size);
    vm_alloc(&process->vm, prog, program_size, VM_EXEC | VM_WRITE);

    mmu_set_address_space(process->vm.page_map);

    memcpy((void*)prog, binary, size);
    memset((void*)prog + size, 0, program_size - size);

    process->state = FLOATING;
}
