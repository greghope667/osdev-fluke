.intel_syntax noprefix
#include "offsets.h"

        # Imports
        .global exception_kernel_entry
        .global exception_user_entry
        .global interrupt_entry
        .global syscall_entry

        # Exports
        .global isr_stubs_loc_asm
        .global syscall_entry_asm
        .global exit_kernel_asm

/* Stack layout on calling into C handler
 * A pointer to this structure is passed to C
 *
 * Field                Offset  Size
 * SS                   168     2
 * RSP                  160     8
 * RFLAGS               152     8
 * CS                   144     2
 * RIP                  136     8
 * Error Code (or 0)    128     4
 * (ISR number)         120     1
 * Registers (15)       0       120
 */

.macro  pushall
        push    r15
        push    r14
        push    r13
        push    r12
        push    r11
        push    r10
        push    r9
        push    r8
        push    rbp
        push    rdi
        push    rsi
        push    rdx
        push    rcx
        push    rbx
        push    rax
.endm

.macro  popall
        pop     rax
        pop     rbx
        pop     rcx
        pop     rdx
        pop     rsi
        pop     rdi
        pop     rbp
        pop     r8
        pop     r9
        pop     r10
        pop     r11
        pop     r12
        pop     r13
        pop     r14
        pop     r15
.endm

        .text

        .type exn_common, @function
exn_common:
        pushall
        xor     ebp, ebp
        movzx   edi, byte ptr [rsp + 120]               # ISR number
        mov     rsi, rsp                                # ISR context structure

        cmp     word ptr [rsp + 144], GDT_KERNEL_CODE   # Test kernel/user entry
        je      1f

        swapgs                                          # Load kernel GS
        call    exception_user_entry
        swapgs
        jmp     2f

1:      call    exception_kernel_entry

2:      popall
        add     rsp, 16                                 # Pop ISR num and error code
        iretq


        .type isr_common, @function
irq_common:
        cmp     word ptr [rsp + 16], GDT_KERNEL_CODE    # If entry from kernel
        je      1f                                      # Skip loading kernel GS
        swapgs

1:      push    [rsp]                                   # ISR number
        pushall

        xor     ebp, ebp
        movzx   edi, byte ptr [rsp + 120]               # ISR number
        mov     rsi, rsp                                # ISR context structure
        call    interrupt_entry

        popall
        add     rsp, 16                                 # Pop ISR num and error code

        cmp     word ptr [rsp + 8], GDT_KERNEL_CODE     # If leaving to kernel
        je      2f                                      # Skip loading user GS
        swapgs
2:      iretq


# Entry stubs for all 256 isr vectors

stubs:
.macro  ex_stub_err, idx
\idx :
        push    \idx
        jmp     exn_common
.endm

.macro  ex_stub_noerr, idx
\idx :
        push    0
        push    \idx
        jmp     exn_common
.endm

.macro  irq_stub, idx
\idx :
.if     (\idx < 128)
        push    \idx
.else
        push    (\idx - 256)
.endif
        jmp     irq_common
.endm

# Exceptions
ex_stub_noerr   0       # DE
ex_stub_noerr   1       # DB, trap
ex_stub_noerr   2       # NMI
ex_stub_noerr   3       # BP, trap
ex_stub_noerr   4       # OF, trap
ex_stub_noerr   5       # BR
ex_stub_noerr   6       # UD
ex_stub_noerr   7       # NM
ex_stub_err     8       # DF, abort
ex_stub_noerr   9       # reserved
ex_stub_err     10      # TS
ex_stub_err     11      # NP
ex_stub_err     12      # SS
ex_stub_err     13      # GP
ex_stub_err     14      # PF
ex_stub_noerr   15      # reserved
ex_stub_noerr   16      # MF
ex_stub_err     17      # AC
ex_stub_noerr   18      # MC, abort
ex_stub_noerr   19      # XF
ex_stub_noerr   20      # reserved
ex_stub_err     21      # CP
ex_stub_noerr   22      # reserved
ex_stub_noerr   23      # reserved
ex_stub_noerr   24      # reserved
ex_stub_noerr   25      # reserved
ex_stub_noerr   26      # reserved
ex_stub_noerr   27      # reserved
ex_stub_noerr   28      # HV
ex_stub_err     29      # VC
ex_stub_err     30      # SX
ex_stub_noerr   31      # reserved

.altmacro

# Interrupts
.set    isrno, 32
.rept   224
irq_stub        %isrno
.set    isrno, isrno+1
.endr

# Pointers to entries
.macro  link_isr, n
        .long   \n\()b - isr_stubs_loc_asm
.endm


        .section .rodata
isr_stubs_loc_asm:
.set    isrno, 0
.rept   256
link_isr        %isrno
.set    isrno, isrno+1
.endr


# Syscall entry
        .text
        .type syscall_entry_asm, @function
syscall_entry_asm:
        swapgs
        mov     gs:CPU_OFFSET_USER_SP, rsp
        mov     rsp, gs:CPU_OFFSET_KERNEL_SP

        push    GDT_USER_DATA                           # SS
        push    gs:CPU_OFFSET_USER_SP                   # RSP
        push    r11                                     # RFLAGS
        push    GDT_USER64_CODE                         # CS
        push    rcx                                     # RIP
        push    0
        push    0

        pushall

        xor     ebp, ebp
        mov     rdi, rsp
        call    syscall_entry

        popall

        mov     rsp, gs:CPU_OFFSET_USER_SP
        swapgs
        sysretq


        .type exit_kernel_asm, @function
exit_kernel_asm:
        mov     rsp, rdi                                # ISR Context structure
        popall
        add     rsp, 16                                 # Pop ISR num and error code
        cmp     word ptr [rsp + 8], GDT_KERNEL_CODE     # If leaving to kernel
        je      1f                                      # Skip loading user GS
        swapgs
1:      iretq
