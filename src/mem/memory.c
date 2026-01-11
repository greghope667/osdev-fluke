#include "memory.h"
#include "klib.h"
#include "errno.h"

bool
copy_to_user(void* dest, const void* src, isize size)
{
    if (
        is_user_pointer(dest)
        && is_user_pointer(dest + size)
        && size > 0
    ){
        // TODO: handle potential page faults
        memcpy(dest, src, size);
        return true;
    } else {
        errno = EFAULT;
        return false;
    }
}

bool
copy_from_user(void* dest, const void* src, isize size)
{
    if (
        is_user_pointer(src)
        && is_user_pointer(src + size)
        && size > 0
    ){
        // TODO: handle potential page faults
        memcpy(dest, src, size);
        return true;
    } else {
        errno = EFAULT;
        return false;
    }
}
