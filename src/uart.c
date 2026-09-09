#include "uart.h"


void uart_putc(char c)
{
    while (!(UART_LSR & LSR_THRE))
        ;
    UART_THR = c;
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}