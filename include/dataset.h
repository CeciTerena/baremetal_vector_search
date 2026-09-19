#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>
#include <stdint.h>

#define VECS 0x53434556u
#define QURY 0x59525551u

struct dataset_header {
    uint32_t magic;
    uint32_t n;
    uint32_t dim;
    uint32_t pad;
};
static_assert(sizeof(struct dataset_header) == 16);

static inline size_t align16(size_t x) {
    return (x + 15) & ~(size_t)15;
}

static inline const struct dataset_header *get_next_header(const struct dataset_header *header) {
    size_t block_bytes = align16(sizeof(*header) + (size_t)header->n * header->dim);
    return (const struct dataset_header *)((const uint8_t *)header + block_bytes);
}

#endif