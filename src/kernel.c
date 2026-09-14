#include "kernel.h"
#include "timer.h"

TCB tcb[TOTTASKS];

uint32_t stacks[TOTTASKS][STACK_SIZE];

uint32_t current_task = 0;

void task_create(int task, void (*fn)(void), uint32_t priority)
{
    uint32_t *sp = &stacks[task][STACK_SIZE];
    sp -= 32;

    for (int w = 0; w < 32; w++)
        sp[w] = 0;

    sp[30] = (uint32_t)fn;
    sp[31] = ((3 << 11) | (1 << 7));

    tcb[task].sp = (uint32_t *)sp;
    tcb[task].priority = priority;
    tcb[task].state = TASK_READY;
}

uint32_t *context_switch(uint32_t *sp)
{
    int higest_priority_task = -1;
    tcb[current_task].sp = sp;
    timer_next();
    int i=current_task+1;

    for(int i=1;i<=TOTTASKS;i++){
        int n = (current_task+i)%TOTTASKS;
        if(tcb[n].state == TASK_READY){
            if(higest_priority_task == -1 || tcb[n].priority> tcb[higest_priority_task].priority ){
                higest_priority_task =n;
            }
        }
    }

    current_task = higest_priority_task;

    return tcb[current_task].sp;
}

void idle_task(void)
{
    for (;;)
        __asm__ volatile("wfi");
}
void idle_task_create(void)
{
    uint32_t *sp = &stacks[0][STACK_SIZE];
    sp -= 32;

    for (int w = 0; w < 32; w++)
        sp[w] = 0;

    sp[30] = (uint32_t)idle_task;
    sp[31] = ((3 << 11) | (1 << 7));

    tcb[0].sp = (uint32_t *)sp;
    tcb[0].priority = 0;
    tcb[0].state = TASK_READY;
}

void task_yield(void)
{
    __asm__ volatile("ecall");
}