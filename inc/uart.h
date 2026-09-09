#ifndef _UART_H
#define _UART_H

#include <stdint.h>

#define UART_BASE 0x10000000UL
#define UART_THR  (*(volatile uint8_t *)(UART_BASE + 0))
#define UART_LSR  (*(volatile uint8_t *)(UART_BASE + 5))
#define LSR_THRE  (1 << 5)

void uart_putc(char c);
void uart_puts(const char *s);


#endif
