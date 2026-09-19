#include "schedule.h"
#include "containers/queue.h"
#include "x86_64/time.h"
#include "process.hxx"
#include "x86_64/tls.h"

static Queue run_queue;
static Thread* sleep_queue;

[[maybe_unused]] static void
print_sleep_queue(const char* from)
{
    printf("%s %s %zu\n", from, __PRETTY_FUNCTION__, nanoseconds() / 1'000'000);
    for (auto thread = sleep_queue; thread; thread = thread->timeout.next) {
        printf("    %p %zu\n", thread, thread->timeout.ns / 1'000'000);
    }
}

void
schedule_ready(Thread_context* thread_ctx)
{
    assert(thread_ctx->state == FLOATING);
    thread_ctx->state = READY;
    queue_push(&run_queue, &thread_cast(thread_ctx)->queue);
}

static Thread_context*
run_queue_pop()
{
    auto node = queue_pop(&run_queue);
    if (!node)
        return nullptr;

    auto thread = thread_cast(node);
    assert(thread->state == READY);
    thread->state = FLOATING;
    return thread;
}

static void
set_timeout(Thread* __restrict__ thread, Thread** queue, isize ns)
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
schedule_nanosleep(Thread_context* thread_ctx, isize wait_ns)
{
    auto thread = thread_cast(thread_ctx);

    assert(thread->state == FLOATING);
    assert(!thread->queue.root);

    thread_ctx->state = SLEEPING;
    assert(wait_ns > 0);
    set_timeout(thread, &sleep_queue, wait_ns);
}

void
schedule_queue_with_timeout(
    Thread_context* thread_ctx,
    Queue* queue,
    i64 wait_ns,
    error_code timeout_return_value
) {
    assert(wait_ns > 0);
    CTX_SYS_R0(&thread_ctx->ctx) = -int(timeout_return_value);
    auto thread = thread_cast(thread_ctx);
    thread->push_into(queue);
    set_timeout(thread, &sleep_queue, wait_ns);
}

void
schedule_wake(Thread_context* thread_ctx, usize return_value)
{
    auto thread = thread_cast(thread_ctx);

    assert(thread->state == SLEEPING || thread->state == WAITING);
    assert(!thread->queue.root);

    thread->state = FLOATING;
    CTX_SYS_R0(&thread->ctx) = return_value;
    thread->timeout.cancel();
    schedule_ready(thread);
}

void
schedule_wake_all(Queue* queue, usize return_value)
{
    while (auto node = queue_pop(queue)) {
        schedule_wake(thread_cast(node), return_value);
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
            assert(!queued->queue.root);
        } else {
            panic("Sleeping thread in illegal state");
        }

        queued->state = FLOATING;

        Thread* next = queued->timeout.next;

        queued->timeout = {};
        schedule_ready(queued);
        // klog("schedule.c: woke %p\n", queued);

        queued = next;
    }
    sleep_queue = queued;
    if (queued)
        queued->timeout.prev = &sleep_queue;
}

void
schedule()
{
    wakeup_sleepers();

    auto thread = run_queue_pop();
    if (not thread)
        return;

    if (auto running = get_tls_current_thread()) {
        cpu_context_save();
        schedule_ready(running);
    }

    cpu_context_restore_and_exit(thread);
}

void
schedule_or_exit()
{
    schedule();
    cpu_exit_idle();
}
