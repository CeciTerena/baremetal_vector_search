#include "uart.h"
#include "gpio.h"
#include "peripherals.h"

#define UART0_TX_PIN 14
#define UART0_RX_PIN 15

#define UART_BAUD_RATE 115200UL

#define UART_BAUD_DIVISOR_X64 ((4 * UART_CLOCK_HZ + UART_BAUD_RATE / 2) / UART_BAUD_RATE)

void uart_init(void) {
    while (UART0_FR & UART0_FR_BUSY) {} // wait for FIFO output to be clear
    UART0_CR = 0;

    gpio_set_function(UART0_TX_PIN, GPIO_FN_ALT0);
    gpio_set_function(UART0_RX_PIN, GPIO_FN_ALT0);
    gpio_set_pull(UART0_TX_PIN, GPIO_PULL_NONE); // output, pull is irrelevant
    gpio_set_pull(UART0_RX_PIN, GPIO_PULL_UP); // idle high if unplugged

    UART0_ICR = UART0_ICR_ALL;

    UART0_IBRD = UART_BAUD_DIVISOR_X64 / 64;
    UART0_FBRD = UART_BAUD_DIVISOR_X64 % 64;

    // this needs to come after IBRD, FBRD. changes in these only take effect on the next LCHR write. 
    UART0_LCRH = UART0_LCRH_WLEN_8BIT | UART0_LCRH_FEN; // 8-bit word, FIFO on

    UART0_CR = UART0_CR_UARTEN | UART0_CR_TXE | UART0_CR_RXE;
}

// -------- TX ----------------

void uart_tx_byte(uint8_t c) {
    while (UART0_FR & UART0_FR_TXFF) {} // wait while transmit fifo queue is full
    UART0_DR = c;
}

void uart_tx(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uart_tx_byte(data[i]);
    }
}

// -------- PRINT ----------------

void uart_print_u64_dec(uint64_t val) {
    char buf[21];
    int i = 0;
    if (val == 0) {
        uart_tx_byte('0');
        return;
    }
    // digits extracted from end to start. 
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    // then outputted in the correct order
    for (int j = i - 1; j >= 0; j--) {
        uart_tx_byte(buf[j]);
    }
}

void uart_print_hex(uint64_t val) {
    static const char hex_digits[] = "0123456789abcdef";
    uart_print("0x");
    for (int shift = 60; shift >= 0; shift -= 4) {
        uart_tx_byte(hex_digits[(val >> shift) & 0xF]);
    }
}

void uart_print(const char *s) {
    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == '\n') {
            uart_tx_byte('\r');
        }
        uart_tx_byte(s[i]);
    }
}

// -------- RX ----------------

uint8_t uart_rx_byte(void) {
    while (UART0_FR & UART0_FR_RXFE) {} // while recv fifo queue is empty, wait --> BLOCKING!!!!!
    return (uint8_t)(UART0_DR & 0xFFu); // keeps only the low 8 bits
}

int16_t uart_try_rx_byte(void) {
    if (UART0_FR & UART0_FR_RXFE) { 
        return -1;  
    }
    return (int16_t)(UART0_DR & 0xFFu);
}