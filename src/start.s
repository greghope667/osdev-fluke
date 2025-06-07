.intel_syntax noprefix

        .text

        .global _start
        .type _start, @function
        .global _hcf
_start:
        cli
        xor     ebp, ebp
        push    rbp
        lea     rdi, [rsp + 16]
        call    entry
_hcf:
        cli
        hlt
        jmp     _hcf


        .global memcpy
        .type memcpy, @function
memcpy:
        mov     rax, rdi                # dest
                                        # src = rsi
        mov     rcx, rdx                # count
        rep movsb
        ret


        .global memset
        .type memset, @function
memset:
        mov     rcx, rdx                # count
        mov     rdx, rdi                # dest
        mov     eax, esi                # val
        rep stosb
        mov     rax, rdx                # return dest
        ret


        .global memchr
        .type memchr, @function
memchr:
                                        # ptr = rdi
        mov     eax, esi                # ch
        mov     rcx, rdx                # count
        repne scasb
        jz      1f
        mov     rax, rdi
        ret
1:      lea     rax, [rdi - 1]
        ret


        .section .symbols, "wa"
        .global __symbols
__symbols:
        .byte   '\n'
