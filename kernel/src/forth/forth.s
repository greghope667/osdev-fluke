.intel_syntax noprefix

/* A simple 64-bit forth, suitable for linking into a kernel */

/* Registers
 * All state is stored in caller saved registers to make
 * calling into C functions cheap
 *
 * rax  W     temporary register
 * rbx  TOS   top of stack
 * rsp  RSP   return stack pointer
 * r12  IP    instruction pointer
 * r13  DSP   data stack pointer
 * r14  EXIT  original stack pointer for exits
 * r15  CTX   forth interpreter context
 *
 * Context structure, all values 1 cell = 8 bytes
 * - data pointer
 * - dictionary
 * - input buffer
 * - input buffer size
 * - input buffer position
 * - state: true if in compilation mode
 */

/* NEXT - jump to next instruction in inner interpreter
 * macro inlined for speed
 */
.macro  NEXT
        mov     rax, [r12]
        add     r12, 8
        jmp     [rax]
.endm

/* Word HEADER structure
 * - next ptr           (8 bytes)
 * - immediate          (1 byte)
 * - name length        (1 byte, max value 126)
 * - name               (variable length)
 * - code field         (8 bytes, aligned)
 * - parameter field    (8*n bytes)
 */
        .equ    _prev_header, 0

.macro  HEADER name, xt_name, flags=0
        .section .rodata
        .balign 8
        .quad   _prev_header
        .equ    _prev_header, .-8
        .byte   \flags
        .byte   (1f-2-.)
        .asciz  "\name\()"
1:
        .balign 8
_\xt_name :
.endm


/* CODE words are primitives defined in assembly.
 * The word's code field points directly to a unique
 * instruction sequence
 */
.macro  CODE name, xt_name
        HEADER  "\name", \xt_name
        .quad  code_\xt_name

        .text
        .balign 4
code_\xt_name :
.endm

/* Stack manipulation */

CODE    "dup", dup
        mov     [r13], rbx
        add     r13, 8
        NEXT

CODE    "swap", swap
        mov     rax, [r13 - 8]
        mov     [r13 - 8], rbx
        mov     rbx, rax
        NEXT

CODE    "2dup", 2dup
        mov     [r13], rbx
        mov     rax, [r13 - 8]
        mov     [r13 + 8], rax
        add     r13, 16
        NEXT

CODE    "drop", drop
        sub     r13, 8
        mov     rbx, [r13]
        NEXT

CODE    "2drop", 2drop
        sub     r13, 16
        mov     rbx, [r13]
        NEXT

CODE    "nip", nip
        sub     r13, 8
        NEXT

/* Memory */

CODE    "@", fetch
        mov     rbx, [rbx]
        NEXT

CODE    "!", store
        mov     rax, [r13 - 8]
        mov     [rbx], rax
        sub     r13, 16
        mov     rbx, [r13]
        NEXT

CODE    "c@", c_fetch
        movzx   rbx, byte ptr [rbx]
        NEXT

CODE    "c!", c_store
        mov     rax, [r13 - 8]
        mov     byte ptr [rbx], al
        sub     r13, 16
        mov     rbx, [r13]
        NEXT

CODE    "aligned", aligned
        add     rbx, 7
        and     rbx, -8
        NEXT

/* Arithmetic */

CODE    "+", plus
        sub     r13, 8
        mov     rax, [r13]
        add     rbx, rax
        NEXT

CODE    "-", minus
        mov     rax, rbx
        sub     r13, 8
        mov     rbx, [r13]
        sub     rbx, rax
        NEXT

CODE    "b.or", bit_or
        sub     r13, 8
        mov     rax, [r13]
        or      rbx, rax
        NEXT

CODE    "1+", one_plus
        inc     rbx
        NEXT

/* Logic */
/* We use 1 and 0 for true and false, to be more C like */

CODE    "true", true
        mov     [r13], rbx
        add     r13, 8
        mov     rbx, 1
        NEXT

CODE    "false", false
        mov     [r13], rbx
        add     r13, 8
        xor     ebx, ebx
        NEXT

CODE    "0=", zero_eq
        xor     eax, eax
        test    rbx, rbx
        setz    al
        mov     rbx, rax
        NEXT

/* Control flow */

CODE    "branch", branch
        mov     r12, [r12]
        NEXT

CODE    "0branch", branch0
        sub     r13, 8
        test    rbx, rbx
        mov     rbx, [r13]
        jz      code_branch
        add     r12, 8
        NEXT

CODE    "?dup", query_dup
        test    rbx, rbx
        jnz     code_dup
        NEXT

CODE    "execute", execute
        mov     rax, rbx
        sub     r13, 8
        mov     rbx, [r13]
        jmp     [rax]

/* Accessing Context */

CODE    "state", state
        mov     [r13], rbx
        add     r13, 8
        lea     rbx, [r15 + 40]
        NEXT

CODE    "dictionary", dictionary
        mov     [r13], rbx
        add     r13, 8
        lea     rbx, [r15 + 8]
        NEXT

