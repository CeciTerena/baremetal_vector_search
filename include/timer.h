#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

static inline uint64_t timer_freq(void) {
    uint64_t freq;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    return freq;
}

static inline uint64_t timer_ticks(void) {
    uint64_t ticks;
    __asm__ volatile("isb\n\tmrs %0, cntvct_el0" : "=r"(ticks));
    return ticks;
}

static inline float ticks_to_s(uint64_t ticks) {
    return (float)((double)ticks / (double)timer_freq());
}



#endif