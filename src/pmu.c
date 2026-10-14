#include "pmu.h"

#define PMU_NSH       (1ULL << 27)
#define PMCR_E        (1ULL << 0)
#define PMCR_P        (1ULL << 1)
#define PMCR_C        (1ULL << 2)
#define PMCR_LC       (1ULL << 6)
#define CYCLE_COUNTER (1ULL << 31)
#define ALL_COUNTERS  (CYCLE_COUNTER | ((1ULL << PMU_N_EVENTS) - 1))

static void select_counter(uint32_t i) {
    __asm__ volatile("msr pmselr_el0, %0\n\tisb" :: "r"((uint64_t)i));
}

static void set_event(uint32_t i, uint16_t event) {
    select_counter(i);
    __asm__ volatile("msr pmxevtyper_el0, %0" :: "r"(event | PMU_NSH));
}

static uint64_t read_counter(uint32_t i) {
    uint64_t val;
    select_counter(i);
    __asm__ volatile("mrs %0, pmxevcntr_el0" : "=r"(val));
    return val;
}

void pmu_init(void) {
    for (uint32_t i = 0; i < PMU_N_EVENTS; i++) {
        set_event(i, pmu_events[i].id);
    }
}

static void clear_overflow(void) {
    __asm__ volatile("msr pmovsclr_el0, %0" :: "r"(ALL_COUNTERS));
}

static uint64_t read_overflow(void) {
    uint64_t flags;
    __asm__ volatile("mrs %0, pmovsclr_el0" : "=r"(flags));
    return flags;
}

void pmu_start(void) {
    clear_overflow();
    __asm__ volatile("msr pmcr_el0, %0" :: "r"(PMCR_E | PMCR_P | PMCR_C | PMCR_LC));
    __asm__ volatile("msr pmcntenset_el0, %0\n\tisb" :: "r"(ALL_COUNTERS));
}

void pmu_stop(void) {
    __asm__ volatile("msr pmcntenclr_el0, %0\n\tisb" :: "r"(ALL_COUNTERS));
}

void pmu_read(struct pmu_counts *out) {
    out->cycles = pmu_cycles();
    for (uint32_t i = 0; i < PMU_N_EVENTS; i++) {
        out->events[i] = read_counter(i);
    }
    out->overflow = read_overflow();
}

void pmu_cycles_enable(void) {
    __asm__ volatile("msr pmcr_el0, %0" :: "r" (PMCR_E | PMCR_LC));
    __asm__ volatile("msr pmcntenset_el0, %0" :: "r" CYCLE_COUNTER);
    __asm__ volatile("msr pmccfiltr_el0, %0" :: "r" PMU_NSH);
    __asm__ volatile("isb");
}

uint64_t pmu_cycles(void) {
    uint64_t c;
    __asm__ volatile("isb\n\tmrs %0, pmccntr_el0" : "=r"(c));
    return c;
}

uint64_t pmu_supported_events(void) {
    uint64_t mask;
    __asm__ volatile("mrs %0, pmceid0_el0" : "=r"(mask));
    return mask;
}
