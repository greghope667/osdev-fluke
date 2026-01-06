#include "schedule.h"
#include "queue.h"

static struct Queue run_queue;

void
schedule_ready(struct Process* process)
{
    assert(process->state == FLOATING);
    process->state = READY;
    queue_push(&run_queue, &process->queue);
}

static struct Process* run_queue_pop()
{
    struct Queue_node* node = queue_pop(&run_queue);
    if (!node)
        return nullptr;

    struct Process* proc = container_of(node, struct Process, queue);
    assert(proc->state == READY);
    proc->state = FLOATING;
    return proc;
}

void
schedule()
{
    struct Process* proc = this_cpu->process;
    if (proc) {
        cpu_context_save();
        schedule_ready(proc);
    }

    proc = run_queue_pop();
    if (proc)
        cpu_context_restore_and_exit(proc);
}
