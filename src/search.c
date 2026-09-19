#include "search.h"

static void init_topK(struct neighbour top[K]) {
    for (size_t k = 0; k < K; k++) {
        top[k].distance = UINT32_MAX;
        top[k].id = UINT32_MAX;
    }
}

static void topK_insert(struct neighbour top[K], uint32_t distance, uint32_t id) {
    struct neighbour new_neighbour = {distance, id};
    size_t i = K - 1;
    while(i > 0 && top[i-1].distance > new_neighbour.distance) {
        top[i] = top[i-1];
        i--;
    }
    top[i] = new_neighbour;
}

static uint32_t l2_sqr(const uint8_t* query, const uint8_t* vec, size_t dim) {
    size_t i; 
    uint32_t distance = 0;
    for (i = 0; i < dim; i++) {
        int32_t temp = query[i] - vec[i];
        distance += temp * temp;
    }
    return distance;
}

void knn_brute_force(const uint8_t *vectors, uint32_t n_vecs, const uint8_t *queries, uint32_t n_queries, uint32_t dim, uint32_t *output) {
    struct neighbour top[K];
    for (uint32_t i = 0; i < n_queries; i++) {
        init_topK(top);        
        const uint8_t *query_i = queries + (size_t)i * dim;
        const uint8_t *vec_j = vectors;
        for (size_t j = 0; j < n_vecs; j++, vec_j += dim) {
            uint32_t distance = l2_sqr(query_i, vec_j, dim);
            if (distance < top[K-1].distance) {
                topK_insert(top, distance, j);
            }   
        }
        for (size_t k = 0; k < K; k++) {
            size_t query_id = (size_t)i * K + k;
            output[query_id] = top[k].id;
        }
    }
}