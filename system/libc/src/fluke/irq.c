#include <fluke/fluke.h>
#include <fluke/defs/fluke.h>
#include "syscall6.h"

int
_fluke_irq_claim(int irq)
{
    return syscall6(
        SYSCALL_claim_irq,
        irq,
        0, 0, 0, 0, 0
    );
}

int
_fluke_irq_ack_wait(int irqd, long wait_ns)
{
    return syscall6(
        SYSCALL_objctl,
        irqd,
        IRQ_CTL_WAIT | IRQ_CTL_ENABLE,
        wait_ns,
        0, 0, 0
    );
}
