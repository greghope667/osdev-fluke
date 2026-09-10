#include "handle.hxx"

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
    return error_code(EPIPE);
}

result<usize>
Handle::ctl(Context, unsigned)
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
