#include "process.hxx"

#include "mem/alloc.hxx"

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
    auto thread = TRY(owned<Thread>::make(Thread{.process = this}));

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

Process& Thread::get_process() { return *process; }
VM& Process::get_vm() { return vm; }
