#include "alloc.h"
#include "klib.h"
#include "panic.h"

int alloc_calc_size_class(usize);
void* kalloc(usize);
void kfree(void*, usize);

struct Free_list {
    struct Free_list* next;
    char data[];
};

struct Pool_data {
    size_t free_count;
    size_t total_count;
};

static struct Free_list* pools[10];
static struct Pool_data stats[10];

static void*
alloc_from_fresh_page(int size_class)
{
    assert(pools[size_class] == nullptr);

    void* page = alloc_page();
    if (!page)
        return nullptr;

    int entry_count = (PAGE_SIZE / 8) >> size_class;
    int size = 8 << size_class;

    struct Free_list* last = nullptr;
    for (int offset=size; offset<PAGE_SIZE; offset+=size) {
        struct Free_list* entry = page + offset;
        entry->next = last;
        last = entry;
    }
    pools[size_class] = last;
    stats[size_class].free_count += (entry_count - 1);
    stats[size_class].total_count += entry_count;

    return page;
}

void*
kalloc_class(int size_class)
{
    assert(0 <= size_class && size_class < 10);
    auto entry = pools[size_class];
    if (entry) {
        pools[size_class] = entry->next;
        stats[size_class].free_count--;
        return entry;
    } else {
        return alloc_from_fresh_page(size_class);
    }
}

void
kfree_class(void* ptr, int size_class)
{
    assert(0 <= size_class && size_class < 10);
    struct Free_list* entry = ptr;
    entry->next = pools[size_class];
    pools[size_class] = entry;
    stats[size_class].free_count++;
}

void
alloc_print_info()
{
    usize bytes_total = 0;
    usize bytes_free = 0;
    for (int i=0; i<10; i++) {
        usize size = 8 << i;
        usize fr = stats[i].free_count;
        usize to = stats[i].total_count;
        klog("alloc: class %zu free %zu used %zu total %zu\n",
            size, fr, to-fr, to);
        bytes_free += size * fr;
        bytes_total += size * to;
    }
    klog("alloc: bytes free %zu used %zu total %zu\n",
        bytes_free, bytes_total-bytes_free, bytes_total);
}
