
#include "bench.h"
#include "search.h"
#include "timer.h"
#include "uart.h"
#include "pmu.h"


static uint64_t batch_ticks[MAX_REPS];
static uint64_t query_ticks[MAX_REPS * MAX_QUERIES];

static struct pmu_counts pmu_counts;

// actually measuring the ammput of time that timing things takes (time the reading itself takes!).
static uint64_t timer_overhead(void) {
    uint64_t min = UINT64_MAX;
    for (uint32_t i = 0; i < 1000; i++) {
        uint64_t t0 = timer_ticks();
        uint64_t t1 = timer_ticks();
        if (t1 - t0 < min) {
            min = t1 - t0;
        }
    }
    return min;
}

void bench_warmup(const struct dataset *ds, uint32_t *results) {
    knn_brute_force(ds->vectors, ds->n_vecs, ds->queries, ds->n_queries, ds->dim, results);
}

void bench_measure_batch(const struct dataset *ds, uint32_t reps, uint32_t *results) {
    pmu_start();
    for (uint32_t rep = 0; rep < reps; rep++) {
        uint64_t t0 = timer_ticks();
        knn_brute_force(ds->vectors, ds->n_vecs, ds->queries, ds->n_queries, ds->dim, results);
        uint64_t t1 = timer_ticks();
        batch_ticks[rep] = t1 - t0;
    }
    pmu_stop();
    pmu_read(&pmu_counts);
}

void bench_measure_queries(const struct dataset *ds, uint32_t reps, uint32_t *results) {
    for (uint32_t rep = 0; rep < reps; rep++) {
        for (uint32_t q = 0; q < ds->n_queries; q++) {
            const float *query = ds->queries + (size_t)q * ds->dim;
            uint64_t t0 = timer_ticks();
            knn_brute_force(ds->vectors, ds->n_vecs, query, 1, ds->dim, results + (size_t)q * K);
            uint64_t t1 = timer_ticks();
            query_ticks[(size_t)rep * MAX_QUERIES + q] = t1 - t0;
        }
    }
}

static void print_kv(const char *key, uint64_t val) {
    uart_print(key);
    uart_print_u64_dec(val);
}

static void print_pmu(const char *name, uint64_t val) {
    uart_print("I pmu_");
    uart_print(name);
    print_kv("=", val);
    uart_print("\n");
}

void bench_report(const struct dataset *ds, uint32_t reps, const uint32_t *results, const char* platform) {
    uart_print("BEGIN brute dataset=siftsmall platform=");
    uart_print(platform);
    print_kv(" K=", K);
    print_kv(" NQ=", ds->n_queries);
    print_kv(" REPS=", reps);
    uart_print("\n");

    print_kv("I timer_freq=", timer_freq());
    uart_print("\n");
    print_kv("I timer_overhead_ticks=", timer_overhead());
    uart_print("\n");

    print_pmu("cycles", pmu_counts.cycles);
    for (uint32_t i = 0; i < PMU_N_EVENTS; i++) {
        print_pmu(pmu_events[i].name, pmu_counts.events[i]);
    }
    print_pmu("overflow", pmu_counts.overflow);

    for (uint32_t rep = 0; rep < reps; rep++) {
        print_kv("B ", rep);
        print_kv(" ", batch_ticks[rep]);
        uart_print(" s=");
        uart_print_float(ticks_to_s(batch_ticks[rep]), 6);
        uart_print("\n");
    }

    for (uint32_t rep = 0; rep < reps; rep++) {
        for (uint32_t q = 0; q < ds->n_queries; q++) {
            print_kv("T ", rep);
            print_kv(" ", q);
            print_kv(" ", query_ticks[(size_t)rep * MAX_QUERIES + q]);
            uart_print("\n");
        }
    }
    
    
    for (uint32_t q = 0; q < ds->n_queries; q++) {
        print_kv("R ", q);
        for (size_t k = 0; k < K; k++) {
            print_kv(" ", results[(size_t)q * K + k]);
        }
        uart_print("\n");
    }
    uart_print("END\n");
}