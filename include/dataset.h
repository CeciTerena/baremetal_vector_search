#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>
#include <stdint.h>

#define VECS 0x53434556u

struct dataset_header {
    uint32_t check;
    uint32_t n;
    uint32_t dim;
    uint32_t pad;
};
static_assert(sizeof(struct dataset_header) == 16);

#endif