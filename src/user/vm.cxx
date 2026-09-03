#include "vm.hxx"

#include "klib.h"
#include "mem/alloc.hxx"
#include "mem/memory.h"
#include "x86_64/mmu.h"

static constexpr usize ADDR_MIN = PAGE_SIZE;
static constexpr usize ADDR_MAX = MEM_LOW_HALF_MAX;
static constexpr usize ADDR_DEFAULT_HINT = 0xaaabbbull * 1024 * 1024;

struct VM_area {
    VM_area* next;
    usize begin;
    usize end;
    unsigned flags;

    isize space_after() {
        auto end_of_space = next ? next->begin : ADDR_MAX;
        return end_of_space - end;
    }

    enum Overlap {
        After, Starts_with, Contains, Whole, Ends_with, Before
    };

    Overlap find_overlap(usize ibegin, usize iend) {
        if (iend <= begin)
            return After;
        if (ibegin >= end)
            return Before;
        bool contains_start = ibegin <= begin;
        bool contains_end = iend >= end;
        if (contains_start)
            return contains_end ? Whole : Starts_with;
        else
            return contains_end ? Ends_with : Contains;
    }
};

result<void>
VM::init()
{
    first = {};
    TRY_ERRC(mmu_create_address_space(&page_map));
    return {};
}

static void
assert_valid(usize address, isize length)
{
    assert(
        is_page_aligned(address)
        && is_page_aligned(length)
        && length > 0
    );
}

static result<VM_area**>
vm_clear_for_insert(VM& vm, usize address, usize end)
{
    auto spare = TRY(owned<VM_area>::make());

    auto* area = vm.first;
    auto** route = &vm.first;

    for (;;) {
        assert(*route == area);

        if (not area)
            return route;

        switch (area->find_overlap(address, end)) {
        case VM_area::After:
            return route;

        case VM_area::Starts_with:
            area->begin = end;
            assert(area->begin < area->end);
            return route;

        case VM_area::Contains:
            // Split area into two, use pre-allocated spare
            *spare = *area;
            spare->begin = end;
            area->end = address;
            area->next = spare.release();
            return route;

        case VM_area::Whole:
            *route = area->next;
            area = [area]{
                auto next = area->next;
                kfree_t(area);
                return next;
            }();
            continue;

        case VM_area::Ends_with:
            area->end = address;
            assert(area->begin < area->end);
            route = &area->next;
            area = area->next;
            continue;

        case VM_area::Before:
            route = &area->next;
            area = area->next;
            continue;
        }

        __builtin_unreachable();
    }
    return route;
}

result<void>
VM::free(usize address, isize length)
{
    assert_valid(address, length);
    if (address >= ADDR_MAX || address+length > ADDR_MAX)
        return error_code(EINVAL);

    TRY(vm_clear_for_insert(*this, address, address+length));
    mmu_clear(page_map, address, length);

    return {};
}

static result<VM_area*>
vm_insert_area(VM& vm, usize address, usize end)
{
    auto new_area = TRY(owned<VM_area>::make());
    *new_area = {
        .next = {},
        .begin = address,
        .end = end,
        .flags = {},
    };

    auto* next = vm.first;
    auto** route = &vm.first;

    for (;;) {
        assert(*route == next);
        if (not next) {
            *route = new_area.release();
            return *route;
        }
        if (address < next->end)
            break;
        route = &next->next;
        next = next->next;
    }

    if (address >= next->begin || end > next->begin)
        return error_code(EEXIST);

    new_area->next = next;
    *route = new_area.release();
    return *route;
}

static mmu_mode
prot_to_mode(unsigned prot_flags)
{
    unsigned mode = MMU_MODE_DEFAULT;
    if (prot_flags)
        mode |= MMU_MODE_USER;
    if (prot_flags & PROT_EXEC)
        mode |= MMU_MODE_EXEC;
    if (prot_flags & PROT_WRITE)
        mode |= MMU_MODE_WRITE;
    return mmu_mode(mode);
}

result<void>
VM::alloc_fixed_noreplace(usize address, isize length, unsigned flags)
{
    assert_valid(address, length);
    if (
        address < ADDR_MIN
        || address >= ADDR_MAX
        || address + length > ADDR_MAX
    )
        return error_code(EINVAL);

    auto mode = prot_to_mode(flags);

    auto area = TRY(vm_insert_area(*this, address, address+length));
    area->flags = mode;

    if (flags)
        mmu_assign(page_map, address, length, mode, MMU_CACHE_DEFAULT);

    return {};
}

result<void>
VM::alloc_fixed_overwrite(usize address, isize length, unsigned flags)
{
    assert_valid(address, length);
    if (
        address < ADDR_MIN
        || address >= ADDR_MAX
        || address + length > ADDR_MAX
    )
        return error_code(EINVAL);

    auto mode = prot_to_mode(flags);

    auto route = TRY(vm_clear_for_insert(*this, address, address+length));
    mmu_clear(page_map, address, length);

    auto area = TRY(owned<VM_area>::make());
    *area = {
        .next = *route,
        .begin = address,
        .end = address+length,
        .flags = mode,
    };

    *route = area.release();

    if (flags)
        mmu_assign(page_map, address, length, mode, MMU_CACHE_DEFAULT);

    return {};
}

result<char*>
VM::alloc_movable(usize hint, isize length, unsigned flags)
{
    if (hint < ADDR_MIN || hint >= ADDR_MAX)
        hint = ADDR_DEFAULT_HINT;

    if (hint + length > ADDR_MAX)
        return error_code(EINVAL);

    VM_area vm_list_begin = {
        .next = first,
        .begin = ADDR_MIN,
        .end = ADDR_MIN,
        .flags = (unsigned)-1,
    };

    auto new_area = TRY(owned<VM_area>::make());
    VM_area* region{};

    for (auto* area = &vm_list_begin; area; area = area->next) {
        if (area->space_after() >= length) {
            if (area->end < hint) {
                region = area;
            } else {
                if (not region || area->end - hint < hint - region->end)
                    region = area;
                break;
            }
        }
    }

    if (not region)
        return error_code(ENOMEM);

    auto address = std::clamp(hint, region->end, region->next ? region->next->begin : ADDR_MAX);
    assert_valid(address, length);
    auto mode = prot_to_mode(flags);

    *new_area = {
        .next = region->next,
        .begin = address,
        .end = address + length,
        .flags = mode,
    };

    if (flags)
        mmu_assign(page_map, address, length, mode, MMU_CACHE_DEFAULT);

    region->next = new_area.release();
    first = vm_list_begin.next;

    return (char*)address;
}

static void
print_range(VM_area& a)
{
    char flags[4] = "---";
    if (a.flags & MMU_MODE_USER)  flags[0] = 'u';
    if (a.flags & MMU_MODE_WRITE) flags[1] = 'w';
    if (a.flags & MMU_MODE_EXEC)  flags[2] = 'x';
    printf("    %016zx - %016zx  %s\n", a.begin, a.end, flags);
}

void
VM::print()
{
    printf("VM page_map %016zx\n", page_map.top_address.address);
    for (auto a = first; a; a = a->next)
        print_range(*a);
}
