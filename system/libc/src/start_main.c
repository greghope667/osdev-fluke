#include <stdlib.h>
#include "libc_impl.h"

int
__libc_start_main(
    long* stack_vars,
    int (*main)(int, char**, char**),
    void (*init)(int, char**, char**),
    void (*fini)(void)
) {
    int argc = *stack_vars++;
    char** argv = (char**)stack_vars;
    char** envp = &argv[argc + 1];

    init(argc, argv, envp);
    atexit(fini);

    return main(argc, argv, envp);
}
