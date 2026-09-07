#pragma once

#ifdef __cplusplus
extern "C" {
#endif
void x86_64_apic_initialise();
void x86_64_apic_set_tickrate(int hz);
void x86_64_apic_send_eoi();

void x86_64_ioapic_initialise();
void x86_64_ioapic_enable_irq(int irq, bool enable);
int x86_64_ioapic_isr_to_irq(int isr);

#ifdef __cplusplus
} // extern C
#endif
