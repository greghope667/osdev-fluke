#include "apic.h"

#include "klib.h"
#include "mem/memory.h"
#include "x86_64/mmu.h"

static const physical_t IOAPIC_ADDRESS_P = { 0xfec0'0000 };
static const usize IOAPIC_ADDRESS = 0xffff'ffff'fec0'0000;

enum ioapic_register {
    IOAPIC_ID = 0,
    IOAPIC_VERSION = 1,
    IOAPIC_REDIRECT_TABLE = 0x10,
};

static inline u32
ioapic_read(int offset, enum ioapic_register reg)
{
    volatile u32* ioregsel = (volatile u32*)(IOAPIC_ADDRESS + offset);
    volatile u32* iowin = (volatile u32*)(IOAPIC_ADDRESS + 0x10 + offset);
    *ioregsel = reg;
    return *iowin;
}

enum delivery_mode {
    DELMOD_FIXED = 0,
    DELMOD_LOWEST = 1,
    DELMOD_SMI = 2,
    DELMOD_NMI = 4,
    DELMOD_INIT = 5,
    DELMOD_EXTINT = 7,
};

enum destination_mode { DESTMOD_PHYSICAL, DESTMOD_LOGICAL, };
enum delivery_status { DELIVS_IDLE, DELIVS_PENDING };
enum polarity { INTPOL_HIGH_ACTIVE, INTPOL_LOW_ACTIVE };
enum trigger { TRIGGER_EDGE, TRIGGER_LEVEL };

struct ioredtbl {
    u8 interrupt_vector;
    enum delivery_mode delivery_mode :3;
    enum destination_mode destination_mode :1;
    enum delivery_status delivery_status :1;
    enum polarity polarity :1;
    bool remote_irr :1;
    enum trigger trigger_mode :1;
    bool masked :1;
    u64 _reserved :39;
    u8 destination_field;
} __attribute__((packed));

static void
ioapic_set_entry(int offset, u8 index, struct ioredtbl entry)
{
    u64 entry_bits;
    memcpy(&entry_bits, &entry, 8);

    volatile u32* ioregsel = (volatile u32*)(IOAPIC_ADDRESS + offset);
    volatile u32* iowin = (volatile u32*)(IOAPIC_ADDRESS + 0x10 + offset);

    *ioregsel = IOAPIC_REDIRECT_TABLE + (index << 1);
    *iowin = (u32)entry_bits;
    *ioregsel = IOAPIC_REDIRECT_TABLE + (index << 1) + 1;
    *iowin = (u32)(entry_bits >> 32);
}

void
x86_64_ioapic_initialise()
{
    INIT_ONCE

    mmu_point1(
        mmu_get_address_space(),
        IOAPIC_ADDRESS, IOAPIC_ADDRESS_P,
        MMU_MODE_WRITE, MMU_CACHE_NONE
    );

    klog(
        "x86_64_ioapic_initialise: id %x version %x\n",
        ioapic_read(0, IOAPIC_ID), ioapic_read(0, IOAPIC_VERSION)
    );
}
