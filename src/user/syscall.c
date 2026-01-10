#include "syscall.h"
#include "fluke.h"
#include "forth/forth.h"

usize
syscall(Context ctx)
{
    switch (CTX_SYS_OP(ctx)) {
        case SYSCALL_nop:
            return 0;

        case SYSCALL_forth_interpret:
            isize forth_stack[32];
            if (forth_interpret((const char*)CTX_SYS_A0(ctx), CTX_SYS_A1(ctx), forth_stack+1) >= 0) {
                CTX_SYS_R1(ctx) = forth_stack[1];
                return 0;
            } else {
                return -EIO;
            }

        default:
            return -ENOSYS;
    }
}
