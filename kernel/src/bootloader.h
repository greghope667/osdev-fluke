#pragma once

#include "kdef.h"

#ifdef __cplusplus
extern "C" {
#endif

void bootloader_init_display();
void bootloader_run_setup();
struct Handle* bootloader_open_module(const char* path, usize path_len);
const char* bootloader_cmdline();

#ifdef __cplusplus
} // extern C
#endif
