#include "bootloader.h"
#include "console.h"
#include "panic.h"

#include <limine.h>

const volatile struct limine_framebuffer_request
limine_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 1,
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
