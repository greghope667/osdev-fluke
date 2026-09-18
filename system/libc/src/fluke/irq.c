#include <fluke/fluke.h>
#include <fluke/defs/fluke.h>

#define SYSCALLV_N 1
#include "syscallv.h"

int
_fluke_irq_claim(int irq)
{
    return _syscallv(
        SYSCALL_claim_irq,
        irq
    );
}

#undef SYSCALLV_N
#define SYSCALLV_N 3
#include "syscallv.h"

int
_fluke_irq_ack_wait(int irqd, long wait_ns)
{
    return _syscallv(
        SYSCALL_objctl,
        irqd,
        IRQ_CTL_WAIT | IRQ_CTL_ENABLE,
        wait_ns
    );
}
