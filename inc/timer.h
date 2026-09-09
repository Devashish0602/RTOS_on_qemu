#ifndef _TIMER_H
#define _TIMER_H

#include <stdint.h>

#define CLINT_BASE   0x02000000UL
#define MTIMECMP_LO  (*(volatile uint32_t *)(CLINT_BASE + 0x4000))
#define MTIMECMP_HI  (*(volatile uint32_t *)(CLINT_BASE + 0x4004))
#define MTIME_LO     (*(volatile uint32_t *)(CLINT_BASE + 0xBFF8))
#define MTIME_HI     (*(volatile uint32_t *)(CLINT_BASE + 0xBFFC))

#define TIMEBASE_HZ  10000000UL     /* confirm against DTB timebase-frequency */
#define TICK_HZ      1UL
#define INTERVAL     (TIMEBASE_HZ / TICK_HZ)


uint64_t mtime_read(void);
void timer_next(void);
void timer_init(void);
void mtimecmp_write(uint64_t v);



#endif