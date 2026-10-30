#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>
#include <stdint.h>

#define VECS 0x53434556u
#define QURY 0x59525551u
#define CENT 0x544E4543u
#define OFFS 0x5346464Fu
#define LIDS 0x5344494Cu
#define IVEC 0x43455649u

struct dataset {
    const float *vectors;
    const float *queries;
    uint32_t n_vecs;
    uint32_t n_queries;
    uint32_t dim; 
    const float *centroids;
    const uint32_t *list_offsets;
    const uint32_t *original_ids;
    const float *list_vectors;
    uint32_t n_lists;
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