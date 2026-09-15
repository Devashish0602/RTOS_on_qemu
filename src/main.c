#include <stdint.h>
#include "main.h"
#include "timer.h"
#include "kernel.h"
#include "uart.h"

extern volatile uint32_t ticks;

extern TCB tcb[TOTTASKS];

void task1(void)
{
  while (1)
  {
    uart_putc('A');
    put_hex(ticks);
    uart_putc('\n');
    task_delay(500);
  }
}

void task2(void)
{
  while (1)
  {
    uart_putc('B');
    put_hex(ticks);
    uart_putc('\n');
    task_delay(100);
  }
}

void task3(void)
{
  while (1)
  {
    uart_putc('C');
    put_hex(ticks);
    uart_putc('\n');
    task_delay(10);
  }
}

int main(void)
{
  idle_task_create();
  task_create(1, task1, 1);
  task_create(2, task2, 1);
  task_create(3, task3, 1);
  timer_init();
  start_first_task(tcb[0].sp);
  for (;;)
    ;
}