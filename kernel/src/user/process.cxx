#include "process.hxx"

#include "mem/alloc.hxx"
#include "schedule.h"
#include "x86_64/tls.h"

result<Process*>
Process::create()
{
    auto process = TRY(owned<Process>::make());
    TRY(process->vm.init());
    return process.release();
}

result<Thread*>
Process::spawn_thread(usize code, usize stack, usize arg)
{
    auto thread = TRY(owned<Thread>::make(*this));

    TRY_ERRC(cpu_context_initialise_user(
        thread.get(),
        vm.page_map.top_address,
        code,
        (void*)stack
    ));

    CTX_SYS_A0(&thread->ctx) = arg;

    auto node = &thread->tree;
    if (threads.root) {
        auto parent = threads.root;
        for (;;) {
            auto dir = parent < node;
            if (not parent->child[dir]) {
                threads.insert_at(parent, dir, node);
                break;
            }
            parent = parent->child[dir];
        }
    } else {
        threads.insert_root(node);
    }

    return thread.release();
}

result<Process*>
Process::fork()
{
    assert(&thread_cast(get_tls_current_thread())->process == this);

    auto to = TRY(owned<Process>::make());
    TRY(vm.clone(to->vm));
    TRY(descriptors.clone(to->descriptors));

    auto to_thread = TRY(owned<Thread>::make(*to));
    TRY_ERRC(cpu_context_clone_current(to_thread.get()));

    to_thread->page_map_top = to->vm.page_map.top_address;
    CTX_SYS_R0(&to_thread->ctx) = 0;
    to->threads.insert_root(&to_thread->tree);
    to->state = ACTIVE;

    schedule_ready(to_thread.release());
    return to.release();
}

Process& Thread::get_process() { return process; }
VM& Process::get_vm() { return vm; }
