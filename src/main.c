#include <stdint.h>
#include "timer.h"
#include "kernel.h"
#include "uart.h"

uint32_t uart_semaphore =1;

void task1(void)
{
  while(1){
    uart_putc('A');
    // task_yield();
  }
}

void task2(void)
{
  while(1){
    uart_putc('B');
    // task_yield();
  }
}
int main(void)
{
  task_create(0, task1);
  task_create(1, task2);
  timer_init();
  for (;;)
    ;
}