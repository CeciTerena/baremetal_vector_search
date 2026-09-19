#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>

// Naming convention: 
// print = everything that's formatted
// tx = transmit (from Pi to PC) raw bytes (or single byte)
// rx = receive (Pi receives from PC) raw bytes

void uart_init(void);

void uart_tx_byte(uint8_t c); // one byte
void uart_tx(const uint8_t *data, size_t len); //len bytes

void uart_print_u64_dec(uint64_t val); //64 because timer counter is 64-bit
void uart_print_hex(uint64_t val); 
void uart_print(const char *s); // text (with "\r\n")

uint8_t uart_rx_byte(void); //blocking until byte arrives
int16_t uart_try_rx_byte(void); // non blocking. returns int16 because of return -1 in case no byte arrives.


#endif