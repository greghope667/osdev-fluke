/* C code that is mapped into kernel space, but executable from user space.
 * (a bit like VSDO on linux)
 * All code must be in ".usertext" section to be stored together, away
 * from kernel data and code.
 *
 * Although this code is linked into the kernel, it cannot call ANY kernel
 * functions or we'll get a page fault. Required functions (e.g. memcpy) must
 * be implemented here, but statically linked to not conflict with kernel
 * symbols
 */

#include <elf.h>
#include "kdef.h"
#include <fluke/defs/fluke.h>

#define UTEXT __attribute__((section(".usertext")))
// #define UCONST __attribute__((section(".userconst")))
#define INLINE __attribute__((always_inline)) inline

asm (
"       .pushsection .usertext,\"ax\",@progbits\n"
"memcpy:\n"
"       mov     %rdi, %rax\n"
"       mov     %rdx, %rcx\n"
"       rep movsb\n"
"       ret\n"

"       .global user_share_exec_elf\n"

"user_share_exec_elf:\n"
"       xor     %ebp, %ebp\n"
"       mov     %rsi, %rsp\n"
"       call    exec_elf_stage2\n"

"fail:\n"
"       hlt\n"
"       .popsection\n"
);

extern void* memcpy(void*, const void*, usize);
extern void fail() __attribute__((noreturn));

INLINE static long
syscall2(long rax, long a1, long a2)
{
    asm volatile (
        "syscall"
        : "+a"(rax)
        : "D"(a1), "S"(a2)
        : "rcx", "r11", "memory"
    );
    return rax;
}

INLINE static long
syscall3(long rax, long a1, long a2, long a3)
{
    asm volatile (
        "syscall"
        : "+a"(rax)
        : "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory"
    );
    return rax;
}

INLINE static long
syscall4(long rax, long a1, long a2, long a3, long a4)
{
    register long r10 asm("r10") = a4;
    asm volatile (
        "syscall"
        : "+a"(rax)
        : "D"(a1), "S"(a2), "d"(a3), "r"(r10)
        : "rcx", "r11", "memory"
    );
    return rax;
}

UTEXT static void
map_fixed(long base, long size, int flags)
{
    if (syscall4(SYSCALL_virtual_map, base, size, flags, MAP_FIXED) != base)
        fail();
}

UTEXT static void
unmap(long base, long size)
{
    if (syscall2(SYSCALL_virtual_unmap, base, size) < 0)
        fail();
}

UTEXT static void
read_into(int fd, void* buffer, long length, long offset)
{
    long n = syscall3(SYSCALL_seek, fd, offset, SEEK_SET);
    if (n < 0) fail();

    while (length > 0) {
        n = syscall3(SYSCALL_read, fd, (long)buffer, length);
        if (n <= 0) fail();
        buffer += n;
        length -= n;
    }
}

UTEXT static void
load_segment(int fd, Elf64_Phdr* phdr)
{
    long base = ROUND_DOWN_P2(phdr->p_vaddr, PAGE_SIZE);
    long end = ROUND_UP_P2(phdr->p_vaddr + phdr->p_memsz, PAGE_SIZE);
    const int flags = PROT_READ|PROT_WRITE|(phdr->p_flags & PF_X ? PROT_EXEC : 0);

    map_fixed(base, end - base, flags);
    read_into(fd, (void*)phdr->p_vaddr, phdr->p_filesz, phdr->p_offset);
}

UTEXT static void
load_elf_segments(int fd, Elf64_Ehdr* elf)
{
    const int phnum = elf->e_phnum;
    const int phoff = elf->e_phoff;

    unsigned long last_loaded_address = PAGE_SIZE;

    for (int i=0; i<phnum; i++) {
        Elf64_Phdr phdr;
        long phdr_location = phoff + i * sizeof(phdr);
        read_into(fd, &phdr, sizeof(phdr), phdr_location);

        switch (phdr.p_type) {
        case PT_LOAD:
                if (ROUND_DOWN_P2(phdr.p_vaddr, PAGE_SIZE) < last_loaded_address)
                    // Overlapping segments
                    fail();

            last_loaded_address = phdr.p_vaddr + phdr.p_memsz;

            load_segment(fd, &phdr);

            break;

        case PT_INTERP:
            // Not yet implemented (no filesystem)
            fail();

        default:
            break;
        }
    }
}

__attribute__((noreturn))
UTEXT void
exec_elf_stage2(int fd, usize stack_ptr, usize stack_base, usize stack_top)
{
    Elf64_Ehdr elf;
    read_into(fd, &elf, sizeof(elf), 0);

    unmap(0, stack_base);
    unmap(stack_top, 0x8000'0000'0000 - stack_top);

    load_elf_segments(fd, &elf);

    syscall2(SYSCALL_exec_flush, elf.e_entry, stack_ptr);
    fail();
}
