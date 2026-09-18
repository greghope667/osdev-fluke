#include "handle.hxx"

#include "x86_64/cpu.h"

result<isize>
Handle::read(void*, isize)
{
    return error_code(EINVAL);
}

result<isize>
Handle::write(const void*, isize)
{
    return error_code(EINVAL);
}

result<isize>
Handle::seek(isize, int)
{
    return error_code(ESPIPE);
}

result<usize>
Handle::ctl(Context, unsigned)
{
    return error_code(EINVAL);
}

error_code
Handle::ipc_call(Context)
{
    return error_code(EINVAL);
}

void
Handle::release()
{
    assert(ref_count_ > 0);
    if (--ref_count_ == 0)
        close();
}
