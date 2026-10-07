#include <fluke/fluke.h>
#include <stdio.h>

int
main(int argc, char** argv, char** envp)
{
    printf("ARGS %i\n", argc);
    for (int i=0; i<argc; i++)
        printf("%p \"%s\" ", argv[i], argv[i]);
    printf("%p\n", argv[argc]);

    puts("ENV");
    while (*envp)
        printf("%s ", *envp++);
    putchar('\n');

    for (;;) {
        puts("hello");
        _fluke_nsleep(15'000'000'000);
    }
}
