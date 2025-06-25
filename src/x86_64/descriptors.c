#include "descriptors.h"
#include "klib.h"
#include "mem/alloc.h"
#include "msr.h"

/* Access byte bits for code/data segments:
 * 0   Accessed - we don't care about this
 * 1   Readable (code segments) Writable (data segments)
 * 2   Conforming (code) Expand-down (data)
 * 3   1=Code, 0=Data
 * 4   S=1 for user/data segments
 * 56  DPL
 * 7   P=1 present
 */

#define ACCESS_CODE 0b10011011
#define ACCESS_DATA 0b10010011
#define ACCESS_USER 0b01100000
#define ACCESS_TSS  0b10001001

/* 'Flags' field, bits G, D, L*/
#define FLAG_CODE64 0b0010
#define FLAG_32 0b1100
#define FLAG_16 0b1000

#define NO_SEGMENT_LIMIT .limit_15_0=0xffff, .limit_19_16=0xf

struct GDT_segment {
    u16 limit_15_0;
    u16 base_15_0;
    u8 base_23_16;
    u8 access;
    u8 limit_19_16:4;
    u8 flags:4;
    u8 base_31_24;
};

_Static_assert(sizeof(struct GDT_segment) == 8);

struct GDT_TSS_entry {
    u16 limit_15_0;
    u16 base_15_0;
    u8 base_23_16;
    u8 access;
    u8 limit_19_16:4;
    u8 flags:4;
    u8 base_31_24;
    u32 base_63_32;
    u32 _reserved;
};

_Static_assert(sizeof(struct GDT_TSS_entry) == 16);

/* The GDT we use for the whole kernel + user space
 * When changing this, the interrupt/syscall entry points
 * may need changing too */
struct GDT {
    u64 null_entry;
    struct GDT_segment kernel_code;
    struct GDT_segment kernel_data;
    struct GDT_segment user_64_data;
    struct GDT_segment user_64_code;
    struct GDT_TSS_entry tss;
};

struct TSS {
    u32 _res0;
    u64 rsp[3];
    u64 _res1;
    u64 ist[7];
    u64 _res2;
    u16 _res3;
    u16 io_map_base;
} PACKED;

_Static_assert(sizeof(struct TSS) == 0x68);

#define GATE_INTERRUPT  0b1110
#define GATE_TRAP       0b1111

struct IDT_entry {
    u16 offset_15_0;
    u16 selector;
    u8 ist:3;
    u8 _res0:5;
    u8 gate_type:4;
    u8 _zero:1;
    u8 dpl:2;
    u8 present:1;
    u16 offset_31_16;
    u32 offset_63_32;
    u32 _res1;
} PACKED;

struct IDT { struct IDT_entry entries[256]; };

_Static_assert(sizeof(struct IDT_entry) == 16);

/* To handle interrupts from user space, we need to allocate
 * a TSS (and therefore GDT) per core. Early in startup we can
 * use this GDT instead to handle faults during initialisation */
static const struct GDT early_gdt = {
    .kernel_code = {
        .access = ACCESS_CODE,
        .flags = FLAG_CODE64,
    },
    .kernel_data = {
        .access = ACCESS_DATA,
    },
};

struct ldt_operand {
    u16 size;
    const void* address;
} PACKED;

_Static_assert(sizeof(struct ldt_operand) == 10);

static void
lgdt(const struct GDT* gdt)
{
    struct ldt_operand lgdt = {
        sizeof(*gdt)-1, gdt
    };
    u16 cs = offsetof(struct GDT, kernel_code);
    u16 ss = offsetof(struct GDT, kernel_data);
    u16 ds = 0;

    /* This inline asm uses push/retfq to change the CS register.
     * For this, we must have the guarantee that nothing is below
     * RSP that could get clobbered. This assembly code is therefore
     * only valid with -mno-red-zone */

    asm volatile (
        "lgdt   %[lgdt]\n"
        "push   %[cs]\n"
        "push   $1f\n"
        "lretq  \n"
        "1:\n"
        "mov    %[ss], %%ss\n"
        "mov    %[ds], %%ds\n"
        "mov    %[ds], %%es\n"
        "mov    %[ds], %%fs\n"
        "mov    %[ds], %%gs\n"
        :
        : [lgdt]"m"(lgdt),
          [cs]"ri"(cs),
          [ss]"r"(ss),
          [ds]"r"(ds)
    );
}

static void
lidt(const struct IDT* idt)
{
    struct ldt_operand lidt = {
        sizeof(*idt)-1, idt
    };
    asm volatile ("lidt %0" : : "m"(lidt));
}

static inline void
ltr()
{
    u16 tss_offset = offsetof(struct GDT, tss);
    asm volatile ("ltr %0" : : "r"(tss_offset));
}


static struct IDT idt;

extern int isr_stubs_loc_asm[256];
extern char syscall_entry_asm[];

void
x86_64_load_early_descriptors()
{
    lgdt(&early_gdt);
    for (int i=0; i<256; i++) {
        usize isr_location = (usize)isr_stubs_loc_asm + isr_stubs_loc_asm[i];
        idt.entries[i] = (struct IDT_entry){
            .gate_type = GATE_INTERRUPT,
            .present = 1,
            .selector = offsetof(struct GDT, kernel_code),
            .offset_15_0 = isr_location,
            .offset_31_16 = isr_location >> 16,
            .offset_63_32 = isr_location >> 32,
        };
    }
    lidt(&idt);
}

static void
enable_syscall()
{
    wrmsr(MSR_CSTAR, 0);
    wrmsr(MSR_LSTAR, (usize)syscall_entry_asm);
    wrmsr(MSR_SFMASK, 0x257fd5);

    static const union STAR {
        u64 value;
        struct {
            u32 _res;
            u16 syscall_selector;
            u16 sysret_selector;
        };
    } star_msr_contents = {
        .syscall_selector = offsetof(struct GDT, kernel_code),
        .sysret_selector = offsetof(struct GDT, user_64_code) + 3 - 16,
    };
    wrmsr(MSR_STAR, star_msr_contents.value);
}

void
x86_64_load_descriptors(usize exception_stack)
{
    struct TSS* tss = xmalloc(sizeof(*tss));
    *tss = (struct TSS) {
        .rsp = { exception_stack },
        .io_map_base = sizeof(*tss),
    };
    usize tss_address = (usize)tss;

    struct GDT* gdt = xmalloc(sizeof(*gdt));
    *gdt = (struct GDT) {
        .kernel_code = {
            .access = ACCESS_CODE,
            .flags = FLAG_CODE64,
        },
        .kernel_data = {
            .access = ACCESS_DATA,
        },
        .user_64_code = {
            .access = ACCESS_CODE | ACCESS_USER,
            .flags = FLAG_CODE64,
        },
        .user_64_data = {
            .access = ACCESS_DATA | ACCESS_USER,
        },
        .tss = {
            .access = ACCESS_TSS,
            .base_15_0 = tss_address,
            .base_23_16 = tss_address >> 16,
            .base_31_24 = tss_address >> 24,
            .base_63_32 = tss_address >> 32,
            .limit_15_0 = sizeof(*tss),
        },
    };

    lgdt(gdt);
    lidt(&idt);
    ltr();
    enable_syscall();
}
