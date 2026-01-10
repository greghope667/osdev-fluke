#pragma once

void x86_64_apic_initialise();
void x86_64_apic_set_tickrate(int hz);
void x86_64_apic_send_eoi();

void x86_64_ioapic_initialise();
