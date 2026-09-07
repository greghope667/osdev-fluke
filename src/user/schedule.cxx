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
schedule_nanosleep(Thread_context* thread_ctx, i64 wait_ns)
{
    auto thread = static_cast<Thread*>(thread_ctx);

    assert(thread->state == FLOATING);
    assert(!thread->queue.root);

    thread_ctx->state = SLEEPING;
    assert(wait_ns > 0);
    set_timeout(thread, &sleep_queue, wait_ns);
}

void
schedule_queue_timeout(
    Thread_context* thread_ctx,
    Queue* queue,
    i64 wait_ns,
    usize timeout_return_value
) {
    auto thread = static_cast<Thread*>(thread_ctx);

    assert(thread->state == FLOATING);
    assert(!thread->queue.root);

    CTX_SYS_R0(&thread_ctx->ctx) = timeout_return_value;
    queue_push(queue, &thread->queue);
    thread->state = WAITING;
    assert(wait_ns > 0);
    set_timeout(thread, &sleep_queue, wait_ns);
}

void
schedule_wake(Thread_context* thread_ctx, usize return_value)
{
    auto thread = static_cast<Thread*>(thread_ctx);

    assert(thread->state == SLEEPING || thread->state == WAITING);
    assert(!thread->queue.root);

    thread->state = FLOATING;
    CTX_SYS_R0(&thread->ctx) = return_value;
    if (thread->timeout.prev) {
        *thread->timeout.prev = thread->timeout.next;
        thread->timeout = {};
    }
    schedule_ready(thread);
}

void
schedule_wake_all(Queue* queue, usize return_value)
{
    while (auto node = queue_pop(queue)) {
        schedule_wake(container_of(node, Thread, queue), return_value);
    }
}

static void
wakeup_sleepers()
{
    u64 now = nanoseconds();
    Thread* queued = sleep_queue;
    while (queued && queued->timeout.ns < now) {
        if (queued->state == WAITING) {
            queue_node_remove(&queued->queue);
        } else if (queued->state == SLEEPING) {
            // nothing to do
        } else {
            panic("Sleeping thread in illegal state");
        }

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
