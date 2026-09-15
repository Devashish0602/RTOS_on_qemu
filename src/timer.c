#include "timer.h"

extern volatile uint32_t ticks;

uint64_t mtime_read(void)
{
    uint32_t hi, lo, hi2;
    do
    {
        hi = MTIME_HI;
        lo = MTIME_LO;
        hi2 = MTIME_HI;
    } while (hi != hi2);
    return ((uint64_t)hi << 32) | lo;
}

void mtimecmp_write(uint64_t v)
{
    MTIMECMP_LO = 0xFFFFFFFFu;
    MTIMECMP_HI = (uint32_t)(v >> 32);
    MTIMECMP_LO = (uint32_t)v;
}

void timer_next(void)
{
    mtimecmp_write(mtime_read() + INTERVAL);
    ticks++;
}

void timer_init(void)
{
    timer_next();                                        
    __asm__ volatile("csrs mie, %0" ::"r"(1u << 7));     // MTIE
    __asm__ volatile("csrs mstatus, %0" ::"r"(1u << 3)); // MIE
}