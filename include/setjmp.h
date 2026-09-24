#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/setjmp.h.html

typedef long jmp_buf[8];

int     setjmp(jmp_buf) __attribute__((returns_twice));
int     _setjmp(jmp_buf) __attribute__((returns_twice));

void    longjmp(jmp_buf, int) __attribute__((noreturn));
void    _longjmp(jmp_buf, int) __attribute__((noreturn));
