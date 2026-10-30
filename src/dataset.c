#include "uart.h"
#include "checksum.h"
#include "mem_management.h"
#include "dataset.h"

enum block {BLOCK_VECS, BLOCK_QURY, BLOCK_CENT, BLOCK_OFFS, BLOCK_LIDS, BLOCK_IVEC, N_BLOCKS};
static const uint32_t block_magics[N_BLOCKS] = {VECS, QURY, CENT, OFFS, LIDS, IVEC};
static const char *block_names[N_BLOCKS] = {"VECS", "QURY", "CENT", "OFFS", "LIDS", "IVEC"};

static inline const struct dataset_header *get_next_header(const struct dataset_header *header) {
    size_t block_bytes = align16(sizeof(*header) + (size_t)header->n * header->dim * sizeof(float));
    return (const struct dataset_header *)((const uint8_t *)header + block_bytes);
}

static const uint8_t *block_data(const struct dataset_header *header) {
    return (const uint8_t *)header + sizeof(*header);
}

static bool check_block(enum block block, const struct dataset_header *header) {
    if (header->magic != block_magics[block]) {
        uart_print("Dataset header magic mismatch\n");
        uart_print(block_names[block]);
        uart_print("\n");
        return false; 
    }
    size_t data_bytes = (size_t)header->n * header->dim * sizeof(float);
    uart_print(block_names[block]);
    uart_print(": n=");
    uart_print_u64_dec(header->n);
    uart_print(" dim=");
    uart_print_u64_dec(header->dim);
    uart_print(" checksum=");
    uart_print_hex(checksum_fnv1a_32(block_data(header), data_bytes));
    uart_print("\n");
    return true;
}

static bool load_headers(const struct dataset_header *headers[N_BLOCKS], const void *base) {
    const struct dataset_header *header = base;
    for (enum block block = 0; block < N_BLOCKS; block++) {
        if (check_block(block, header) == false) {
            return false;
        }
        headers[block] = header;
        header = get_next_header(header);
    }
    return true;
}

static bool dims_match(const struct dataset_header *headers[N_BLOCKS]) {
    uint32_t dim = headers[BLOCK_VECS]->dim;
    bool queries_match = headers[BLOCK_QURY]->dim == dim;
    bool centroids_match = headers[BLOCK_CENT]->dim == dim;
    bool list_vectors_match = headers[BLOCK_IVEC]->dim == dim;
    return queries_match && centroids_match && list_vectors_match;
}

bool dataset_load(struct dataset *ds, const void *base) {
    const struct dataset_header *headers[N_BLOCKS];
    if (load_headers(headers, base) == false) {
        return false;
    }
    if (dims_match(headers) == false) {
        uart_print("Mismatch: block dimensions differ\n");
        return false;
    }
    ds->vectors = (const float *)block_data(headers[BLOCK_VECS]);
    ds->queries = (const float *)block_data(headers[BLOCK_QURY]);
    ds->centroids = (const float *)block_data(headers[BLOCK_CENT]);
    ds->list_offsets = (const uint32_t *)block_data(headers[BLOCK_OFFS]);
    ds->original_ids = (const uint32_t *)block_data(headers[BLOCK_LIDS]);
    ds->list_vectors = (const float *)block_data(headers[BLOCK_IVEC]);
    ds->n_vecs = headers[BLOCK_VECS]->n;
    ds->n_queries = headers[BLOCK_QURY]->n;
    ds->dim = headers[BLOCK_VECS]->dim;
    ds->n_lists = headers[BLOCK_CENT]->n;
    return true;
}

