#include "share.h"
#include "klib.h"
#include "x86_64/mmu.h"

extern const char __usertext[];
extern const char __usertext_end[];
extern const char user_share_exec_elf[];

#define UCONST __attribute__((section(".userconst")))

static const UCONST struct user_shared_object objects[] = {
    { user_share_exec_elf, "exec_elf" },
    { 0, },
};

void
user_share_init()
{
    INIT_ONCE
    auto page_map = mmu_get_address_space();
    auto size = ROUND_UP_P2(__usertext_end - __usertext, PAGE_SIZE);
    mmu_edit(page_map, (usize)__usertext, size, MMU_MODE_USER|MMU_MODE_EXEC, MMU_CACHE_DEFAULT);
    mmu_set_address_space(page_map);
}

const struct user_shared_object*
user_share_get_objects()
{
    return objects;
}
