#include <float.h>
#include "search.h"
#include "dataset.h"

static void init_top(struct neighbour top[K], size_t size) {
    for (size_t k = 0; k < size; k++) {
        top[k].distance = FLT_MAX;
        top[k].id = UINT32_MAX;
    }
}

static void top_insert(struct neighbour top[K], float distance, uint32_t id, size_t size) {
    struct neighbour new_neighbour = {distance, id};
    size_t i = size - 1;
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

static const float *get_vector(const float *vectors, uint32_t index, uint32_t dim) {
    return vectors + (size_t)index * dim;
}

void knn_brute_force_query(const struct dataset *ds, const float *query_i, uint32_t *output) {
    struct neighbour top[K];
    init_top(top, K);    
    const float *vec_j = ds->vectors;
    for (size_t j = 0; j < ds->n_vecs; j++, vec_j += ds->dim) {
        float distance = l2_sqr(query_i, vec_j, ds->dim);
        if (distance < top[K-1].distance) {
            top_insert(top, distance, j, K);
        }   
    }
    for (size_t k = 0; k < K; k++) {
        output[k] = top[k].id;
    }
}

void knn_brute_force(const struct dataset *ds, uint32_t *output) {
    for (uint32_t i = 0; i < ds->n_queries; i++) {    
        const float *query_i = get_vector(ds->queries, i, ds->dim);
        knn_brute_force_query(ds, query_i, output + (size_t)i * K);
    }
}

struct ivf_list get_list(const struct dataset *ds, uint32_t list_num) {
    uint32_t start = ds->list_offsets[list_num];
    uint32_t end = ds->list_offsets[list_num + 1];
    struct ivf_list list = {
        .vectors = get_vector(ds->list_vectors, start, ds->dim),
        .ids = ds->original_ids + start,
        .size = end - start,
    };
    return list;
}


void find_top_centroids(struct neighbour *top, const struct dataset *ds, const float* query_i) {
    init_top(top, NPROBE); 
    for (size_t i = 0; i < ds->n_lists; i++) {
        const float *centroid = get_vector(ds->centroids, i, ds->dim);
        float distance = l2_sqr(query_i, centroid, ds->dim);
        if (distance < top[NPROBE-1].distance) {
            top_insert(top, distance, i, NPROBE);
        }   
    }
}

void scan(struct neighbour *top, struct neighbour *top_centroids, const struct dataset *ds, const float* query_i) { 
    init_top(top, K);  
    for (int i = 0; i < NPROBE; i++) {
        uint32_t list_num = top_centroids[i].id;
        struct ivf_list list = get_list(ds, list_num);
        const float* vec_j = list.vectors;
        for (size_t j = 0; j < list.size; j++, vec_j += ds->dim) {
            float distance = l2_sqr(query_i, vec_j, ds->dim);
            if (distance < top[K-1].distance) {
                top_insert(top, distance, list.ids[j], K);
            }   
        }
    }
}

void knn_ivf_query(const struct dataset *ds, const float *query_i, uint32_t *output) {
    struct neighbour top_centroids[NPROBE];
    struct neighbour top_neighbours[K];
    find_top_centroids(top_centroids, ds, query_i);
    scan(top_neighbours, top_centroids, ds, query_i);
    for (size_t k = 0; k < K; k++) {
        output[k] = top_neighbours[k].id;
    }   
}

void knn_ivf(const struct dataset *ds, uint32_t *output) {  
    for (uint32_t i = 0; i < ds->n_queries; i++) {
        const float *query_i = get_vector(ds->queries, i, ds->dim);
        knn_ivf_query(ds, query_i, output + (size_t)i * K);
    }
}