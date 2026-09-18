#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include <stdint.h>

#define PERIPHERAL_BASE 0xFE000000UL

// Treat a physical address as a 32-bit hardware register
#define REG32(addr) (*(volatile uint32_t *)(addr))

#define GPFSEL_PINS_PER_REG 10
#define GPFSEL_BITS_PER_PIN 3
#define PUP_PDN_PINS_PER_REG 16
#define PUP_PDN_BITS_PER_PIN 2

//so it matches the width of the pin bits. 
#define FIELD_MASK(width) ((1u << (width)) - 1u)

// ---------- GPIO ---------
#define GPIO_BASE (PERIPHERAL_BASE + 0x200000UL)

// n = register index (NOT PIN NUM)
#define GPFSEL(n) REG32(GPIO_BASE + 0x00 + 4 * (n)) // 3 bits/pin, 10 pins/reg
#define GPSET(n) REG32(GPIO_BASE + 0x1C + 4 * (n)) // Write 1 --> high
#define GPCLR(n) REG32(GPIO_BASE + 0x28 + 4 * (n)) // Write 1 --> low
#define PUP_PDN_CNTRL_REG(n) REG32(GPIO_BASE + 0xE4 + 4 * (n)) // 2 bits/pin, 16 pins/reg

#define ACT_LED_PIN 42

// ---------- UART0 -------------
#define UART0_BASE (PERIPHERAL_BASE + 0x201000UL)
#define UART0_DR REG32(UART0_BASE + 0x00)
#define UART0_FR REG32(UART0_BASE + 0x18)
#define UART0_IBRD REG32(UART0_BASE + 0x24)
#define UART0_FBRD REG32(UART0_BASE + 0x28)
#define UART0_LCRH REG32(UART0_BASE + 0x2C)
#define UART0_CR REG32(UART0_BASE + 0x30)
#define UART0_ICR REG32(UART0_BASE + 0x44)

#define UART0_FR_BUSY (1u << 3)
#define UART0_FR_RXFE (1u << 4)
#define UART0_FR_TXFF (1u << 5)
#define UART0_LCRH_FEN (1u << 4)
#define UART0_LCRH_WLEN_8BIT (0x3u << 5)
#define UART0_CR_UARTEN (1u << 0)
#define UART0_CR_TXE (1u << 8)
#define UART0_CR_RXE (1u << 9)
#define UART0_ICR_ALL 0x7FFu

#define UART_CLOCK_HZ 48000000UL // firmware

#endif