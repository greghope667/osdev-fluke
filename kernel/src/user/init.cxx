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

    int fd;
    auto desc = TRY(descriptor_new(&proc->descriptors, &fd));
    auto thread = TRY(proc->spawn_thread(func, (usize)stack + PAGE_SIZE, fd));

    auto cmdline = bootloader_cmdline();
    klog("Loading init program from module: %s\n", cmdline);

    auto handle = bootloader_open_module(cmdline, strlen(cmdline));
    if (not handle)
        panic("Init program not found");
    descriptor_assign(desc, handle);

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
