#include "kernel.h"

#define NTASKS 2
#define STACK_SIZE 256



TCB tcb[NTASKS];

uint32_t stacks[NTASKS][STACK_SIZE];

uint32_t current_task = 0;

void task_create(int task, void (*fn)(void))
{
    uint32_t *sp = &stacks[task][STACK_SIZE];   
    sp -= 32;                                

    for (int w = 0; w < 32; w++)
        sp[w] = 0;

    sp[30] = (uint32_t)fn;                
    sp[31] = ((3<<11) | (1<<7));          

    tcb[task].sp = (uint32_t)sp;
}


uint32_t context_switch(uint32_t *sp)
{
    static int first_time = 0;
    if (first_time) {
        tcb[current_task].sp = sp;
    }
    first_time = 1;
    timer_next();
    current_task= ((current_task + 1) % NTASKS);
    return tcb[current_task].sp;
}


void task_yield(void)
{
    __asm__ volatile("ecall");
}