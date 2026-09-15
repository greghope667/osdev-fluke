#pragma once

/*
 * Low-level Fluke system operations that are not directly exposed by the
 * POSIX api. These interfaces are non-portable and only semi-stable, so really
 * should only be used by core system servers & drivers
 *
 * All of these C functions return error codes by value, and don't write to
 * errno on failure
 */

#include <fluke/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

long    _fluke_forth_interpret(const char*) __NOTHROW;
void    _fluke_klog(const char*) __NOTHROW;
void*   _fluke_virtual_map(void* address, size_t len, int prot, int flags) __NOTHROW;
int     _fluke_irq_claim(int irq) __NOTHROW;
int     _fluke_irq_ack_wait(int irqd, long wait_ns) __NOTHROW;
void    _fluke_panic(const char* reason) __NORETURN;
int     _fluke_thread_spawn(void (*function)(long), void* stack, long arg) __NOTHROW;

#ifdef __cplusplus
} // extern C
#endif