CODE    "data-pointer", data_pointer
        mov     [r13], rbx
        add     r13, 8
        mov     rbx, r15
        NEXT

CODE    "here", here
        mov     [r13], rbx
        add     r13, 8
        mov     rbx, [r15]
        NEXT

CODE    ",", comma
        mov     rax, [r15]
        mov     [rax], rbx
        add     rax, 8
        mov     [r15], rax
        sub     r13, 8
        mov     rbx, [r13]
        NEXT


/* Calling into C dynamically, runtime known function */

CODE    "ccall6", ccall6
        mov     rdi, [r13 - 48]
        mov     rsi, [r13 - 40]
        mov     rdx, [r13 - 32]
        mov     rcx, [r13 - 24]
        mov     r8, [r13 - 16]
        mov     r9, [r13 - 8]

        call    rbx
        mov     rbx, rax

        sub     r13, 48
        NEXT

CODE    "ccall2", ccall2
        mov     rdi, [r13 - 16]
        mov     rsi, [r13 - 8]
        call    rbx
        mov     rbx, rax
        sub     r13, 16
        NEXT

CODE    "ccall1", ccall1
        mov     rdi, [r13 - 8]
        call    rbx
        mov     rbx, rax
        sub     r13, 8
        NEXT

/* Code fields for higher-level words */

/* Enter/exit higher level colon definitions.
 * We push much more stuff than we need, so that we get stack traces
 * through forth code in case of a panic */
        .text
        .global code_docolon
code_docolon:
        push    r12
        lea     rcx, [rax + 1]
        push    rcx
        push    rbp
        mov     rbp, rsp
        lea     r12, [rax + 8]
        NEXT

CODE    "exit", exit
        leave
        pop     rax
        pop     r12
        NEXT

        .global code_doconstant
code_doconstant:
        mov     [r13], rbx
        add     r13, 8
        mov     rbx, [rax + 8]                  # Parameter field[0]
        NEXT

        .global code_dovar
code_dovariable:
        mov     [r13], rbx
        add     r13, 8
        lea     rbx, [rax + 8]                  # Parameter field[0]
        NEXT

CODE    "(lit)", lit                            # Run-time part of 'literal'
        mov     [r13], rbx
        add     r13, 8
        mov     rbx, [r12]
        add     r12, 8
        NEXT

/* CFUNC words are implemented in C. The function is known
 * at compile time. The number of arguments must be specified.
 *
 * For fixed arity N, the top N values of the stack are popped
 * and passed in registers. These functions are assumed to return
 * a single value.
 * Variadic functions (arity 'v') are passed the entire stack by
 * pointer, and can return any number of arguments. these also
 * pass the forth Context object.
 *
 * Implementations in forth.c
 */
.macro  CFUNC name, function, arity
        HEADER  "\name", \function
        .quad   code_cfunc\arity
        .quad   \function
.endm

        .text
code_cfunc3:
        mov     rdi, [r13 - 16]                         # Arg 1
        mov     rsi, [r13 - 8]                          # Arg 2
        mov     rdx, rbx                                # Arg 3
        call    [rax + 8]                               # Call parameter field[0]
        sub     r13, 16                                 # Pop args
        mov     rbx, rax                                # Push return
        NEXT

code_cfunc2:
        mov     rdi, [r13 - 8]                          # Arg 1
        mov     rsi, rbx                                # Arg 2
        call    [rax + 8]                               # Call parameter field[0]
        sub     r13, 8                                  # Pop args
        mov     rbx, rax                                # Push return
        NEXT

code_cfunc1:
        mov     rdi, rbx                                # Arg 1
        call    [rax + 8]                               # Call parameter field[0]
        mov     rbx, rax                                # Push return
        NEXT

code_cfuncv:
        mov     [r13], rbx                              # Push TOS onto stack
        mov     rbx, [r14]                              # Stack base
        mov     rsi, rbx                                # Stack base
        mov     rdi, r15                                # Context
        lea     rdx, [r13 + 8]
        sub     rdx, rbx
        shr     rdx, 3                                  # Stack length

        call    [rax + 8]                               # Call parameter field[0]

        lea     r13, [rbx + 8*rax]                      # Reload stack length
        jmp     code_drop

CFUNC   "parse-name",   forth_parse_name,       v
CFUNC   "parse",        forth_parse_char,       v
CFUNC   "find-name",    forth_find_name,        v
CFUNC   "str>num",      forth_str_to_num,       v
CFUNC   "find-symbol",  forth_find_symbol,      2
CFUNC   ".s",           forth_print_stack,      v
CFUNC   "header,",      forth_make_header,      v

/* High level forth words, as defined by ':'
 * We predefine a few here, in assembler, to bootstrap the
 * interpreter
 */
.macro  COLON name, xt_name, flags=0
        HEADER  "\name", \xt_name, \flags
        .quad   code_docolon
.endm

/* : symbol>address ( sym -- u ) 8 + @ ; */
COLON   "symbol>address", symbol_to_address
        .quad   _lit, 8, _plus, _fetch, _exit

