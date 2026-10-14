#ifndef PMU_H
#define PMU_H

#include <stdint.h>

#define PMU_N_EVENTS 6

struct pmu_event {
    uint16_t id;
    const char *name;
};

static const struct pmu_event pmu_events[PMU_N_EVENTS] = {
    {0x08, "inst_retired"},
    {0x09, "exc_taken"},
    {0x05, "l1d_tlb_refill"},
    {0x04, "l1d_cache"},
    {0x03, "l1d_cache_refill"},
    {0x17, "l2d_cache_refill"},
};

struct pmu_counts {
    uint64_t cycles;
    uint64_t events[PMU_N_EVENTS];
    uint64_t overflow;
};

void pmu_init(void);
void pmu_start(void);
void pmu_stop(void);
void pmu_read(struct pmu_counts *out);

void pmu_cycles_enable(void);
uint64_t pmu_cycles(void);
uint64_t pmu_supported_events(void);

#endif