#include "syscall.h"
#include "fluke/fluke.h"
#include "forth/forth.h"
#include "mem/memory.h"
#include "schedule.h"
#include "bootloader.h"
#include "descriptor.hxx"
#include "handle.hxx"
#include "process.hxx"
#include "share/share.h"
#include "irq.h"

#define SYSCALL(s) static result<usize> do_syscall_ ## s (Context ctx, [[maybe_unused]] Thread* thread)

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

SYSCALL(user_share)
{
    (void)ctx;
    return (usize)user_share_get_objects();
}

SYSCALL(claim_irq)
{
    int fd = 0;
    auto desc = TRY(descriptor_new(&thread->get_process().descriptors, &fd));
    auto handle = TRY(irq_claim(CTX_SYS_A0(ctx)));
    descriptor_assign(desc, handle);
    return fd;
}

SYSCALL(klog)
{
    char buffer[1024 + 1];
    auto ptr = (void*)CTX_SYS_A0(ctx);
    auto len = std::min<usize>(CTX_SYS_A1(ctx), sizeof(buffer)-1);
    TRY_ERRC(copy_from_user(buffer, ptr, len));
    buffer[len] = 0;
    klog("%s", buffer);
    return len;
}

SYSCALL(panic)
{
    (void)ctx;
    panic("SYSCALL_panic called");
}

SYSCALL(read)
{
    int fd = CTX_SYS_A0(ctx);
    auto buffer = (void*)CTX_SYS_A1(ctx);
    isize len = std::min<usize>(CTX_SYS_A2(ctx), ISIZE_MAX);

    auto desc = TRY(descriptor_get(&thread->get_process().descriptors, fd));
    auto handle = desc->handle;

    TRY_ERRC(check_user_range(buffer, len));

    return TRY(handle->read(buffer, len));
}

SYSCALL(seek)
{
    int fd = CTX_SYS_A0(ctx);
    auto desc = TRY(descriptor_get(&thread->get_process().descriptors, fd));
    auto handle = desc->handle;
    return TRY(handle->seek(CTX_SYS_A1(ctx), CTX_SYS_A2(ctx)));
}

SYSCALL(objctl)
{
    int fd = CTX_SYS_A0(ctx);
    unsigned op = CTX_SYS_A1(ctx);
    auto desc = TRY(descriptor_get(&thread->get_process().descriptors, fd));
    auto handle = desc->handle;
    return handle->ctl(ctx, op);
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
    if (flags & ~(MAP_FIXED | MAP_FIXED_NOREPLACE))
        return error_code(EINVAL);

    if (flags & (MAP_FIXED | MAP_FIXED_NOREPLACE)) {
        if (!is_page_aligned(addr))
            return error_code(EINVAL);

        if (flags & MAP_FIXED)
            TRY(process->vm.alloc_fixed_overwrite(addr, len, prot));
        else
            TRY(process->vm.alloc_fixed_noreplace(addr, len, prot));
    } else {
        addr = ROUND_DOWN_P2(addr, PAGE_SIZE);
        addr = (usize)TRY(process->vm.alloc_movable(addr, len, prot));
    }
    // process->vm.print();
    return addr;
}

SYSCALL(virtual_unmap)
{
    auto process = container_of(thread, Process, thread);

    isize len = CTX_SYS_A1(ctx);
    len = ROUND_UP_P2(len, PAGE_SIZE);
    usize addr = CTX_SYS_A0(ctx);
    if (!is_page_aligned(addr))
        return error_code(EINVAL);
    TRY_ERRC(check_user_range((void*)addr, len));

    TRY(process->vm.free(addr, len));

    // process->vm.print();
    return 0;
}

struct syscall_table_entry {
    result<usize> (*handler)(Context ctx, Thread*);
    const char* name;
};

static constexpr auto syscalls = []{
    constexpr auto N = SYSCALL_virtual_unmap - SYSCALL_nop + 1;
    std::array<syscall_table_entry, N> table = {};

#define ENTRY(s) \
    table[ SYSCALL_ ## s - SYSCALL_nop ].handler = do_syscall_ ## s; \
    table[ SYSCALL_ ## s - SYSCALL_nop ].name = # s;

    ENTRY(nop);
    ENTRY(forth_interpret);
    ENTRY(nsleep);
    ENTRY(open_module);
    ENTRY(user_share);
    ENTRY(claim_irq);
    ENTRY(klog);
    ENTRY(panic);
    ENTRY(read);
    ENTRY(seek);
    ENTRY(objctl);
    ENTRY(virtual_map);
    ENTRY(virtual_unmap);

    return table;
}();

usize
syscall(Context ctx, Thread_context* thread_ctx)
{
    usize index = CTX_SYS_OP(ctx) - SYSCALL_nop;
    if (index >= syscalls.count())
        return -ENOSYS;

    auto handler = syscalls[index].handler;

    if (handler == nullptr)
        return -ENOSYS;

    auto result = handler(ctx, thread_cast(thread_ctx));
    return result ? result.value() : -result.err();
}

const char*
syscall_get_name(Context ctx)
{
    usize index = CTX_SYS_OP(ctx) - SYSCALL_nop;
    if (index >= syscalls.count())
        return "(unnamed)";
    return syscalls[index].name ?: "(unnamed)";
}
