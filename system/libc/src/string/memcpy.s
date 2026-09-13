.intel_syntax noprefix

        .text

        .global     memcpy
        .type       memcpy, @function
memcpy:
        mov     rax, rdi                # dest
                                        # src = rsi
        mov     rcx, rdx                # count
        rep movsb
        ret

        .size       memcpy, . - memcpy
