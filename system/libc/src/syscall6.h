#pragma once

#include <fluke/defs/syscalls.h>

static inline long
syscall6(long op, long _1, long _2, long _3, long _4, long _5, long _6)
{
    register long r9 asm("r9") = _6;
    register long r8 asm("r8") = _5;
    register long r10 asm("r10") = _4;
    asm volatile (
        "syscall"
        : "+a"(op), "+d"(_3)
        : "D"(_1), "S"(_2), "r"(r10), "r"(r8), "r"(r9)
        : "memory", "rcx", "r11"
    );
    return op;
}
