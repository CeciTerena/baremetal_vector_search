#define _GNU_SOURCE

#include <linux/perf_event.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "pmu.h"

#define CPU_CYCLES 0x11

static int leader = -1;

static int open_counter(uint16_t event, int group) {
    struct perf_event_attr attr;
    memset(&attr, 0, sizeof attr);
    attr.type = PERF_TYPE_RAW;
    attr.size = sizeof attr;
    attr.config = event;
    attr.disabled = (group == -1);
    attr.read_format = PERF_FORMAT_GROUP;
    return syscall(SYS_perf_event_open, &attr, 0, -1, group, 0);
}

void pmu_init(void) {
    leader = open_counter(CPU_CYCLES, -1);
    for (uint32_t i = 0; i < PMU_N_EVENTS; i++) {
        open_counter(pmu_events[i].id, leader);
    }
}

void pmu_start(void) {
    ioctl(leader, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
    ioctl(leader, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
}

void pmu_stop(void) {
    ioctl(leader, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
}

void pmu_read(struct pmu_counts *out) {
    uint64_t buf[2 + PMU_N_EVENTS];
    read(leader, buf, sizeof buf);
    out->cycles = buf[1];
    for (uint32_t i = 0; i < PMU_N_EVENTS; i++) {
        out->events[i] = buf[2 + i];
    }
    out->overflow = 0;
}