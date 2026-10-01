#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "uart.h"
#include "dataset.h"
#include "search.h"
#include "bench.h"

static uint32_t results[MAX_QUERIES * K];

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
        fscanf(f, "%" SCNu64, &khz); // on failure khz stays 0
        fclose(f);
    }
    return khz * 1000;
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

    uart_print("--------------------------------------\n");
    bench_measure(&ds, MAX_REPS, results);
    bench_report(&ds, MAX_REPS, results, "linux");
    uart_print("--------------------------------------\n");
    return 0;
}