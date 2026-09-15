#include <string.h>

#if 0

__NOTHROW
size_t
strlen(const char* str)
{
    size_t n = 0;
    while (*str++) n++;
    return n;
}

#else
#include <xmmintrin.h>

__NOTHROW
size_t
strlen(const char* str)
{
    const __m128i zero = {};

    int align_offset = (long)str & 0xf;
    const __m128i* ptr = (const __m128i*)(str - align_offset);

    if (align_offset) {
        __m128i eq = _mm_cmpeq_epi8(zero, *ptr++);
        int mask = _mm_movemask_epi8(eq) >> align_offset;
        if (mask)
            return __builtin_ctz(mask);
    }

    for (;; ptr++) {
        __m128i eq = _mm_cmpeq_epi8(zero, *ptr);
        int mask = _mm_movemask_epi8(eq);
        if (mask)
            return (const char*)ptr - str + __builtin_ctz(mask);
    }
}

#endif
