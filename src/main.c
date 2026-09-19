#include "gpio.h"
#include "peripherals.h"
#include "uart.h"
#include "checksum.h"
#include "mem_map.h"
#include "dataset.h"

static void delay(uint32_t count) {
    while (count--) {
        __asm__("nop");
    }
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

static bool check_dataset(void) {
    const struct dataset_header *vec_header = (const struct dataset_header *)DATASET_BASE;
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
    return true;
}

void main(void) {
    gpio_set_function(ACT_LED_PIN, GPIO_FN_OUTPUT);
    gpio_set(ACT_LED_PIN);

    uart_init();

    uart_print("Ok we will try out this checksum thing now\n");
    delay(1000);
    bool ok = check_dataset();
    if (ok == false) {
        uart_print("Dataset ERR\n");
        return;
    }
    uart_print("Dataset OK\n");
    return;
}