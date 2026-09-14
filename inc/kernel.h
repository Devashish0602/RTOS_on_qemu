#ifndef _KERNEL_H
#define _KERNEL_H

#include <stdint.h>

#define IDLE_TASK   1
#define NTASKS  3
#define TOTTASKS (IDLE_TASK + NTASKS)
#define STACK_SIZE 256

typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED
}state_t;

typedef struct {
    uint32_t* sp;
    uint32_t priority;
    state_t state;
} TCB;

uint32_t* context_switch(uint32_t *sp);

void task_create(int task, void (*fn)(void),uint32_t priority);

void task_yield(void);

void idle_task(void);

void ideal_task_create(void);



#endif
