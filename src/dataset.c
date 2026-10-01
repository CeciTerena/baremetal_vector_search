#include "uart.h"
#include "checksum.h"
#include "mem_management.h"
#include "dataset.h"

static inline const struct dataset_header *get_next_header(const struct dataset_header *header) {
    size_t block_bytes = align16(sizeof(*header) + (size_t)header->n * header->dim);
    return (const struct dataset_header *)((const uint8_t *)header + block_bytes);
}

static bool check_block(uint32_t magic, const struct dataset_header *header) {
    const uint8_t *data = (const uint8_t *)header + sizeof(*header);
    if (header->magic != magic) {
        uart_print("Dataset header magic mismatch\n");
        return false;
    }
    size_t data_bytes = (size_t)header->n * (size_t)header->dim;
    uart_print("\nNumber of vectors: ");
    uart_print_u64_dec(header->n);
    uart_print("\nNumber of dimentions: ");
    uart_print_u64_dec(header->dim);
    uart_print("\nChecksum: ");
    uart_print_hex(checksum_fnv1a_32(data, data_bytes));
    uart_print("\n");
    return true;
}

bool dataset_load(struct dataset *ds, const void *base){
    const struct dataset_header *vec_header = (const struct dataset_header *)base;

    uart_print("--------- vector block check --------- \n");
    if (check_block(VECS, vec_header) == false){
        uart_print("FAIL: vector block check\n");
        return false;
    }

    const struct dataset_header *query_header = get_next_header(vec_header);
    uart_print("--------- query block check --------- \n");
    if (check_block(QURY, query_header) == false){
        uart_print("FAIL: query block check\n");
        return false;
    }

    if (vec_header->dim != query_header->dim){
        uart_print("Mismatch: vector dimentions differ from query dimentions\n");
        return false;
    }

    const uint8_t *vectors = (const uint8_t *)vec_header + sizeof(*vec_header);
    const uint8_t *queries = (const uint8_t *)query_header + sizeof(*query_header);
    uint32_t n_vecs = vec_header->n;
    uint32_t n_queries = query_header->n;
    uint32_t dim = vec_header->dim;
    
    ds->dim = dim; 
    ds->n_queries = n_queries;
    ds->n_vecs = n_vecs;
    ds->queries = queries;
    ds->vectors = vectors;

    return true;

}

