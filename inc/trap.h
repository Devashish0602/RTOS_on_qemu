#ifndef _TRAP_H
#define _TRAP_H

#include <stdint.h>

static void put_hex(uint32_t v);
void trap_handler(void);

#endif