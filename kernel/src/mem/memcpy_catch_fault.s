.intel_syntax noprefix

        .text

        .global memcpy_catch_fault
        .type memcpy_catch_fault, @function
        .global __memcpy_catch_fault_op
        .global __memcpy_catch_fault_err
memcpy_catch_fault:
        mov     eax, ecx                # faults to catch
                                        # dest = rdi
                                        # src = rsi
        mov     rcx, rdx                # count
__memcpy_catch_fault_op:
        rep movsb
        xor     eax, eax                # return 0 on success
__memcpy_catch_fault_err:
        ret

        .size memcpy_catch_fault, .-memcpy_catch_fault
