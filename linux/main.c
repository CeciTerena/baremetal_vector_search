#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sched.h>
#include <sys/resource.h>
#include <string.h>
#include "uart.h"
#include "dataset.h"
#include "search.h"
#include "bench.h"
#include "pmu.h"


// arbitrary 16kb, should be big enough
static char interrupts_before[16384];
static char interrupts_after[16384];
static uint32_t results[MAX_QUERIES * K];

static void snapshot(const char *path, char *buf, size_t cap) {
    buf[0] = '\0';
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        return;
    }
    size_t n = fread(buf, 1, cap - 1, f);
    buf[n] = '\0';
    fclose(f);
}

// idea - print every line of text from the interrupts with the tag BEFORE or AFTER
// then a program outside of the PI itself can parse it by subtracting BEFORE from AFTER to get the interrupts during the run
// so theres no interference during the run()
static void print_tagged(const char *tag, char *text) {
    for (char *line = strtok(text, "\n"); line != NULL; line = strtok(NULL, "\n")) {
        uart_print(tag);
        uart_print(line);
        uart_print("\n");
    }
}


static void *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    void *buf = malloc(size);
    if (buf != NULL && fread(buf, 1, size, f) != (size_t)size) {
        free(buf);
        buf = NULL;
    }
    fclose(f);
    return buf;
}

static uint64_t cpu_hz(void) {
    uint64_t khz = 0;
    FILE *f = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq", "r");
    if (f != NULL) {
        fscanf(f, "%" SCNu64, &khz);
        fclose(f);
    }
    return khz * 1000;
}

static void print_info(const char *key, uint64_t val) {
    uart_print("I ");
    uart_print(key);
    uart_print("=");
    uart_print_u64_dec(val);
    uart_print("\n");
}

static void print_os_events(struct rusage before, struct rusage after, int cpu_start, int cpu_end) {
    print_info("ctx_switch_voluntary", after.ru_nvcsw - before.ru_nvcsw);
    print_info("ctx_switch_forced", after.ru_nivcsw - before.ru_nivcsw);
    print_info("page_faults_minor", after.ru_minflt - before.ru_minflt);
    print_info("page_faults_major", after.ru_majflt - before.ru_majflt);
    print_info("cpu_start", cpu_start);
    print_info("cpu_end", cpu_end);
    print_tagged("INTERRUPTS_BEFORE ", interrupts_before);
    print_tagged("INTERRUPTS_AFTER ", interrupts_after);
}

int main(void) {
    void *data = read_file("/boot/firmware/data.bin");
    if (data == NULL) {
        uart_print("Dataset ERR: cannot read file\n");
        return 1;
    }

    struct dataset ds;
    if (dataset_load(&ds, data) == false) {
        uart_print("Dataset ERR\n");
        return 1;
    }
    uart_print("Dataset OK\n");

    uart_print("I cpu_hz=");
    uart_print_u64_dec(cpu_hz());
    uart_print("\n");

    pmu_init();

    uart_print("--------------------------------------\n");
    struct rusage before, after;

    bench_warmup(&ds, results);

    int cpu_start = sched_getcpu();
    snapshot("/proc/interrupts", interrupts_before, sizeof interrupts_before);
    getrusage(RUSAGE_SELF, &before);

    bench_measure_batch(&ds, MAX_REPS, results);

    getrusage(RUSAGE_SELF, &after);
    snapshot("/proc/interrupts", interrupts_after, sizeof interrupts_after);
    int cpu_end = sched_getcpu();

    bench_measure_queries(&ds, MAX_REPS, results);

    print_os_events(before, after, cpu_start, cpu_end);

    uart_print("--------------------------------------\n");
    bench_report(&ds, MAX_REPS, results, "linux");
    uart_print("--------------------------------------\n");
    return 0;
}