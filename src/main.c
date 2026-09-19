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

static bool check_dataset(void) {
    const struct dataset_header *header = (const struct dataset_header *)DATASET_BASE;
    const uint8_t *vectors = (const uint8_t *)DATASET_BASE + sizeof(*header);
    if (header->check != VECS) {
        uart_print("Dataset header check failed\n");
        return false;
    }
    size_t data_bytes = (size_t)header->n * (size_t)header->dim;
    uart_print("\nNumber of vectors: ");
    uart_print_u64_dec(header->n);
    uart_print("\nNumber of dimentions: ");
    uart_print_u64_dec(header->dim);
    uart_print("\nChecksum: ");
    uart_print_hex(checksum_fnv1a_32(vectors, data_bytes));
    return true;
}

void main(void) {
    gpio_set_function(ACT_LED_PIN, GPIO_FN_OUTPUT);
    gpio_set(ACT_LED_PIN);

    uart_init();

    uart_print("Ok we will try out this checksum thing now\n");
    delay(1000);

    check_dataset();

    // uart_print("Heeeeeeeeeeyyyyy\n");
    // delay(10000);
    // for (int i = 0; i < 20; i++) {
    //     uart_print_u64_dec(i);
    // }
    // while (1) {
    //     int16_t c = uart_try_rx_byte();
    //     while (c >= 0) {
    //         uart_tx_byte((uint8_t)c);
    //         c = uart_try_rx_byte();
    //     }
    // }
}