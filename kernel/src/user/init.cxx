#include "init.h"
#include "bootloader.h"
#include "klib.hxx"
#include "schedule.h"
#include "process.hxx"

extern "C" char user_share_exec_elf[];

static const char boot_module[] = "/boot/init";

static result<void>
try_user_init()
{
    auto proc = TRY(Process::create());

    auto stack = TRY(proc->vm.alloc_movable(0, PAGE_SIZE, PROT_READ|PROT_WRITE));
    auto func = (usize)user_share_exec_elf;
    cpu_context_initialise_user(&proc->thread.ctx, func, (usize)stack + PAGE_SIZE);

    int fd;
    auto desc = TRY(descriptor_new(&proc->descriptors, &fd));
    CTX_SYS_A0(&proc->thread.ctx) = fd;

    auto handle = bootloader_open_module(boot_module, sizeof(boot_module)-1);
    if (not handle)
        panic("Init module /boot/init not found");
    descriptor_assign(desc, handle);

    proc->state = Process::ACTIVE;
    schedule_ready(&proc->thread);

    return {};
}

void
user_init()
{
    auto result = try_user_init();
    if (result.is_err())
        klog("user_init: failed with error code %i\n", result.err());
}
