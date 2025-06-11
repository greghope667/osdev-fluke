#pragma once

void panic(const char* reason) __attribute__((noreturn));
void show_backtrace(void* frame);
void show_backtrace_here();

#define assert(x) if (!(x)) panic("assertion failure")

#define INIT_ONCE {                         \
    static bool _done = false;              \
    if (_done) panic("INIT_ONCE check");    \
    _done = true;                           \
}
