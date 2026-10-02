.intel_syntax noprefix

        .text

        .global     memset
        .type       memset, @function
memset:
        mov     rcx, rdx                # count
        mov     rdx, rdi                # dest
        mov     eax, esi                # val
        rep stosb
        mov     rax, rdx                # return dest
        ret

        .size       memset, .-memset
