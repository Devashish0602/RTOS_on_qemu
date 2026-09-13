#ifndef _KERNEL_H
#define _KERNEL_H

#include <stdint.h>


typedef struct {
    uint32_t* sp; // Stack pointer
} TCB;

uint32_t context_switch(uint32_t *sp);

void task_create(int task, void (*fn)(void));

void task_yield(void);



#endif
