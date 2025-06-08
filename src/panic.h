#pragma once

void panic(const char* reason) __attribute__((noreturn));
void show_backtrace(void* frame);
void show_backtrace_here();
