#pragma once

#ifdef __cplusplus

#include "klib.hxx"
result<struct Handle*> irq_claim(int irq);

extern "C" {
#endif

void user_on_irq_receive(int irq);

#ifdef __cplusplus
} // extern C
#endif
