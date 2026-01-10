#include "schedule.h"
#include "queue.h"
#include "x86_64/time.h"

static struct Queue run_queue;
static struct Process* sleep_queue;

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

static void
set_timeout(struct Process* restrict process, struct Process** queue, u64 ns)
{
    assert(process->timeout_prev == nullptr);
    assert(process->timeout_next == nullptr);

    u64 timeout = ns + nanoseconds();
    process->timeout_ns = timeout;

    for (struct Process* next = *queue; next; next = *queue) {
        if (timeout < next->timeout_ns) {
            next->timeout_prev = &process->timeout_next;
            break;
        }
        queue = &next->timeout_next;
    }
    process->timeout_next = *queue;
    process->timeout_prev = queue;
    *queue = process;
}

void
schedule_nanosleep(struct Process* process, u64 wait_ns)
{
    assert(process->state == FLOATING);
    process->state = SLEEPING;
    assert((i64)wait_ns > 0);
    set_timeout(process, &sleep_queue, wait_ns);
}

static void
wakeup_sleepers()
{
    u64 now = nanoseconds();
    struct Process* queued = sleep_queue;
    while (queued && queued->timeout_ns < now) {
        assert(queued->state = SLEEPING);
        queued->state = FLOATING;

        struct Process* next = queued->timeout_next;

        queued->timeout_prev = nullptr;
        queued->timeout_next = nullptr;
        queued->timeout_ns = 0;
        schedule_ready(queued);
        klog("schedule.c: woke %p\n", queued);

        queued = next;
    }
    sleep_queue = queued;
    if (queued)
        queued->timeout_prev = &sleep_queue;
}

void
schedule()
{
    struct Process* proc = this_cpu->process;
    if (proc) {
        cpu_context_save();
        schedule_ready(proc);
    }

    wakeup_sleepers();

    proc = run_queue_pop();
    if (proc)
        cpu_context_restore_and_exit(proc);
}

void
schedule_or_exit()
{
    schedule();
    cpu_exit_idle();
}
