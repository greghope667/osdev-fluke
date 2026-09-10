#include "memory.h"
#include "klib.h"

error_code
copy_to_user(void* dest, const void* src, isize size)
{
    TRY_ERRC(check_user_range(dest, size));

    // TODO: handle potential page faults
    memcpy(dest, src, size);
    return 0;
}

error_code
copy_from_user(void* dest, const void* src, isize size)
{
    TRY_ERRC(check_user_range(src, size));

    // TODO: handle potential page faults
    memcpy(dest, src, size);
    return 0;
}

error_code
check_user_range(const void* v, isize len)
{
    return (
        is_user_pointer(v)
        && is_user_pointer(v + len - 1)
        && len > 0
    ) ? 0 : EFAULT;
}
