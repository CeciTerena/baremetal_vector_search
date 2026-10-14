#include <float.h>
#include "search.h"

static void init_topK(struct neighbour top[K]) {
    for (size_t k = 0; k < K; k++) {
        top[k].distance = FLT_MAX;
        top[k].id = UINT32_MAX;
    }
}

static void topK_insert(struct neighbour top[K], float distance, uint32_t id) {
    struct neighbour new_neighbour = {distance, id};
    size_t i = K - 1;
    while(i > 0 && top[i-1].distance > new_neighbour.distance) {
        top[i] = top[i-1];
        i--;
    }
    top[i] = new_neighbour;
}

static float l2_sqr(const float* query, const float* vec, size_t dim) {
    size_t i; 
    float distance = 0.0f;
    for (i = 0; i < dim; i++) {
        float temp = query[i] - vec[i];
        distance += temp * temp;
    }
    return distance;
}

void knn_brute_force(const float *vectors, uint32_t n_vecs, const float *queries, uint32_t n_queries, uint32_t dim, uint32_t *output) {
    struct neighbour top[K];
    for (uint32_t i = 0; i < n_queries; i++) {
        init_topK(top);        
        const float *query_i = queries + (size_t)i * dim;
        const float *vec_j = vectors;
        for (size_t j = 0; j < n_vecs; j++, vec_j += dim) {
            float distance = l2_sqr(query_i, vec_j, dim);
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