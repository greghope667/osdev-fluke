.intel_syntax noprefix

        .global     _setjmp
        .type       _setjmp, @function
_setjmp:
        mov     rcx, [rsp]                  # Return address
        mov     [rdi], rcx
        mov     [rdi + 0x08], r12
        mov     [rdi + 0x10], r13
        mov     [rdi + 0x18], r14
        mov     [rdi + 0x20], r15
        mov     [rdi + 0x28], rbx
        mov     [rdi + 0x30], rbp
        mov     [rdi + 0x38], rsp
        xor     eax, eax                    # Return 0 for non-longjmp
        ret

        .size       _setjmp, .-_setjmp


        .global     setjmp
        .set        setjmp, _setjmp


        .global     _longjmp
        .type       _longjmp, @function
_longjmp:
        test    esi, esi
        mov     eax, 1
        cmovne  eax, esi                    # if ret = 0, return 1 instead

        mov     rcx, [rdi]
        mov     r12, [rdi + 0x08]
        mov     r13, [rdi + 0x10]
        mov     r14, [rdi + 0x18]
        mov     r15, [rdi + 0x20]
        mov     rbx, [rdi + 0x28]
        mov     rbp, [rdi + 0x30]
        mov     rsp, [rdi + 0x38]

        mov     [rsp], rcx                  # return address
        ret

        .size       _longjmp, .-_longjmp


        .global     longjmp
        .set        longjmp, _longjmp
