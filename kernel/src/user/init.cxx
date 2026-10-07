#include "init.h"
#include "bootloader.h"
#include "klib.hxx"
#include "schedule.h"
#include "process.hxx"

extern "C" char user_share_exec_elf[];

static result<void>
try_user_init()
{
    auto proc = TRY(Process::create());

    auto stack = TRY(proc->vm.alloc_movable(0, PAGE_SIZE, PROT_READ|PROT_WRITE));
    auto func = (usize)user_share_exec_elf;
    auto stack_pointer = (usize)stack + PAGE_SIZE - 128;

    int fd;
    auto desc = TRY(proc->descriptors.alloc(fd));
    auto thread = TRY(proc->spawn_thread(func, stack_pointer, fd));

    CTX_SYS_A1(&thread->ctx) = stack_pointer;
    CTX_SYS_A2(&thread->ctx) = (usize)stack;
    thread->ctx.rcx = (usize)stack + PAGE_SIZE;
    // TODO: fix this. Arg is getting passed in R10 instead of RCX
    CTX_SYS_A3(&thread->ctx) = (usize)stack + PAGE_SIZE;

    auto cmdline = bootloader_cmdline();
    klog("Loading init program from module: %s\n", cmdline);

    auto handle = bootloader_open_module(cmdline, strlen(cmdline));
    if (not handle)
        panic("Init program not found");
    desc->assign(handle, true);

    proc->state = Process::ACTIVE;
    schedule_ready(thread);

    return {};
}

void
user_init()
{
    auto result = try_user_init();
    if (result.is_err())
        klog("user_init: failed with error code %i\n", result.err());
}
