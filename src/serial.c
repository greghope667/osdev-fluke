#include "serial.h"
#include "io_port.h"
#include "types.h"

// TODO: move these
u8 io_inb(u16 addr);
void io_outb(u16 addr, u8 byte);

/* Serial port - in IO space
 * struct serial {
 *     union {
 *         struct {
 *             u8 data;
 *             u8 interrupt_enable;
 *         };
 *         uint16le_t baud_divisor;
 *     };
 *     u8 int_id_fifo;
 *     u8 line_control;
 *     u8 modem_control;
 *     u8 line_status;
 *     u8 modem_status;
 *     u8 scratch;
 * }
 */

/* Constants for chip 16550 */
#define REG_BAUD_LOW                0   // With divisor latch
#define REG_BAUD_HIGH               1   // With divisor latch
#define REG_DATA                    0
#define REG_INTERRUPT_ENABLE        1
#define REG_INTERRUPT_ID            2   // r
#define REG_FIFO_CONTROL            2   // w
#define REG_LINE_CONTROL            3
#define REG_MODEM_CONTROL           4
#define REG_LINE_STATUS             5
#define REG_MODEM_STATUS            6
#define REG_SCRATCH                 7

#define INT_ENABLE_RX_AVAIL         0x01
#define INT_ENABLE_TX_EMPTY         0x02
#define INT_ENABLE_LINE_STATUS      0x04
#define INT_ENABLE_MODEM_STATUS     0x08

#define FIFO_CTRL_ENABLE            0x01
#define FIFO_CTRL_CLEAR_RX          0x02
#define FIFO_CTRL_CLEAR_TX          0x04
#define FIFO_CTRL_RDY_EN            0x08
#define FIFO_CTRL_TRIGGER_01        0x00
#define FIFO_CTRL_TRIGGER_04        0x40
#define FIFO_CTRL_TRIGGER_08        0x80
#define FIFO_CTRL_TRIGGER_14        0xC0

#define LINE_CTRL_DIV_LATCH         0x80
#define LINE_CTRL_8N1               0x03

#define MODEM_CTRL_DTR              0x01
#define MODEM_CTRL_RTS              0x02
#define MODEM_CTRL_ENABLE_IRQ       0x08 // a.k.a. OUT2
#define MODEM_CTRL_LOOPBACK         0x10

#define LINE_STATUS_DATA_READY      0x01
#define LINE_STATUS_TX_EMPTY        0x40

// TODO: add remaining register bits when needed

#define MAX_BAUD_RATE               115200U

/* Configurable options */
#define BAUD_RATE                   9600U
#define SCRATCH_TEST_BYTE           ((u8)0xAB) // arbitrary value
#define LOOP_TEST_BYTE              ((u8)0xCD) // arbitrary value

static inline void
set_div_latch (u16 port, bool enabled)
{
    u16 line_control = port + REG_LINE_CONTROL;
    u8 val = io_inb(line_control);
    u8 new;

    if (enabled)
        new = val | LINE_CTRL_DIV_LATCH;
    else
        new = val & ~LINE_CTRL_DIV_LATCH;

    if (new != val)
        io_outb(line_control, new);
}

static bool
serial_detect_setup(u16 port)
{
    io_outb(port + REG_SCRATCH, SCRATCH_TEST_BYTE);
    if (SCRATCH_TEST_BYTE != io_inb(port + REG_SCRATCH))
        return false;

    // Disable interrupts
    set_div_latch(port, false);
    io_outb(port + REG_INTERRUPT_ENABLE, 0);

    // Set BAUD
    set_div_latch(port, true);
    const u16 baud = MAX_BAUD_RATE / BAUD_RATE;
    io_outb(port + REG_BAUD_LOW, baud & 0xff);
    io_outb(port + REG_BAUD_HIGH, baud >> 8);

    // Configure reasonable defaults
    io_outb(port + REG_LINE_CONTROL, LINE_CTRL_8N1);
    io_outb(port + REG_FIFO_CONTROL,
        FIFO_CTRL_ENABLE | FIFO_CTRL_CLEAR_RX |
        FIFO_CTRL_CLEAR_TX | FIFO_CTRL_TRIGGER_08);

    io_outb(port + REG_MODEM_CONTROL,
        MODEM_CTRL_DTR | MODEM_CTRL_RTS | MODEM_CTRL_LOOPBACK);

    // Test loopback
    io_outb(port, LOOP_TEST_BYTE);
    if (io_inb(port) != LOOP_TEST_BYTE)
        return false;

    io_outb(port + REG_MODEM_CONTROL,
        MODEM_CTRL_DTR | MODEM_CTRL_RTS);

    return true;
}

const u16 serial_address[] = {
    0x3f8, 0x2f8, 0x3e8, 0x2e8
};

static u16 port;

int
serial_init()
{
    static bool done = false;
    if (done) return port;

    for (int i=0; i<ARRAY_LENGTH(serial_address); i++) {
        if (serial_detect_setup(serial_address[i])) {
            port = serial_address[i];
            done = true;
            return port;
        }
    }
    return 0;
}

void
serial_write(char c)
{
    if (c == '\n')
        serial_write('\r');

    while (!(io_inb(port + REG_LINE_STATUS) & LINE_STATUS_TX_EMPTY))
        __builtin_ia32_pause();
    io_outb(port, c);
}

int
serial_read()
{
    if (io_inb(port + REG_LINE_STATUS) & LINE_STATUS_DATA_READY)
        return io_inb(port);
    else
        return EOF;
}
