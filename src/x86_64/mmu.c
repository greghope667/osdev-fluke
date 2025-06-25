#include "mmu.h"
#include "mem/alloc.h"
#include "klib.h"

static void* mmu_alloc_page()
{
    void* page = alloc_page();
    if (!page) panic("OOM in mmu_alloc_page (TODO: fix this)");
    return memset(page, 0, PAGE_SIZE);
}

void
mmu_set_address_space(struct Page_map pm)
{
    asm volatile("mov %0, %%cr3" : : "r"(pm.top_address.address));
}

struct Page_map
mmu_get_address_space()
{
    struct Page_map pm;
    asm ("mov %%cr3, %0" : "=r"(pm.top_address.address));
    return pm;
}

void
mmu_reload_address_space()
{
    usize scratch;
    asm volatile ("mov %%cr3, %0\nmov %0, %%cr3" : "=r"(scratch));
}

struct Page_map
mmu_create_address_space()
{
    void* page = mmu_alloc_page();
    const void* current = phys_to_virt(mmu_get_address_space().top_address);
    const int HALF = PAGE_SIZE / 2;
    memcpy(page + HALF, current + HALF, HALF);
    return (struct Page_map){ hhdm_virt_to_phys(page) };
}

void
mmu_destroy_address_space(struct Page_map pm)
{
    assert(pm.top_address.address != mmu_get_address_space().top_address.address);
    mmu_clear(pm, 0, MEM_LOW_HALF_MAX);
    free_page(phys_to_virt(pm.top_address));
}

typedef enum table_entry : u64 {
    PRESENT         = 1 << 0,
    WRITE           = 1 << 1,
    USER            = 1 << 2,
    WRITE_THROUGH   = 1 << 3,
    CACHE_DISABLE   = 1 << 4,
    PAT_PTE         = 1 << 7,
    AVL0            = 1 << 9,
    NO_EXECUTE      = 1ull << 63,

    INTERMEDIATE    = PRESENT | WRITE | USER,
} table_entry_t;

#define SHARED AVL0

#define TABLE_ENTRY_COUNT 512
#define TABLE_ENTRY_ADDRESS_MASK    0x000f'ffff'ffff'f000ull

struct Translation_table {
    alignas(PAGE_SIZE) table_entry_t entries[TABLE_ENTRY_COUNT];
};

_Static_assert(sizeof(struct Translation_table) == PAGE_SIZE);
_Static_assert(alignof(struct Translation_table) == PAGE_SIZE);

static inline enum table_entry
entry_address_bits(void* page)
{
    return hhdm_virt_to_phys(page).address & TABLE_ENTRY_ADDRESS_MASK;
}

static inline void*
entry_target(table_entry_t entry)
{
    return phys_to_virt((physical_t){ entry & TABLE_ENTRY_ADDRESS_MASK });
}

static enum table_entry
entry_flags(enum mmu_cache cache, enum mmu_mode mode)
{
    table_entry_t entry = PRESENT;
    if (!(mode & MMU_MODE_EXEC))    entry |= NO_EXECUTE;
    if (mode & MMU_MODE_USER)       entry |= USER;
    if (mode & MMU_MODE_WRITE)      entry |= WRITE;
    if (cache & MMU_CACHE_NONE)     entry |= (WRITE_THROUGH | CACHE_DISABLE);
    return entry;
}

enum depth {
    PML4 = 4,
    PDPT = 3,
    PDT = 2,
    PT = 1,
};

static inline int
virt_shift(enum depth d)
{
    return 3 + 9 * d;
}

static inline int
table_index(enum depth d, usize address)
{
    return 0x1ff & (address >> virt_shift(d));
}

