#pragma once

#include <fluke/defs/errno.h>           // IWYU pragma: export

extern int* __errno_location();
#define errno (*__errno_location())
