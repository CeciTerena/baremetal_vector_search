#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>

void uart_init(void);
void uart_putc(uint8_t c); // one byte
void uart_write(const uint8_t *data, size_t len); //len bytes
void uart_puts(const char *s); // text (with "\r\n")
uint8_t uart_getc(void); //blocking until byte arrives
int16_t uart_try_getc(void); // non blocking

#endif