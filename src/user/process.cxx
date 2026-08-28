#include "process.hxx"

#include "mem/alloc.hxx"
#include "mem/memory.h"
#include "x86_64/cpu.h"
#include "fluke.h"

result<Process*>
Process::create()
{
    auto process = TRY(owned<Process>::make());

    memset(process.get(), 0, sizeof(*process));
    process->state = SPAWNING;

    TRY(process->vm.init());

    process->thread.page_map_top = process->vm.page_map.top_address;

    return process.release();
}

void
Process::load_flat_binary(const char* binary, usize size)
{
    assert(state == SPAWNING);

    usize prog = 0x200000;
    usize program_size = ROUND_UP_P2(size, PAGE_SIZE) + 4 * PAGE_SIZE;

    cpu_context_initialise_user(&thread.ctx, prog, prog + program_size);
    if (vm.alloc_fixed_noreplace(prog, program_size, PROT_EXEC|PROT_READ|PROT_WRITE).is_err())
        panic("vm_alloc_fixed failed");

    mmu_set_address_space(vm.page_map);

    memcpy((char*)prog, binary, size);
    memset((char*)prog + size, 0, program_size - size);

    state = ACTIVE;
}

struct elf64_header {
    char    e_ident[16];
    u16     e_type;
    u16     e_machine;
    u32     e_version;
    u64     e_entry;
    u64     e_phoff;
    u64     e_shoff;
    u32     e_flags;
    u16     e_ehsize;
    u16     e_phentsize;
    u16     e_phnum;
    u16     e_shentsize;
    u16     e_shnum;
    u16     e_shstrndx;
};

struct elf64_phdr {
    u32     p_type;
    u32     p_flags;
    u64     p_offset;
    u64     p_vaddr;
    u64     p_paddr;
    u64     p_filesz;
    u64     p_memsz;
    u64     p_align;
};

#define PT_LOAD 1

void
Process::load_init_elf(const char* elf)
{
    assert(state == SPAWNING);

    assert(memcmp("\177ELF", elf, 4) == 0);

    auto header = (const struct elf64_header*)elf;
    auto phdrs = (const struct elf64_phdr*)(elf + header->e_phoff);
    int ph_count = header->e_phnum;

    constexpr usize stack = 0x007f'ffff'0000;
    constexpr usize stack_size = 0x1'0000;

    cpu_context_initialise_user(&thread.ctx, header->e_entry, stack);

    mmu_set_address_space(vm.page_map);

    for (int i=0; i<ph_count; i++) {
        struct elf64_phdr p = phdrs[i];
        if (p.p_type == PT_LOAD) {
            assert(is_page_aligned(p.p_vaddr));
            u64 memsz = ROUND_UP_P2(p.p_memsz, PAGE_SIZE);
            if (vm.alloc_fixed_noreplace(p.p_vaddr, memsz, PROT_READ|PROT_WRITE|PROT_EXEC).is_err())
                panic("vm_alloc_fixed failed");
            memcpy((char*)p.p_vaddr, elf + p.p_offset, p.p_filesz);
            memset((char*)p.p_vaddr + p.p_filesz, 0, memsz - p.p_filesz);
        }
    }

    if (vm.alloc_fixed_noreplace(stack - stack_size, stack_size, PROT_READ|PROT_WRITE).is_err())
        panic("vm_alloc_fixed failed");

    memset((char*)stack - stack_size, 0, stack_size);

    state = ACTIVE;
}

// TODO: move this elsewhere
#include "schedule.h"
extern "C" void spawn_init_elf(void* elf)
{
    auto proc = Process::create().value();
    proc->load_init_elf((const char*)elf);
    schedule_ready(&proc->thread);
}

Process&
Thread::get_process()
{
    return *container_of(this, Process, thread);
}

VM& Process::get_vm() { return vm; }
