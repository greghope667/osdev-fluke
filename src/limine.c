#include "bootloader.h"
#include "print/console.h"
#include "klib.h"
#include "mem/memory.h"
#include "mem/pmm.h"

#include <limine.h>

/* INFO EXTRACTED

framebuffer: address, width, height, pitch (dpp=32)
memory map: []{phys, size}
modules: []{address, size, path, cmdline}
rsdp: phys
command line: string
smp: ???
hhdm: offset
stacks: ???
*/

const volatile struct limine_framebuffer_request
limine_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 1,
};

const volatile struct limine_hhdm_request
limine_hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

const volatile struct limine_memmap_request
limine_memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

void
bootloader_init_display()
{
    auto response = limine_framebuffer_request.response;
    if (!response)
        panic("no bootloader framebuffer");
    if (response->framebuffer_count == 0)
        panic("no framebuffers present");

    auto framebuffer = response->framebuffers[0];
    if (framebuffer->bpp != 32)
        panic("32bpp framebuffer required");

    console_init(&(struct Framebuffer){
        .address = framebuffer->address,
        .width = framebuffer->width,
        .height = framebuffer->height,
        .pitch = framebuffer->pitch,
    });
}

void
bootloader_run_setup()
{
    {
        auto response = limine_hhdm_request.response;
        if (!response)
            panic("no hhdm response");
        hhdm_offset = response->offset;
    }

    {
        auto response = limine_memmap_request.response;
        if (!response)
            panic("no memmap response");

        static const char* const memmap_type[] = {
            [LIMINE_MEMMAP_USABLE] = "usable",
            [LIMINE_MEMMAP_RESERVED] = "reserved",
            [LIMINE_MEMMAP_ACPI_RECLAIMABLE] = "acpi",
            [LIMINE_MEMMAP_ACPI_NVS] = "acpi nvs",
            [LIMINE_MEMMAP_BAD_MEMORY] = "bad memory",
            [LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE] = "bootloader reclaimable",
            [LIMINE_MEMMAP_KERNEL_AND_MODULES] = "kernel and modules",
            [LIMINE_MEMMAP_FRAMEBUFFER] = "framebuffer",
        };

        usize count = response->entry_count;
        klog("bootloader_run_setup: memory map:\n");
        klog("    Base                Length              Type\n");
        for (usize i=0; i<count; i++) {
            auto entry = response->entries[i];
            klog("    %016zx    %016zx    %s\n", entry->base, entry->length, memmap_type[entry->type]);
            if (entry->type == LIMINE_MEMMAP_USABLE)
                pmm_add_pages((physical_t){entry->base}, entry->length);
        }
        pmm_print_info();
    }
    //*(int*)0x12'3456'7890 = 0x1234;
}
