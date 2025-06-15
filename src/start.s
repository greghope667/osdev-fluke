.intel_syntax noprefix

        .text

        .global _start
        .type _start, @function
        .global _hcf
_start:
        cli
        lea     rdi, [rsp + 0xff8]
        and     rdi, ~0xfff             # Calculate top of stack
        xor     ebp, ebp
        push    rbp                     # Create end of rbp call chain
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
        jz      1f                      # if found, rdi is one past end
        mov     rax, rdi
        ret
1:      lea     rax, [rdi - 1]
        ret


        .global memcmp
        .type memcmp, @function
memcmp:
                                        # lhs = rdi
                                        # rhs = rsi
        mov     rcx, rdx                # count
        repe cmpsb
        ja      1f
        jb      2f
        xor     eax, eax                # lhs == rhs
        ret
1:      mov     eax, -1                 # lhs < rhs
        ret
2:      mov     eax, 1                  # lhs > rhs
        ret


        .global strlen
        .type strlen, @function
strlen:
        mov     rdx, rdi                # start
        or      rcx, -1                 # size = max
        xor     eax, eax                # null-terminator
        repne scasb
        mov     rax, rdi                # end
        sub     rax, rdx
        dec     rax
        ret


        .global strnlen
        .type strnlen, @function
strnlen:
        mov     rdx, rdi                # start
        mov     rcx, rsi                # max count
        xor     eax, eax                # null-terminator
        repne scasb
        mov     rax, rdi                # end
        jz      1f                      # test if null found
        mov     rax, rsi                #       not found
        ret
1:      mov     rax, rdi                #       found
        sub     rax, rdx
        dec     rax
        ret


        .section .symbols, "wa"
        .global __symbols
__symbols:
        .byte   '\n'
