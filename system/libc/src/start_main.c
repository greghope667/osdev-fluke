#include <stdlib.h>
#include "libc_impl.h"

extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);
extern void (*__fini_array_start[])(void);
extern void (*__fini_array_end[])(void);

static void
__libc_do_init_array()
{
    for (void (**f)(void) = __init_array_start; f != __init_array_end; f++) {
        f[0]();
    }
}

static void
__libc_do_fini_array()
{
    for (void (**f)(void) = __fini_array_end; f != __fini_array_start; f--) {
        f[-1]();
    }
}

int
__libc_start_main(
    long* stack_vars,
    int (*main)(int, char**, char**),
    void (*init)(void),
    void (*fini)(void)
) {
    init();
    atexit(fini);

    __libc_do_init_array();
    atexit(__libc_do_fini_array);

    int argc = *stack_vars++;
    char** argv = (char**)stack_vars;
    char** envp = &argv[argc + 1];

    return main(argc, argv, envp);
}
