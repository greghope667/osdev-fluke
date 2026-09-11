#include "irq.h"
#include "user/handle.hxx"
#include <fluke/defs/fluke.h>
#include "process.hxx"
#include "x86_64/apic.h"
#include "schedule.h"

static constexpr int IRQ_MAX = 15;

struct IRQ_handle final : Handle {
    result<usize> ctl(Context ctx, unsigned op) override;
    void close() override;
    bool is_claimed = {};
    bool interrupt_occurred = {};
    Queue waiting = {};
};

constinit static IRQ_handle handlers[IRQ_MAX + 1] = {};

result<Handle*>
irq_claim(int irq)
{
    if (irq < 0 || irq > IRQ_MAX)
        return error_code(EINVAL);

    auto& handle = handlers[irq];
    if (handle.is_claimed)
        return error_code(EBUSY);

    handle.is_claimed = true;
    return &handle;
}

result<usize>
IRQ_handle::ctl(Context ctx, unsigned op)
{
    assert(is_claimed);

    if (op & ~(IRQ_CTL_ENABLE|IRQ_CTL_WAIT))
        return error_code(EINVAL);

    int irq = this - handlers;

    if (op & IRQ_CTL_ENABLE) {
        x86_64_ioapic_enable_irq(irq, true);
    }

    if (op & IRQ_CTL_WAIT) {
        if (interrupt_occurred) {
            interrupt_occurred = false;
            return 0;
        }

        isize timeout = CTX_SYS_A2(ctx);
        if (timeout <= 0)
            return error_code(EAGAIN);

        schedule_queue_with_timeout(
            cpu_context_save(),
            &waiting,
            timeout,
            error_code(EAGAIN)
        );
        schedule_or_exit();
    }

    return 0;
}

void
user_on_irq_receive(int irq)
{
    if (irq < 0 || irq > IRQ_MAX)
        return;

    auto& handle = handlers[irq];

    if (auto node = queue_pop(&handle.waiting)) {
        auto thread = thread_cast(node);
        schedule_wake(thread, 1);
    } else {
        handle.interrupt_occurred = true;
    }
}

void
IRQ_handle::close()
{
    is_claimed = false;
    schedule_wake_all(&waiting, -EBADF);
}
