#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/stdarg.h.html

#include <fluke/types.h>

typedef __va_list va_list;

#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_copy(dest, src)  __builtin_va_copy(dest, src)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_end(ap)          __builtin_va_end(ap)
