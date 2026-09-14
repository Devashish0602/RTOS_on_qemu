#include <stdint.h>
#include "main.h"
#include "timer.h"
#include "kernel.h"
#include "uart.h"

uint32_t uart_semaphore =1;

extern TCB tcb[TOTTASKS];

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

void task3(void)
{
  while(1){
    uart_putc('C');
    // task_yield();
  }
}

int main(void)
{
  idle_task_create();
  task_create(1, task1,1);
  task_create(2, task2,1);
  task_create(3, task3,1);
  timer_init();
  start_first_task(tcb[0].sp);
  for (;;)
    ;
}