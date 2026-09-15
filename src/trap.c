#include <stdint.h>

#include "uart.h"
#include "timer.h"


void put_hex(uint32_t v)
{
    const char *d = "0123456789abcdef";
    char buf[11] = "0x";
    for (int i = 0; i < 8; i++)
        buf[2 + i] = d[(v >> ((7 - i) * 4)) & 0xf];
    buf[10] = '\0';
    uart_puts(buf);
}

void trap_handler(void)
{
    uint32_t mcause, mepc, mtval;

    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
    __asm__ volatile("csrr %0, mepc"   : "=r"(mepc));
    __asm__ volatile("csrr %0, mtval"  : "=r"(mtval));

    uart_puts("\nmecp = "); put_hex(mepc);
    uart_puts("\nmcause = "); put_hex(mcause);
    uart_puts("\nmtval = "); put_hex(mtval);
    uart_puts("\nmtime = "); put_hex(mtime_read());
    timer_next();
    // __asm__ volatile("csrw mepc, %0" :: "r"(mepc+2));

}