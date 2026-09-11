#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/string.h.html

#include <stddef.h>                     // IWYU pragma: export

#ifdef __cplusplus
extern "C" {
#endif

void*   memchr(const void* __s, int __ch, size_t __len) __NOTHROW;
int     memcmp(const void*, const void*, size_t __len) __NOTHROW;
void*   memcpy(
            void* __restrict __dst, const void* __restrict __src, size_t __len
            ) __NOTHROW;
void*   memset(void* __s, int __ch, size_t __len) __NOTHROW;

char*   strcat(char* __restrict __dst, const char* __restrict __src) __NOTHROW;
char*   strchr(const char* __s, int ch) __NOTHROW;
char*   strcpy(char* __restrict __dst, const char* __restrict __src) __NOTHROW;
size_t  strlen(const char* __str) __NOTHROW;


#ifdef __cplusplus
} // extern C
#endif
