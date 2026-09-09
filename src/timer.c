#include "timer.h"



/* 64-bit read via two 32-bit loads: re-read hi and retry if it changed. */
uint64_t mtime_read(void)
{
    uint32_t hi, lo, hi2;
    do {
        hi  = MTIME_HI;
        lo  = MTIME_LO;
        hi2 = MTIME_HI;
    } while (hi != hi2);
    return ((uint64_t)hi << 32) | lo;
}

/* 64-bit write: park lo at all-ones first so no intermediate value can
   satisfy mtime >= mtimecmp while the two halves disagree. */
void mtimecmp_write(uint64_t v)
{
    MTIMECMP_LO = 0xFFFFFFFFu;
    MTIMECMP_HI = (uint32_t)(v >> 32);
    MTIMECMP_LO = (uint32_t)v;
}
 
void timer_next(void)
{
    mtimecmp_write(mtime_read() + INTERVAL);
}
 
void timer_init(void)
{
    timer_next();                                   /* arm before enabling */
    __asm__ volatile("csrs mie, %0"     :: "r"(1u << 7));   /* MTIE */
    __asm__ volatile("csrs mstatus, %0" :: "r"(1u << 3));   /* MIE  */
}