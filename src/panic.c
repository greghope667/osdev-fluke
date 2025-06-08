#include "panic.h"
#include "console.h"
#include "main.h"

static const struct Symbol*
symbol_of_address(void* address)
{
    const struct Symbol* match = 0;
    for (const struct Symbol* s = symbol_list; s; s = s->next) {
        if (s->address >= address)
            break;
        match = s;
    }
    return match;
}

void __attribute__((noreturn))
panic(const char* reason)
{
    console_setcolor(COLOR_WHITE, COLOR_BLUE);
    extern void _hcf() __attribute__((noreturn));
    printf("\nKERNEL PANIC: %s\nbacktrace:\n", reason);
    show_backtrace_here();
    _hcf();
}

void show_backtrace(void* frame)
{
    puts("| frame            | return           | symbol");
    while (frame) {
        printf("| %p | ", frame);

        void* return_address = *(void**)(frame + 8);
        printf("%p | ", return_address);

        auto match = symbol_of_address(return_address);
        if (match)
            printf("%s+0x%zx", match->name, return_address - match->address);

        putchar('\n');
        frame = *(void**)frame;
    }
}

void show_backtrace_here()
{
    show_backtrace(__builtin_frame_address(0));
}
