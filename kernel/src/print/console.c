#include "console.h"

#include "font/font.h"
#include "klib.h"

static struct Framebuffer fb = {};

static struct Console {
    int x, y;
    int maxx, maxy;
    Color fg, bg;
} console;

void
console_init(const struct Framebuffer* pfb)
{
    INIT_ONCE

    memcpy(&fb, pfb, sizeof(fb));
    console = (struct Console){
        .maxx = fb.width / 8,
        .maxy = fb.height / 8,
        .bg = COLOR_BLACK,
        .fg = COLOR_WHITE,
    };
}

const Color
console_palette[16] = {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    COLOR_BRIGHT_BLACK,
    COLOR_BRIGHT_RED,
    COLOR_BRIGHT_GREEN,
    COLOR_BRIGHT_YELLOW,
    COLOR_BRIGHT_BLUE,
    COLOR_BRIGHT_MAGENTA,
    COLOR_BRIGHT_CYAN,
    COLOR_BRIGHT_WHITE,
};

static void
putcharat_unchecked(char ch, int x, int y, Color fg, Color bg)
{
    void* corner = fb.address + 8 * y * fb.pitch + 32 * x;
    for (int j=0; j<8; j++) {
        u8 line = font_8x8[ch - ' '][j];
        for (int i=0; i<8; i++) {
            Color* pixel = corner + j * fb.pitch + 4 * i;
            *pixel = (line & 1) ? fg : bg;
            line >>= 1;
        }
    }
}

static bool
printable(char ch) { return (ch >= ' ') && (ch < 127); }

void
console_putcharat(char ch, int x, int y, Color fg, Color bg)
{
    if (0 <= x && x < console.maxx && 0 <= y && y < console.maxy) {
        if (!printable(ch)) ch = ' ';
        putcharat_unchecked(ch, x, y, fg, bg);
    }
}

void
console_move(int x, int y)
{
    console.x = CLAMP(x, 0, console.maxx);
    console.y = CLAMP(y, 0, console.maxx);
}

void
console_setcolor(Color fg, Color bg)
{
    console.fg = fg;
    console.bg = bg;
}

void
console_clear()
{
    for (int y=0; y<fb.height; y++) {
        for (int x=0; x<fb.width; x++) {
            Color* pixel = fb.address + y * fb.pitch + 4 * x;
            *pixel = console.bg;
        }
    }
}

static void
clear_line(int y)
{
    for (int j=0; j<8; j++) {
        for (int x=0; x<fb.width; x++) {
            Color* pixel = fb.address + (8 * y + j) * fb.pitch + 4 * x;
            *pixel = console.bg;
        }
    }
}

void
console_write(char ch)
{
    assert(console.maxx && console.maxy);

    if (ch == '\n') {
        console.x = 0;
        console.y++;
        if (console.y >= console.maxy)
            console.y = 0;
        clear_line(console.y);
        return;
    }

    if (console.x >= console.maxx)
        console_write('\n');

    if (!printable(ch)) ch = ' ';
    putcharat_unchecked(ch, console.x, console.y, console.fg, console.bg);
    console.x++;
}
