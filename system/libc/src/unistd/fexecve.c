#include <errno.h>
#include <fluke/defs/limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <elf.h>
#include <sys/mman.h>
#include <fluke/fluke.h>
#include <setjmp.h>

#define STACK_SIZE (80 * 1024)

/*
 * Initial Program Stack:
 *
 * Information/Data Block
 * Null auxv entry                              eightbyte
 * Auxv entries                                 2 eightbytes each
 * 0                                            eightbyte
 * Envp pointers                                1 eightbyte each
 * 0                                            eightbyte
 * Arg pointers                                 1 eightbyte each
 * Arg count                        %rsp        eightbyte
 */

static bool
read_into(int fd, void* out, size_t len)
{
    do {
        auto n = read(fd, out, len);
        if (n < 0)
            return false;
        if (n == 0) { // Unexpected EOF, file too short
            errno = ENOEXEC;
            return false;
        }
        len -= n;
        out += n;
    } while (len);

    return true;
}

static bool
check_elf_valid(Elf64_Ehdr* elf)
{
    static const char IDENT[] = {
        [0]             = ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3,
        [EI_CLASS]      = ELFCLASS64,
        [EI_DATA]       = ELFDATA2LSB,
        [EI_VERSION]    = EV_CURRENT,
        [EI_OSABI]      = ELFOSABI_SYSV,
    };

    return
        memcmp(elf->e_ident, IDENT, sizeof(IDENT)) == 0
        && elf->e_type      == ET_EXEC
        && elf->e_machine   == EM_X86_64
        && elf->e_version   == EV_CURRENT
        && elf->e_entry     != 0
        && elf->e_phoff     != 0
        && elf->e_ehsize    == sizeof(Elf64_Ehdr)
        && elf->e_phentsize == sizeof(Elf64_Phdr)
        && elf->e_phnum      > 0
        ;
}

struct stack_builder {
    char* base;
    char* top;
    char* arg_next;
    char* data_area;
    jmp_buf on_error;
};

static void
create_stack(struct stack_builder* s)
{
    long stack_hint =
        0x7e00'0000'0000 | (0xff'ffff'f000 & (__builtin_ia32_rdtsc() << 12));

    char* stack_base = mmap(
        (void*)stack_hint, STACK_SIZE, PROT_READ|PROT_WRITE, MAP_ANON, -1, 0);

    if (stack_base == MAP_FAILED)
        _longjmp(s->on_error, -1);

    s->base       = stack_base;
    s->arg_next   = stack_base;
    s->data_area  = stack_base + STACK_SIZE;
    s->top        = stack_base + STACK_SIZE;
}

__attribute((noreturn, cold))
static void
overflow(struct stack_builder* s)
{
    errno = E2BIG;
    _longjmp(s->on_error, -1);
}

static void
check_space(struct stack_builder* s, size_t additional)
{
    size_t space = s->data_area - s->arg_next;
    size_t used = STACK_SIZE - space;

    if (used + additional >= ARG_MAX)
        overflow(s);
}

static void
push_arg(struct stack_builder* s, long v)
{
    check_space(s, 8);
    memcpy(s->arg_next, &v, 8);
    s->arg_next += 8;
}

static void
push_data_string(struct stack_builder* s, const char* str)
{
    auto len = strlen(str);
    check_space(s, len+1);
    s->data_area -= (len + 1);
    memcpy(s->data_area, str, len);
    s->data_area[len] = 0;
}

static void
add_array(struct stack_builder* s, char* const args[])
{
    for (auto a = args; *a; a++) {
        push_data_string(s, *a);
        push_arg(s, (long)s->data_area);
    }
    push_arg(s, 0); // terminator
}

static void*
finish_stack(struct stack_builder* s)
{
    // Align data
    s->data_area = (char*)((long)s->data_area & ~0x7l);

    size_t arg_size = s->arg_next - s->base;
    if (((long)s->data_area - arg_size) & 0x8)
        s->data_area -= 8;

    void* stack_ptr = s->data_area - arg_size;
    return memmove(stack_ptr, s->base, arg_size);
}

static void*
build_stack(struct stack_builder* s, char* const argv[], char* const envp[])
{
    if (!_setjmp(s->on_error)) {
        memset(s, 0, sizeof(*s));
        create_stack(s);

        push_arg(s, 0);             // Reserve argc
        add_array(s, argv);         // Push argv
        long argc = (s->arg_next - 16 - s->base) / 8;
        memcpy(s->base, &argc, 8);  // Write argc

        add_array(s, envp);         // Push envp
        push_arg(s, 0);             // Auxv terminator
        return finish_stack(s);
    } else {
        if (s->base)
            munmap(s->base, s->top - s->base);
        return 0;
    }
}

__NORETURN
static void
exec(int fd, void* sp, struct stack_builder* s)
{
    void (*loadelf)(int, void*, void*, void*) = _fluke_user_share_name("exec_loadelf");
    loadelf(fd, sp, s->base, s->top);
    abort();
}

int
fexecve(int fd, char* const argv[], char* const envp[])
{
    if (lseek(fd, 0, SEEK_SET) == -1)
        return -1;

    Elf64_Ehdr header;

    if (!read_into(fd, &header, sizeof(header)))
        return -1;

    if (!check_elf_valid(&header))
        return -1;

    struct stack_builder s;

    auto sp = build_stack(&s, argv, envp);
    if (!sp)
        return -1;

    exec(fd, sp, &s);
}
