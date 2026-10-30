#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>
#include <stdint.h>
#include "dataset.h"

#define K 10
#define NPROBE 8

struct neighbour{
    float distance;
    uint32_t id;
};

struct ivf_list {
    const float *vectors; 
    const uint32_t *ids; 
    uint32_t size;
};

void knn_brute_force(const struct dataset *ds, uint32_t *output);
void knn_brute_force_query(const struct dataset *ds, const float *query_i, uint32_t *output);
void knn_ivf(const struct dataset *ds, uint32_t *output);
void knn_ivf_query(const struct dataset *ds, const float *query_i, uint32_t *output);


#endif