physical_t
virt_to_phys(struct Page_map pm, void* virtual)
{
    static const table_entry_t PS_BIT = 1 << 7;
    usize address = (usize)virtual;

    struct Translation_table* table;
    table_entry_t entry;

    table = phys_to_virt(pm.top_address);
    entry = table->entries[table_index(PML4, address)];
    if (!(entry & PRESENT))
        return (physical_t){ 0 };

    table = phys_to_virt((physical_t){ entry & TABLE_ENTRY_ADDRESS_MASK });
    entry = table->entries[table_index(PDPT, address)];
    if (!(entry & PRESENT))
        return (physical_t){ 0 };

    if (entry & PS_BIT) // 1 GB page
        return (physical_t){ (entry & 0x000f'ffff'c000'0000ull) | (address & 0x3fff'ffff) };

    table = phys_to_virt((physical_t){ entry & TABLE_ENTRY_ADDRESS_MASK });
    entry = table->entries[table_index(PDT, address)];
    if (!(entry & PRESENT))
        return (physical_t){ 0 };

    if (entry & PS_BIT) // 2 MB page
        return (physical_t){ (entry & 0x000f'ffff'ffe0'0000ull) | (address & 0x1f'ffff) };

    table = phys_to_virt((physical_t){ entry & TABLE_ENTRY_ADDRESS_MASK });
    entry = table->entries[table_index(PT, address)];
    if (!(entry & PRESENT))
        return (physical_t){ 0 };

    // 4 KB page
    return (physical_t){ (entry & 0x000f'ffff'ffff'f000ull) | (address & 0xfff) };
}

static void
assign(
    struct Translation_table* table,
    enum depth depth,
    usize vbegin,
    usize vend,
    table_entry_t flags
) {
    int shift = virt_shift(depth);
    usize pointee_size = 1ull << shift;

    usize base = ROUND_DOWN_P2(vbegin, TABLE_ENTRY_COUNT * pointee_size);
    int ibegin = (ROUND_DOWN_P2(vbegin, pointee_size) - base) >> shift;
    int iend = (ROUND_UP_P2(vend, pointee_size) - base) >> shift;

    for (isize i=ibegin; i<iend; i++) {
        usize next_vbegin = MAX((i << shift) + base, vbegin);
        usize next_vend = MIN(((i+1) << shift) + base, vend);
        table_entry_t* entry = &table->entries[i];

        if (depth > PT) {
            void* next_page;
            if (!(*entry & PRESENT)) {
                next_page = mmu_alloc_page();
                *entry = entry_address_bits(next_page) | INTERMEDIATE;
            } else {
                next_page = entry_target(*entry);
            }
            assign(next_page, depth-1, next_vbegin, next_vend, flags);
        } else {
            if (*entry & PRESENT)
                panic("mmu: cannot assign over existing page table entry");
            void* mem = mmu_alloc_page();
            *entry = entry_address_bits(mem) | flags;
        }
    }
}

static bool
clear(
    struct Translation_table* table,
    enum depth depth,
    usize vbegin,
    usize vend
) {
    int shift = virt_shift(depth);
    usize pointee_size = 1ull << shift;

    usize base = ROUND_DOWN_P2(vbegin, TABLE_ENTRY_COUNT * pointee_size);
    int ibegin = (ROUND_DOWN_P2(vbegin, pointee_size) - base) >> shift;
    int iend = (ROUND_UP_P2(vend, pointee_size) - base) >> shift;

    for (isize i=ibegin; i<iend; i++) {
        usize next_vbegin = MAX((i << shift) + base, vbegin);
        usize next_vend = MIN(((i+1) << shift) + base, vend);
        table_entry_t* entry = &table->entries[i];

        if (*entry & PRESENT) {
            void* target = entry_target(*entry);
            if (depth == PT || clear(target, depth-1, next_vbegin, next_vend)) {
                free_page(target);
                *entry = 0;
            }
        }
    }

    for (int i=0; i<TABLE_ENTRY_COUNT; i++) {
        if (table->entries[i] & PRESENT) return false;
    }
    return true;
}

static void
validate_limits(usize address, isize len)
{
    assert(((address | len) & (PAGE_SIZE - 1)) == 0);
    assert(len > 0);

    if (address <= MEM_LOW_HALF_MAX) {
        assert(address + len <= MEM_LOW_HALF_MAX);
    } else {
        assert(address >= MEM_HIGH_HALF_MIN);
        assert(address + len >= MEM_HIGH_HALF_MIN);
    }
}

void
mmu_assign(
    struct Page_map pm,
    usize address,
    isize len,
    enum mmu_mode mode,
    enum mmu_cache cache
) {
    validate_limits(address, len);
    assign(phys_to_virt(pm.top_address), PML4, address, address + len, entry_flags(cache, mode));
}

void
mmu_clear(
    struct Page_map pm,
    usize address,
    isize len
) {
    validate_limits(address, len);
    clear(phys_to_virt(pm.top_address), PML4, address, address + len);
}
