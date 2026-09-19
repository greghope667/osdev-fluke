#include "memory.h"
#include "klib.h"
#include "x86_64/mmu.h"
#include "x86_64/cpu.h"
#include "x86_64/tls.h"

error_code
copy_to_user(void* dest, const void* src, isize size)
{
    TRY_ERRC(check_user_range(dest, size));
    int fault = memcpy_catch_fault(dest, src, size, MEMCPY_FAULT_WRITE);
    return fault ? EFAULT : 0;
}

error_code
copy_to_user8(void* dest, u64 value)
{
    return copy_to_user(dest, &value, sizeof(value));
}

error_code
copy_from_user(void* dest, const void* src, isize size)
{
    TRY_ERRC(check_user_range(src, size));
    int fault = memcpy_catch_fault(dest, src, size, MEMCPY_FAULT_READ);
    return fault ? EFAULT : 0;
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

int
memcpy_user_user_catch_fault(
    struct Thread_context* dest_thread,
    void* dest_addr,
    struct Thread_context* source_thread,
    const void* source_addr,
    isize size
) {
    if (check_user_range(source_addr, size) != 0)
        return MEMCPY_FAULT_READ;
    if (check_user_range(dest_addr, size) != 0)
        return MEMCPY_FAULT_WRITE;

    assert(this_tls->current_thread == nullptr);
    mmu_set_address_space_opt((struct Page_map){ dest_thread->page_map_top });
    struct Page_map source_map = { source_thread->page_map_top };

    usize count = size;
    while (count) {
        auto bytes_to_align = PAGE_SIZE - ((usize)source_addr & (PAGE_SIZE - 1));
        auto n = MIN(bytes_to_align, count);

        auto copy_src = phys_to_virt(virt_to_phys(source_map, (void*)source_addr));

        auto err = memcpy_catch_fault(
            dest_addr, copy_src, n,
            MEMCPY_FAULT_READ | MEMCPY_FAULT_WRITE
        );
        if (err) return err;

        source_addr += n;
        dest_addr += n;
        count -= n;
    }
    return 0;
}
