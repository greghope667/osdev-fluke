#include "syscall.h"
#include "fluke.h"
#include "forth/forth.h"
#include "schedule.h"
#include "bootloader.h"
#include "descriptor.h"
#include "handle.h"

#define SYSCALL(s) static usize do_syscall_ ## s (Context ctx)
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
        return -EIO;
    }
}

SYSCALL(nsleep)
{
    isize duration = CTX_SYS_A0(ctx);
    if (duration <= 0)
        return -EINVAL;

    CTX_SYS_R0(ctx) = 0;
    schedule_nanosleep(cpu_context_save(), duration);
    schedule_or_exit();
}

SYSCALL(open_module)
{
    int fd = 0;
    struct Process* process = this_cpu->process;
    struct Descriptor* desc = descriptor_new(&process->descriptors, &fd);

    if (!desc)
        return -errno;

    char path[128];
    usize len = CTX_SYS_A1(ctx);
    if (len > sizeof(path))
        return -ENAMETOOLONG;

    if (!copy_from_user(path, (const void*)CTX_SYS_A0(ctx), len))
        return -errno;

    struct Handle* handle = bootloader_open_module(path, len);
    if (!handle)
        return -errno;

    descriptor_assign(desc, handle);
    return fd;
}

SYSCALL(read)
{
    int fd = CTX_SYS_A0(ctx);
    struct Process* process = this_cpu->process;
    struct Descriptor* desc = descriptor_get(&process->descriptors, fd);
    if (!desc)
        return -errno;

    struct Handle* handle = desc->handle;
    if (!handle->vtbl->read)
        return -EINVAL;

    isize count = MIN(CTX_SYS_A2(ctx), (usize)ISIZE_MAX);
    isize n = handle->vtbl->read(handle, (void*)CTX_SYS_A1(ctx), count);
    return n >= 0 ? n : -errno;
}


usize (*const syscalls[])(Context ctx) = {
    ENTRY(nop),
    ENTRY(forth_interpret),
    ENTRY(nsleep),
    ENTRY(open_module),
    ENTRY(read),
};

usize
syscall(Context ctx)
{
    usize index = CTX_SYS_OP(ctx) - SYSCALL_nop;
    if (index >= ARRAY_LENGTH(syscalls))
        return -ENOSYS;

    if (syscalls[index] == nullptr)
        return -ENOSYS;

    return syscalls[index](ctx);
}
