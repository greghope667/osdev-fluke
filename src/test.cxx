// #include "klib.hxx"
#include "mem/alloc.hxx"

static result<void>
run()
{
    auto i = TRY(owned<int>::make());
    auto j = TRY(owned<int>::make(3));
    return {};
}

extern "C" void
cpp_test_code()
{
    if (not run())
        panic("run failed");
}
