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

#include </usr/include/elf.h>
#include "kdef.h"
#include <fluke/defs/fluke.h>

#define UTEXT __attribute__((section(".usertext")))
#define UCONST __attribute__((section(".userconst")))
#define INLINE __attribute__((always_inline)) inline

asm (
"       .pushsection .usertext,\"ax\",@progbits\n"
"memcpy:\n"
"       mov     %rdi, %rax\n"
"       mov     %rdx, %rcx\n"
"       rep movsb\n"
"       ret\n"

"trampoline:\n"
"       xor     %ebp, %ebp\n"
"       mov     %rsi, %rsp\n"
"       call    exec_elf_stage2\n"
"       jmp     *%rax\n"

"fail:\n"
"       hlt\n"
"       .popsection\n"
);

extern void trampoline(int fd, long stack, Elf64_Ehdr* elf) __attribute__((noreturn));
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
    if (syscall4(SYSCALL_virtual_map, base, size, flags, MAP_FIXED) <= 0)
        fail();
}

UTEXT static void
unmap(long base, long size)
{
    if (syscall2(SYSCALL_virtual_unmap, base, size) < 0)
        fail();
}

UTEXT static bool
read_into(int fd, void* buffer, long length, long offset)
{
    long n = syscall3(SYSCALL_seek, fd, offset, SEEK_SET);
    if (n < 0) return false;

    while (length > 0) {
        n = syscall3(SYSCALL_read, fd, (long)buffer, length);
        if (n <= 0) return false;
        buffer += n;
        length -= n;
    }
    return true;
}


/* Basic checks to see if this looks like a valid elf structure.
 * If not, don't even attempt to load it + report an error to the user.
 * We don't need to strictly check everything here as this is in an isolated
 * user space process so any out of bounds reads/writes are not dangerous.
 */
UTEXT bool
check_elf_valid(Elf64_Ehdr* elf)
{
    UCONST static const char IDENT[] = "\177ELF\2\1\1\0";

    for (int i=0; i<8; i++)
        if (elf->e_ident[i] != IDENT[i])
            return false;

    if (elf->e_entry == 0)
        return false;

    if (elf->e_phoff == 0)
        return false;

    if (elf->e_phentsize != sizeof(Elf64_Phdr))
        return false;

    if (elf->e_phnum == 0)
        return false;

    return true;
}

UTEXT static void
load_segment(int fd, Elf64_Phdr* phdr)
{
    long base = ROUND_DOWN_P2(phdr->p_vaddr, PAGE_SIZE);
    long end = ROUND_UP_P2(phdr->p_vaddr + phdr->p_memsz, PAGE_SIZE);
    const int flags = PROT_READ|PROT_WRITE|PROT_EXEC;

    map_fixed(base, end - base, flags);
    if (!read_into(fd, (void*)phdr->p_vaddr, phdr->p_filesz, phdr->p_offset))
        fail();
}

#define STACK_SIZE 0x10000

UTEXT static long
create_stack()
{
    long rng = (long)__builtin_ia32_rdtsc() & 0xff'ffff;
    long base = 0x7f00'0000'0000 | (rng << 16);
    return syscall4(SYSCALL_virtual_map, base, STACK_SIZE, PROT_READ|PROT_WRITE, 0);
}

UTEXT int
user_share_exec_elf(int fd)
{
    Elf64_Ehdr elf;
    if (!read_into(fd, &elf, sizeof(elf), 0))
        return -EIO;

    if (!check_elf_valid(&elf))
        return -ENOEXEC;

    long stack_base = create_stack();
    if (stack_base <= 0)
        return -ENOMEM;

    // Basic checks now done. At this point, we've committed to the
    // attempt to exec the file. Errors are now process-exit

    // syscall(SYS_exec_flush_old)
    // stack_add_args()

    trampoline(fd, stack_base + STACK_SIZE, &elf);
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
        if (!read_into(fd, &phdr, sizeof(phdr), phdr_location))
            fail();

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

/* Stage 2 - operating on new stack. Returning from this function
 * jumps to the new address (elf entry point)
 */
UTEXT long
exec_elf_stage2(int fd, long stack, Elf64_Ehdr* elf_)
{
    Elf64_Ehdr elf;
    memcpy(&elf, elf_, sizeof(elf)); // Copy data before unmapping

    const long stack_base = stack - STACK_SIZE;
    unmap(0, stack_base);
    unmap(stack, 0x8000'0000'0000 - stack);

    load_elf_segments(fd, &elf);

    return elf.e_entry;
}
