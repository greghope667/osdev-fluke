#include "forth.h"
#include "../main.h" // IWYU pragma: keep
#include "../panic.h"

static const struct Forth_header*
lookup_word(const struct Forth_header* dict, const char* name, isize len)
{
    for (const struct Forth_header* e = dict; e; e = e->next) {
        if (e->name_length == len && memcmp(e->name, name, len) == 0) {
            return e;
        }
    }
    return NULL;
}

isize // ( "<spaces>name<space>" -- caddr u )
forth_parse_name(struct Forth_context* ctx, isize stack[], isize stack_use)
{
    isize remainder = ctx->ntib - ctx->to_in;
    const char* begin = ctx->tib + ctx->to_in;
    while (remainder > 0 && *begin <= ' ') {
        remainder--;
        begin++;
    }
    const char* end = begin;
    while (remainder > 0 && *end > ' ') {
        remainder--;
        end++;
    }
    ctx->to_in = end - ctx->tib + (remainder > 0);
    stack[stack_use++] = (isize)begin;
    stack[stack_use++] = end - begin;
    return stack_use;
}

isize // ( char "ccc<char>" -- caddr u )
forth_parse_char(struct Forth_context* ctx, isize stack[], isize stack_use)
{
    isize remainder = ctx->ntib - ctx->to_in;
    char delimiter = stack[stack_use-1];
    const char* begin = ctx->tib + ctx->to_in;
    const char* end = begin;
    while (remainder > 0 && *end != delimiter ) {
        remainder--;
        end++;
    }
    ctx->to_in = end - ctx->tib + (remainder > 0);
    stack[stack_use-1] = (isize)begin;
    stack[stack_use++] = end - begin;
    return stack_use;
}

isize // ( caddr u -- nt | 0 )
forth_find_name(struct Forth_context* ctx, isize stack[], isize stack_use)
{
    auto word = lookup_word(
        ctx->dictionary,
        (const char*)stack[stack_use-2],
        CLAMP(stack[stack_use-1], 0, 126)
    );
    stack[stack_use-2] = (isize)word;
    return stack_use-1;
}

isize // ( caddr u -- n 1 | x 0)
forth_str_to_num(struct Forth_context*, isize stack[], isize stack_use)
{
    const char* buf = (const char*)stack[stack_use - 2];
    isize len = stack[stack_use - 1];
    isize value = 0;
    for (int i=0; i<len; i++) {
        char ch = buf[i];
        if (ch >= '0' && ch <= '9')
            value = value * 16 + ch - '0';
        else if (ch >= 'a' && ch <= 'f')
            value = value * 16 + 10 + ch - 'a';
        else {
            stack[stack_use-1] = false;
            return stack_use;
        }
    }
    stack[stack_use-2] = value;
    stack[stack_use-1] = true;
    return stack_use;
}

isize // ( caddr u -- s )
forth_find_symbol(const char* name, isize len)
{
    for (const struct Symbol* s = symbol_list; s; s = s->next) {
        if (s->length == len && isupper(s->type) && memcmp(s->name, name, len) == 0)
            return (isize)s;
    }
    return 0;
}

isize
forth_c_print(isize x)
{
    return printf("%zx ", x);
}

isize
forth_print_stack(void*, isize stack[], isize stack_usage)
{
    write("[ ", 2);
    for (isize i=0; i<stack_usage; i++)
        printf("%zx ", stack[i]);
    write("]\n", 2);
    return stack_usage;
}

isize // ( caddr u -- nt )
forth_make_header(struct Forth_context* ctx, isize stack[], isize stack_usage)
{
    const char* name = (const char*)stack[stack_usage-2];
    isize len = CLAMP(stack[stack_usage-1], 0, 126);

    struct Forth_header* header = ctx->here;
    header->next = ctx->dictionary;
    header->immediate = 0;
    header->name_length = len;
    memcpy(header->name, name, len);
    header->name[len] = 0;

    ctx->here = forth_header_to_xt(header);

    stack[stack_usage-2] = (isize)header;
    return stack_usage-1;
}

extern struct Forth_body _exitforth;
extern struct Forth_body _interpret;

int
forth_interpret(const char* text, isize chars, isize stack[])
{
    static u8 data_space[4096];
    static const Forth_xt program[2] = { &_interpret, &_exitforth };
    memset(data_space, 0, sizeof(data_space));

    isize stack_usage = 0;

    struct Forth_context ctx = {
        .dictionary = forth_headers,
        .here = data_space,
        .tib = text,
        .ntib = chars,
        .to_in = 0,
    };

    stack_usage = forth_exec(&ctx, program, stack, stack_usage);

    if (stack_usage < 0) {
        puts("aborted");
        return -1;
    } else {
        return forth_print_stack(&ctx, stack, stack_usage);
    }
}

void
forth_init()
{
    static u8 data_space[4096];
    static const char init_code[] = {
        #embed "startup.4th"
    };
    static const Forth_xt program[2] = { &_interpret, &_exitforth };

    isize init_stack[32];

    struct Forth_context ctx = {
        .dictionary = forth_headers,
        .here = data_space,
        .tib = init_code,
        .ntib = sizeof(init_code),
        .to_in = 0,
    };

    isize ret = forth_exec(&ctx, program, init_stack+1, 0);
    if (ret != 0)
        panic("forth code startup failure");
    if (ctx.here > (void*)&data_space[sizeof(data_space)])
        panic("forth init code overran data space");

    forth_headers = ctx.dictionary;
}
