#include <stdlib.h>
#include <unistd.h>

__NORETURN void
abort()
{
    static const char message[] = "abort() called\n";
    write(2, message, sizeof(message)-1);
    __builtin_trap();
    // TODO: SIGABRT
}
