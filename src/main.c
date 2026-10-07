#include "gpio.h"
#include "peripherals.h"
#include "uart.h"
#include "checksum.h"
#include "mem_management.h"
#include "dataset.h"
#include "search.h"
#include "timer.h"
#include "bench.h"
#include "mmu.h"

static uint32_t results[MAX_QUERIES * K];

static void cycles_enable(void) {
    __asm__ volatile("msr pmcr_el0, %0" :: "r"((1ULL << 0) | (1ULL << 6)));
    __asm__ volatile("msr pmcntenset_el0, %0" :: "r"(1ULL << 31));
    __asm__ volatile("msr pmccfiltr_el0, %0" :: "r"(1ULL << 27));
    __asm__ volatile("isb");
}

static inline uint64_t cycles(void) {
    uint64_t c;
    __asm__ volatile("isb\n\tmrs %0, pmccntr_el0" : "=r"(c));
    return c;
}

static uint64_t cpu_hz(void) {
    uint64_t t0 = timer_ticks();
    uint64_t c0 = cycles();
    while (timer_ticks() - t0 < timer_freq()) {
    }
    uint64_t c1 = cycles();
    return c1 - c0;
}

static void delay(uint32_t count) {
    while (count--) {
        __asm__("nop");
    }
}

void main(void) {
    uart_init();
    delay(1000);

    uart_print("MMU: enabling\n");
    mmu_enable();
    uart_print("MMU: on\n");

    struct dataset ds;

    bool ok = dataset_load(&ds, (const void *)DATASET_BASE);
    if (ok == false) {
        uart_print("Dataset ERR\n");
        return;
    }
    uart_print("Dataset OK\n");

    
    uart_print("--------------------------------------\n");
    cycles_enable();
    uint64_t hz = cpu_hz();
    uart_print("I cpu_hz=");
    uart_print_u64_dec(hz);
    uart_print("\n");
    
    bench_measure(&ds, 20, results);
    bench_report(&ds, 20, results, "baremetal_cache");
    uart_print("--------------------------------------\n");

    return;
}