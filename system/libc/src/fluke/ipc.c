#include <fluke/fluke.h>

#define SYSCALLV_N 2
#define SYSCALLV_O 1
#include "syscallv.h"

struct _fluke_ipair
_fluke_ipc_create(
    unsigned char transfer_map[],
    unsigned ntransfer_map
) {
    long handle_out;
    int ret = _syscallv(
        &handle_out,
        SYSCALL_ipc_create,
        (long)transfer_map,
        ntransfer_map
    );
    return (struct _fluke_ipair){ ret, handle_out };
}

#undef SYSCALLV_O
#undef SYSCALLV_N
#define SYSCALLV_N 4
#include "syscallv.h"

int
_fluke_ipc_respond(long status, long aptr, long alen, int mode)
{
    return _syscallv(
        SYSCALL_ipc_respond,
        status, aptr, alen, mode
    );
}
