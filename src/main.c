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
#include "pmu.h"

#if BM_CACHE
#define PLATFORM "baremetal_cache"
#else
#define PLATFORM "baremetal_nocache"
#endif
static uint32_t results[MAX_QUERIES * K];

static uint64_t cpu_hz(void) {
    uint64_t t0 = timer_ticks();
    uint64_t c0 = pmu_cycles();
    while (timer_ticks() - t0 < timer_freq()) {
    }
    uint64_t c1 = pmu_cycles();
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

    uart_print("START\n");

    #if BM_CACHE
        uart_print("MMU: enabling\n");
        mmu_enable();
        uart_print("MMU: on\n");
    #endif

    struct dataset ds;

    bool ok = dataset_load(&ds, (const void *)DATASET_BASE);
    if (ok == false) {
        uart_print("Dataset ERR\n");
        return;
    }
    uart_print("Dataset OK\n");

    
    uart_print("--------------------------------------\n");
    pmu_cycles_enable();
    uint64_t hz = cpu_hz();
    uart_print("I cpu_hz=");
    uart_print_u64_dec(hz);
    uart_print("\n");
    
    pmu_init();
    uart_print("I pmu_supported=");
    uart_print_hex(pmu_supported_events());
    uart_print("\n");
    
    bench_warmup(&ds, results);
    bench_measure_batch(&ds, MAX_REPS, results);
    bench_measure_queries(&ds, MAX_REPS, results);
    bench_report(&ds, MAX_REPS, results, PLATFORM);
    uart_print("--------------------------------------\n");

    return;
}