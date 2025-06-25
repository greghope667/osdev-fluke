#include "pmm.h"

#include "memory.h"
#include "panic.h"
#include "klib.h"

usize hhdm_offset;

struct Page {
    struct Page* next;
    char data[];
};

static struct PMM_state {
    struct Page* free_list;
    size_t free_count;
    size_t total_count;
} pmm;

void
pmm_add_pages(physical_t start, isize length)
{
    assert(is_page_aligned(start.address));
    assert(is_page_aligned(length));

    static struct Page** tail = &pmm.free_list;

    for (isize i=0; i<length; i+=PAGE_SIZE) {
        physical_t ppage = { start.address + i };
        struct Page* vpage = phys_to_virt(ppage);
        *tail = vpage;
        tail = &vpage->next;
    }

    *tail = nullptr;
    pmm.total_count += length / PAGE_SIZE;
    pmm.free_count  += length / PAGE_SIZE;
}

void*
alloc_page()
{
    struct Page* page = pmm.free_list;
    if (!page)
        return 0;

    pmm.free_list = page->next;
    assert(pmm.free_count > 0);
    pmm.free_count--;
    return page;
}

void
free_page(void *page)
{
    assert(is_page_aligned((usize)page));
    assert(is_kernel_pointer(page));
    struct Page* vpage = page;
    vpage->next = pmm.free_list;
    pmm.free_list = vpage;
    pmm.free_count++;
}

void
pmm_print_info()
{
    klog("pmm: pages free %zu used %zu total %zu\n",
        pmm.free_count, pmm.total_count-pmm.free_count, pmm.total_count);
    const usize page_per_M = (1024 * 1024) / PAGE_SIZE;
    klog("pmm: total %zu MiB\n", pmm.total_count / page_per_M);
}
