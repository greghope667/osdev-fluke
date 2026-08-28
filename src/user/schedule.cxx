#include "schedule.h"
#include "containers/queue.h"
#include "x86_64/time.h"
#include "process.hxx"
#include "x86_64/tls.h"

static Queue run_queue;
static Thread* sleep_queue;

void
schedule_ready(Thread_context* thread_ctx)
{
    assert(thread_ctx->state == FLOATING);
    thread_ctx->state = READY;
    queue_push(&run_queue, &static_cast<Thread*>(thread_ctx)->queue);
}

static Thread_context*
run_queue_pop()
{
    auto node = queue_pop(&run_queue);
    if (!node)
        return nullptr;

    auto proc = container_of(node, Thread, queue);
    assert(proc->state == READY);
    proc->state = FLOATING;
    return proc;
}

static void
set_timeout(Thread* __restrict__ thread, Thread** queue, u64 ns)
{
    assert(thread->timeout.prev == nullptr);
    assert(thread->timeout.next == nullptr);

    u64 timeout = ns + nanoseconds();
    thread->timeout.ns = timeout;

    for (Thread* next = *queue; next; next = *queue) {
        if (timeout < next->timeout.ns) {
            next->timeout.prev = &thread->timeout.next;
            break;
        }
        queue = &next->timeout.next;
    }
    thread->timeout.next = *queue;
    thread->timeout.prev = queue;
    *queue = thread;
}

void
schedule_nanosleep(Thread_context* thread_ctx, u64 wait_ns)
{
    assert(thread_ctx->state == FLOATING);
    thread_ctx->state = SLEEPING;
    assert((i64)wait_ns > 0);
    set_timeout(static_cast<Thread*>(thread_ctx), &sleep_queue, wait_ns);
}

static void
wakeup_sleepers()
{
    u64 now = nanoseconds();
    Thread* queued = sleep_queue;
    while (queued && queued->timeout.ns < now) {
        assert(queued->state = SLEEPING);
        queued->state = FLOATING;

        Thread* next = queued->timeout.next;

        queued->timeout.prev = nullptr;
        queued->timeout.next = nullptr;
        queued->timeout.ns = 0;
        schedule_ready(queued);
        klog("schedule.c: woke %p\n", queued);

        queued = next;
    }
    sleep_queue = queued;
    if (queued)
        queued->timeout.prev = &sleep_queue;
}

void
schedule()
{
    auto thread = get_tls_current_thread();
    if (thread) {
        cpu_context_save();
        schedule_ready(thread);
    }

    wakeup_sleepers();

    thread = run_queue_pop();
    if (thread)
        cpu_context_restore_and_exit(thread);
}

void
schedule_or_exit()
{
    schedule();
    cpu_exit_idle();
}
