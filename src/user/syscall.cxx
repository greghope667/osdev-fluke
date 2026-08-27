#include "syscall.h"
#include "fluke.h"
#include "forth/forth.h"
#include "mem/memory.h"
#include "schedule.h"
#include "bootloader.h"
#include "descriptor.hxx"
#include "handle.hxx"
#include "process.hxx"

#define SYSCALL(s) static result<usize> do_syscall_ ## s (Context ctx, [[maybe_unused]] Thread* thread)
#define ENTRY(s) [ SYSCALL_ ## s - SYSCALL_nop ] = do_syscall_ ## s

SYSCALL(nop)
{
    (void)ctx;
    return 0;
}

SYSCALL(forth_interpret)
{
    isize forth_stack[32];
    if (forth_interpret((const char*)CTX_SYS_A0(ctx), CTX_SYS_A1(ctx), forth_stack+1) >= 0) {
        CTX_SYS_R1(ctx) = forth_stack[1];
        return 0;
    } else {
        return error_code(EIO);
    }
}

SYSCALL(nsleep)
{
    isize duration = CTX_SYS_A0(ctx);
    if (duration <= 0)
        return error_code(EINVAL);

    CTX_SYS_R0(ctx) = 0;
    schedule_nanosleep(cpu_context_save(), duration);
    schedule_or_exit();
}

SYSCALL(open_module)
{
    int fd = 0;
    auto process = container_of(thread, Process, thread);
    auto desc = TRY(descriptor_new(&process->descriptors, &fd));

    char path[128];
    usize len = CTX_SYS_A1(ctx);
    if (len > sizeof(path))
        return error_code(ENAMETOOLONG);

    TRY_ERRC(copy_from_user(path, (const void*)CTX_SYS_A0(ctx), len));

    struct Handle* handle = bootloader_open_module(path, len);
    if (!handle)
        return error_code(ENOENT);

    descriptor_assign(desc, handle);
    return fd;
}

SYSCALL(read)
{
    int fd = CTX_SYS_A0(ctx);
    auto buffer = (void*)CTX_SYS_A1(ctx);
    isize len = std::min<usize>(CTX_SYS_A2(ctx), ISIZE_MAX);

    auto process = container_of(thread, Process, thread);
    auto desc = TRY(descriptor_get(&process->descriptors, fd));
    auto handle = desc->handle;

    TRY_ERRC(check_user_range(buffer, len));

    return TRY(handle->read(buffer, len));
}

SYSCALL(virtual_map)
{
    auto process = container_of(thread, Process, thread);

    isize len = CTX_SYS_A1(ctx);
    len = ROUND_UP_P2(len, PAGE_SIZE);
    usize addr = CTX_SYS_A0(ctx);
    TRY_ERRC(check_user_range((void*)addr, len));

    usize prot = CTX_SYS_A2(ctx);
    if (prot & ~(PROT_EXEC | PROT_READ | PROT_WRITE))
      return error_code(EINVAL);

    usize flags = CTX_SYS_A3(ctx);
    if (flags & ~MAP_FIXED)
        return error_code(EINVAL);

    if (flags & MAP_FIXED) {
        if (!is_page_aligned(addr))
            return error_code(EINVAL);
        TRY(process->vm.alloc_fixed(addr, len, prot));
        return addr;
    } else {
        return (usize)TRY(process->vm.alloc_movable(addr, len, prot));
    }
}

#pragma GCC diagnostic ignored "-Wc99-designator"

result<usize> (*const syscalls[])(Context ctx, Thread*) = {
    ENTRY(nop),
    ENTRY(forth_interpret),
    ENTRY(nsleep),
    ENTRY(open_module),
    ENTRY(read),
    ENTRY(virtual_map),
};

usize
syscall(Context ctx, Thread_context* thread_ctx)
{
    usize index = CTX_SYS_OP(ctx) - SYSCALL_nop;
    if (index >= ARRAY_LENGTH(syscalls))
        return -ENOSYS;

    if (syscalls[index] == nullptr)
        return -ENOSYS;

    auto result = syscalls[index](ctx, static_cast<Thread*>(thread_ctx));
    return result ? result.value() : -result.err();
}
