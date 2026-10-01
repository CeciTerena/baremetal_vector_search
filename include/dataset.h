#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>
#include <stdint.h>

#define VECS 0x53434556u
#define QURY 0x59525551u

struct dataset {
    const uint8_t *vectors;
    const uint8_t *queries;
    uint32_t n_vecs;
    uint32_t n_queries;
    uint32_t dim; 
};

struct dataset_header {
    uint32_t magic;
    uint32_t n;
    uint32_t dim;
    uint32_t pad;
};
static_assert(sizeof(struct dataset_header) == 16);

bool dataset_load(struct dataset *ds, const void *base);

#endif