/* : parse-constant ( caddr u -- v flag )
 *      2dup str>num if
 *              nip nip true
 *      else drop find-symbol dup if
 *              symbol>address true
 *      else false then then ;
 */
COLON   "parse-constant", parse_constant
        .quad   _2dup, _forth_str_to_num, _branch0, 1f
        .quad   _nip, _nip, _true, _exit
1:      .quad   _drop, _forth_find_symbol, _dup, _branch0, 2f
        .quad   _symbol_to_address, _true, _exit
2:      .quad   _false, _exit

/* : execute? ( nt -- flag )
 *      nt-immediate? state @ 0= b.or ;
 */
COLON   "execute?", execute_query
        .quad   _lit, 8, _plus, _c_fetch, _state, _c_fetch, _zero_eq, _bit_or, _exit

/* : nt>name ( nt -- caddr u )
 *      dup 10 + swap 9 + c@ ;
 */
COLON   "nt>name", nt_to_name
        .quad   _dup, _lit, 10, _plus, _swap, _lit, 9, _plus, _c_fetch, _exit

/* : nt>xt ( nt -- xt )
 *      9 + dup c@ 1+ + 1+ aligned ;
 */
COLON   "nt>xt", nt_to_xt
        .quad   _lit, 9, _plus, _dup, _c_fetch, _one_plus, _plus, _one_plus, _aligned, _exit

/* : literal ['] (lit) , , ; immediate */
COLON   "literal", literal, flags=1
        .quad   _lit, _lit, _comma, _comma, _exit

/* : interpret ( roughly... )
 *      begin parse-name dup while
 *              2dup find-name dup if
 *                      nip nip dup execute? if nt>xt execute else nt>xt , then
 *              else drop parse-constant if
 *                      state @ if postpone literal then
 *              else abort then
 *      repeat ;
 */
        .global _interpret
COLON   "interpret", interpret
.Lloop:
        .quad   _forth_parse_name, _dup, _branch0, .Ldone
        .quad   _2dup, _forth_find_name, _dup, _branch0, .Lnot_word
        .quad           _nip, _nip, _dup, _execute_query, _branch0, .Lcompile_word
        .quad                   _nt_to_xt, _execute, _branch, .Lloop
.Lcompile_word:
        .quad                   _nt_to_xt, _comma, _branch, .Lloop
.Lnot_word:
        .quad   _drop, _parse_constant, _branch0, .Lfail
        .quad           _state, _fetch, _branch0, .Lloop, _literal, _branch, .Lloop
.Lfail:
        .quad   _abort
.Ldone:
        .quad   _2drop, _exit

/* : [ 0 state ! ; immediate */
COLON   "[", left_bracket, flags=1
        .quad   _lit, 0, _state, _store, _exit

/* : ] 1 state ! ; */
COLON   "]", right_bracket
        .quad   _lit, 1, _state, _store, _exit

/* : : parse-name create-header code_docolon , ; */
COLON   ":", colon
        .quad   _forth_parse_name, _forth_make_header, _lit, code_docolon, _comma, _right_bracket, _exit

/* : ; dictionary ! ['] exit , postpone [ ; immediate */
COLON   ";", semicolon, flags=1
        .quad   _dictionary, _store, _lit, _exit, _comma, _left_bracket, _exit

/* : ' parse-name find-name dup 0= if abort then nt>xt ; */
COLON   "'", tick
        .quad   _forth_parse_name, _forth_find_name, _dup, _branch0, 1f, _nt_to_xt, _exit
1:      .quad   _abort


/* Entering / exiting forth */

/* Entry point to jump into compile forth code.
 * Arguments:
 * - Context object
 * - Program byte code, must end with 'exitforth'
 * - Data stack
 * - Number of elements on stack
 *
 * Note that due to the top-of-stack being in a register, this forth
 * may read from/write to data_stack[-1] so this should be a valid address.
 */
        .text
        .global forth_exec
        .type forth_exec, @function
forth_exec:
        push    rbp
        mov     rbp, rsp

        push    rbx
        push    r12
        push    r13
        push    r14
        push    r15

        push    rdx                                     # Data stack base
        mov     r14, rsp                                # Save entry stack frame

        mov     r15, rdi                                # Context
        mov     r12, rsi                                # Instruction pointer = Program
        lea     r13, [rdx + 8*rcx]                      # Data stack pointer

        jmp     code_drop                               # Load TOS and execute

        .global _exitforth
CODE    "bye", exitforth
        mov     [r13], rbx                              # Put TOS back on stack
        lea     rax, [r13 + 8]                          # Data stack end address
        mov     rsp, r14                                # Restore entry stack frame
        pop     rdx                                     # Data stack begin
        sub     rax, rdx                                # Subtract stack begin
        shr     rax, 3                                  # Return current stack size

        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     rbx

        pop     rbp
        ret

CODE    "abort", abort
        lea     rsp, [r14 + 8]                          # Restore entry stack
        mov     rax, -1                                 # Return failure

        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     rbx

        pop     rbp
        ret


/* Head of dictionary linked-list for statically defined words */
        .data
        .global forth_headers
forth_headers:
        .quad   _prev_header

