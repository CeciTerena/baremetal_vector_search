#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>
#include <stdint.h>

#define K 10

struct neighbour{
    uint32_t distance;
    uint32_t id;
};

void knn_brute_force(const uint8_t *vectors, uint32_t n_vecs, const uint8_t *queries, uint32_t n_queries, uint32_t dim, uint32_t *output);

#endif