#pragma once

enum print_dest {
    PRINT_DEST_SERIAL   = 1 << 0,
    PRINT_DEST_CONSOLE  = 1 << 1,
};

void print_dest_enable(enum print_dest dest);
void print_dest_disable(enum print_dest dest);
