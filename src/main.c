#include "gpio.h"
#include "peripherals.h"
#include "uart.h"
#include "checksum.h"
#include "mem_map.h"
#include "dataset.h"
#include "search.h"

#define MAX_QUERIES 100
static uint32_t results[MAX_QUERIES * K];

static void delay(uint32_t count) {
    while (count--) {
        __asm__("nop");
    }
}

// Log format
// BEGIN <algo> dataset=<name> K=<k> NQ=<n>
// R <query> <id_0> ... <id_K-1>  --> one line per query
// END
// the dataset is still hardcoded... FIX THAT LATER AAAAA
static void print_results(const struct dataset *ds, const uint32_t *res) {
    uart_print("BEGIN brute dataset=siftsmall K=");
    uart_print_u64_dec(K);
    uart_print(" NQ=");
    uart_print_u64_dec(ds->n_queries);
    uart_print("\n");

    for (uint32_t query = 0; query < ds->n_queries; query++) {
        uart_print("R ");
        uart_print_u64_dec(query);
        for (size_t k = 0; k < K; k++) {
            uart_print(" ");
            uart_print_u64_dec(res[(size_t)query * K + k]);
        }
        uart_print("\n");
    }
    uart_print("END\n");
}

void main(void) {
    uart_init();
    delay(1000);
    struct dataset ds;

    bool ok = dataset_load(&ds);
    if (ok == false) {
        uart_print("Dataset ERR\n");
        return;
    }
    uart_print("Dataset OK\n");

    knn_brute_force(ds.vectors, ds.n_vecs, ds.queries, ds.n_queries, ds.dim, results);
    print_results(&ds, results);
    return;
}