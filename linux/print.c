#include <inttypes.h>
#include <stdio.h>
#include "uart.h"

void uart_print(const char *s) {
    fputs(s, stdout);
}

void uart_print_u64_dec(uint64_t val) {
    printf("%" PRIu64, val);
}

void uart_print_hex(uint64_t val) {
    printf("0x%016" PRIx64, val);
}

void uart_print_float(float val, uint8_t precision) {
    printf("%.*f", precision, (double)val);
}