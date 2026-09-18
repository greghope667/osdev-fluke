#include <fluke/defs/syscalls.h>

/* Define SYSCALLV_N and SYSCALLV_O, then #include this file.
 * Don't read this file though, it's cursed
 */

#ifndef SYSCALLV_N
#define SYSCALLV_N 6
#endif

#ifndef SYSCALLV_O
#define SYSCALLV_O 0
#endif

#define CONCAT_(a, b) a ## b
#define CONCAT(a, b) CONCAT_(a, b)

#undef _syscallv
#if SYSCALLV_O
#define _syscallv CONCAT(_syscallv_o, SYSCALLV_N)
#else
#define _syscallv CONCAT(_syscallv_, SYSCALLV_N)
#endif

static __attribute__((always_inline)) inline
long
_syscallv(
#if SYSCALLV_O
    long* out,
#endif
    long op
#if SYSCALLV_N > 0
    , long _1
#if SYSCALLV_N > 1
    , long _2
#if SYSCALLV_N > 2
    , long _3
#if SYSCALLV_N > 3
    , long _4
#if SYSCALLV_N > 4
    , long _5
#if SYSCALLV_N > 5
    , long _6
#endif
#endif
#endif
#endif
#endif
#endif
) {
#if SYSCALLV_N < 3
    long _3 = 0;
#else
#if SYSCALLV_N > 3
    register long r10 asm("r10") = _4;
#if SYSCALLV_N > 4
    register long r8 asm("r8") = _5;
#if SYSCALLV_N > 5
    register long r9 asm("r9") = _6;
#endif
#endif
#endif
#endif

    asm volatile (
        "syscall"
        : "+a"(op), "+d"(_3)
        :
#if SYSCALLV_N > 0
            "D"(_1)
#if SYSCALLV_N > 1
            ,"S"(_2)
#if SYSCALLV_N > 2
#if SYSCALLV_N > 3
            ,"r"(r10)
#if SYSCALLV_N > 4
            ,"r"(r8)
#if SYSCALLV_N > 5
            ,"r"(r9)
#endif
#endif
#endif
#endif
#endif
#endif
        : "memory", "rcx", "r11"
    );
#if SYSCALLV_O
    *out = _3;
#endif
    return op;
}
