#pragma once

#include <fluke/types.h>
#include <fluke/defs/errno.h>           // IWYU pragma: export

#ifdef __cplusplus
extern "C"
#endif
int* __errno_location() __CONST __NOTHROW;

#define errno (*__errno_location())
