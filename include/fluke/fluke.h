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

struct _fluke_lpair { long first; long second; };
struct _fluke_ipair { int  first; long second; };

long    _fluke_forth_interpret(const char*);
void    _fluke_klog(const char*);
void*   _fluke_virtual_map(void* address, size_t len, int prot, int flags);
int     _fluke_irq_claim(int irq);
int     _fluke_irq_ack_wait(int irqd, long wait_ns);
void    _fluke_panic(const char* reason) __NORETURN;
int     _fluke_thread_spawn(void (*function)(long), void* stack, long arg);
void    _fluke_nsleep(long nanoseconds);

typedef int (*_fluke_ipc_callback)(
    long handle, long aptr, long alen,
    int mode, long ax1, long ax2
);

struct _fluke_ipair
        _fluke_ipc_create(signed char transfer_map[], unsigned ntransfer_map);

struct _fluke_lpair
        _fluke_ipc_call(int fd, long aptr, long alen, int mode, ...);

int     _fluke_ipc_listen(
            long handle, long aptr, long alen, int channel,
            _fluke_ipc_callback callback);

int     _fluke_ipc_respond(long status, long aptr, long alen, int mode);

#ifdef __cplusplus
} // extern C
#endif
