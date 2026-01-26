#pragma once

#include "kdef.h"

void bootloader_init_display();
void bootloader_run_setup();
void bootloader_run_init_modules();
struct Handle* bootloader_open_module(const char* path, usize path_len);
