#include "bootloader.h"
#include "mem/alloc.h"
#include "print/console.h"
#include "klib.h"
#include "mem/memory.h"
#include "mem/pmm.h"
#include "errno.h"
#include "user/handle.h"

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

const volatile u64 limine_base_revision[] = LIMINE_BASE_REVISION(1);

const volatile struct limine_bootloader_info_request
limine_bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST_ID,
    .revision = 0,
};

const volatile struct limine_framebuffer_request
limine_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 1,
};

const volatile struct limine_hhdm_request
limine_hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
};

const volatile struct limine_memmap_request
limine_memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
};

const volatile struct limine_module_request
limine_module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
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
        auto response = limine_bootloader_info_request.response;
        if (!response)
            panic("no bootloader info response");
        klog("bootloader_run_setup: %s %s\n", response->name, response->version);
    }

    klog("bootloader_run_setup: limine base revision %zu\n", limine_base_revision[2]);

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
            [LIMINE_MEMMAP_EXECUTABLE_AND_MODULES] = "kernel and modules",
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

    {
        auto response = limine_module_request.response;
        if (!response)
            panic("no module response");

        klog("bootloader_run_setup: modules:\n");
        klog("    Address             Size                Path\n");

        usize count = response->module_count;
        for (usize i=0; i<count; i++) {
            auto module = response->modules[i];
            klog(
                "    %p    %016zx    %s    %s\n",
                module->address, module->size, module->path, module->string
            );
        }
    }
}

// Kernel modules
// (most of this implementation should probably be elsewhere)

struct Module_handle {
    struct Handle handle;
    void* address;
    isize size;
    isize offset;
};

static void
module_destruct(struct Handle* h)
{
    kfree(h, sizeof(struct Module_handle));
}

static isize
module_read(struct Handle* h, void* dest, isize len)
{
    assert(len > 0);
    auto module = container_of(h, struct Module_handle, handle);
    isize bytes = MIN(len, module->size - module->offset);
    if (!copy_to_user(dest, module->address + module->offset, bytes))
        return -1;
    module->offset += bytes;
    return bytes;
}

const struct Handle_vtbl module_vtable = {
    .destruct = module_destruct,
    .read = module_read,
};

struct Handle*
bootloader_open_module(const char* path, usize path_len)
{
    auto modules = limine_module_request.response;
    usize count = modules->module_count;
    for (usize i=0; i<count; i++) {
        auto module = modules->modules[i];
        usize len = strlen(module->path);

        if (path_len == len && memcmp(path, module->path, len) == 0) {
            if (module->size > ISIZE_MAX)
                panic("kernel module too large");

            struct Module_handle* handle = kalloc(sizeof(*handle));
            if (!handle)
                return nullptr;

            *handle = (struct Module_handle) {
                .handle  = { .vtbl = &module_vtable },
                .address = module->address,
                .size    = module->size,
                .offset  = 0,
            };
            return &handle->handle;
        }
    }

    errno = ENOENT;
    return nullptr;
}
