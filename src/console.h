#pragma once

#include "types.h"

struct Framebuffer {
    void* restrict address;
    int width;                // pixels
    int height;               // pixels
    int pitch;                // bytes
};

typedef u32 Color;

#define COLOR_BLACK             0x000000
#define COLOR_RED               0xcd0000
#define COLOR_GREEN             0x00cd00
#define COLOR_YELLOW            0xcdcd00
#define COLOR_BLUE              0x0000ee
#define COLOR_MAGENTA           0xcd00cd
#define COLOR_CYAN              0x00cdcd
#define COLOR_WHITE             0xe5e5e5
#define COLOR_BRIGHT_BLACK      0x7f7f7f
#define COLOR_BRIGHT_RED        0xff0000
#define COLOR_BRIGHT_GREEN      0x00ff00
#define COLOR_BRIGHT_YELLOW     0xffff00
#define COLOR_BRIGHT_BLUE       0x5c5cff
#define COLOR_BRIGHT_MAGENTA    0xff00ff
#define COLOR_BRIGHT_CYAN       0x00ffff
#define COLOR_BRIGHT_WHITE      0xffffff

// Above colors as table
extern const Color console_palette[16];

void console_init(const struct Framebuffer* fb);

void console_putcharat(char ch, int x, int y, Color fg, Color bg);
void console_move(int x, int y);
void console_setcolor(Color fg, Color bg);
void console_clear();
void console_write(char ch);
