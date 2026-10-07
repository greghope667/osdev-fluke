#include <fluke/fluke.h>
#include <string.h>

#define SYSCALLV_N 0
#include "syscallv.h"

struct _fluke_user_shared_object*
_fluke_user_share()
{
    return (void*)_syscallv(SYSCALL_user_share);
}

const void*
_fluke_user_share_name(const char* name)
{
    auto share = _fluke_user_share();
    while (share->object) {
        if (strcmp(name, share->name) == 0)
            return share->object;
    }
    return nullptr;
}
