#include "process.hxx"

#include "mem/alloc.hxx"

result<Process*>
Process::create()
{
    auto process = TRY(owned<Process>::make());

    memset(process.get(), 0, sizeof(*process));
    process->state = SPAWNING;

    TRY(process->vm.init());

    process->thread.page_map_top = process->vm.page_map.top_address;

    return process.release();
}

Process&
Thread::get_process()
{
    return *container_of(this, Process, thread);
}

VM& Process::get_vm() { return vm; }
