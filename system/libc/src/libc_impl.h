#pragma once

int __libc_start_main(
    long* stack_vars,
    int (*main)(int, char**, char**),
    void (*init)(int, char**, char**),
    void (*fini)(void)
);

void __libc_do_atexit();
