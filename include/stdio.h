#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/stdio.h.html

#include <fluke/types.h>
#include <fluke/defs/seek.h>            // IWYU pragma: export

#include <stddef.h>                     // IWYU pragma: export

typedef struct FILE FILE;
typedef __off_t off_t;
typedef __size_t size_t;
typedef __ssize_t ssize_t;
typedef __va_list va_list;

#define BUFSZ 4096
#define _IOFBF 1
#define _IOLBF 2
#define _IONBF 3

#define EOF (-1)

extern struct FILE* stdin;
extern struct FILE* stdout;
extern struct FILE* stderr;

#define stdin stdin
#define stdout stdout
#define stderr stderr

#ifdef __cplusplus
extern "C" {
#endif

int     fclose(FILE*);
FILE*   fdopen(int __fd, const char* __mode);
int     feof(FILE*) __NOTHROW;
int     ferror(FILE*) __NOTHROW;
int     fflush(FILE*);
int     fgetc(FILE*);
char*   fgets(char* __restrict __buf, size_t __n, FILE*);
FILE*   fopen(const char* __name, const char* __mode);
int     fputc(int __ch, FILE*);
int     fputs(const char* __str, FILE*);
size_t  fread(void* __restrict __buf, size_t __item_size, size_t __nitems, FILE*);
int     fseek(FILE*, long __off, int __whence);
int     fseeko(FILE*, off_t __off, int __whence);
long    ftell(FILE*);
off_t   ftello(FILE*);
size_t  fwrite(const void* __buf, size_t __item_size, size_t __nitems, FILE*);

int     getchar();
int     putchar(int __ch);

int     getc(FILE*);
int     putc(int __ch, FILE*);
int     puts(const char* __str);
int     setbuf(FILE*, char* __restrict __buf);
int     setvbuf(FILE*, char* __restrict __buf, int __type, size_t __size);
int     ungetc(int __ch, FILE*);

int     fprintf(FILE*, const char* __fmt, ...)
            __attribute__((__format__(printf, 2, 3)));
int     printf(const char* __fmt, ...)
            __attribute__((__format__(printf, 1, 2)));
int     snprintf(char* __restrict __buf, size_t __n, const char* __fmt, ...)
            __attribute__((__format__(printf, 3, 4)));
int     sprintf(char* __restrict __buf, const char* __fmt, ...)
            __attribute__((__format__(printf, 2, 3)));

int     vfprintf(FILE*, const char* __fmt, va_list);
int     vprintf(const char* __fmt, va_list);
int     vsnprintf(char* __restrict __buf, size_t __n, const char* __fmt, va_list);
int     vsprintf(char* __restrict __buf, const char* __fmt, va_list);

#ifdef __cplusplus
} // extern C
#endif
