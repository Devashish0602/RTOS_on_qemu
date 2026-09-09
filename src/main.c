#include <stdint.h>
void uart_puts(const char *s);
void trap_handler(void);
void timer_init(void);

int main(void)
{
  timer_init();
    for (;;)
        ;
}