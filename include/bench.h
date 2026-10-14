#ifndef BENCH_H
#define BENCH_H

#include <stdint.h>
#include "dataset.h"

#define MAX_REPS 20
#define MAX_QUERIES 100

void bench_warmup(const struct dataset *ds, uint32_t *results);
void bench_measure(const struct dataset *ds, uint32_t reps, uint32_t *results);

// FORMATTTTT
//                      BEGIN <algo> dataset=<name> platform=<baremetal|linux> K=<k> NQ=<n> REPS=<r>
// Info -->             I <key>=<value>
// Batch Time -->       B <rep> <ticks> s=<seconds>      
// Per Query Time -->   T <rep> <query> <ticks> 
// Results -->          R <query> <id_0> ... <id_K-1>
//                      END
void bench_report(const struct dataset *ds, uint32_t reps, const uint32_t *results, const char *platform);

#endif