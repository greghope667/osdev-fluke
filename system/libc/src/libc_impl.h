#pragma once

#define PRIVATE __attribute__((visibility("protected")))

int __libc_start_main(
    long* stack_vars,
    int (*main)(int, char**, char**),
    void (*init)(void),
    void (*fini)(void)
);

void __libc_do_atexit();

PRIVATE extern int __libc_working_dirfd;
PRIVATE void __libc_init_stdio();
