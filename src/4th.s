.intel_syntax noprefix

/* Registers
 * rax  W     temporary register
 * rbx  TOS   top of stack
 * rsp  RSP   return stack pointer
 * r12  IP    instruction pointer
 * r13  DSP   data stack pointer
 */

.macro  NEXT
        mov     rax, [r12]
        add     r12, 8
        jmp     [rax]
.endm

.macro  CODE name, xt_name
        .global xt_\xt_name

        .section .rodata
        .balign 8
xt_\xt_name :
        .quad  code_\xt_name

        .text
        .balign 4
code_\xt_name :
.endm

        .text

CODE    "dup", dup
        mov     [r13], rbx
        add     r13, 8
        NEXT

CODE    "pop", pop
        sub     r13, 8
        mov     rbx, [r13]
        NEXT

CODE    "+", plus
        sub     r13, 8
        mov     rax, [r13]
        add     rbx, rax
        NEXT

code_docolon:
        push    r12
        lea     r12, [rax + 8]
        NEXT

CODE    "exit", exit
        pop     r12
        NEXT

CODE    "(lit)", lit
        mov     [r13], rbx
        add     r13, 8
        mov     rbx, [r12]
        add     r12, 8
        NEXT

CODE    "ccall3", ccall3
        mov     rdi, [r13 - 24]
        mov     rsi, [r13 - 16]
        mov     rdx, [r13 - 8]

        call    rbx
        mov     rbx, rax

        sub     r13, 24
        NEXT

        .global code_cfunc3
code_cfunc3:
        mov     rdi, [r13 - 16]
        mov     rsi, [r13 - 8]
        mov     rdx, rbx

        call    [rax + 8]
        mov     rbx, rax

        sub     r13, 16
        NEXT


        .global run_forth
        .type run_forth, @function
run_forth:
        push    rbp
        mov     rbp, rsp

        push    rbx
        push    r12
        push    r13
        push    r14

        mov     r12, rdi                                # Instruction pointer = Program
        mov     r14, rsi                                # Data stack base
        lea     r13, [rsi + 8*rdx]                      # Data stack pointer

        jmp     code_pop                                # Load TOS and execute

CODE    "exitforth", exitforth
        mov     [r13], rbx                              # Put TOS back on stack
        lea     rax, [r13 + 8]                          # Stack end address
        sub     rax, r14                                # Subtract stack begin
        shr     rax, 3                                  # Return current stack size

        pop     r14
        pop     r13
        pop     r12
        pop     rbx

        pop     rbp

        ret


