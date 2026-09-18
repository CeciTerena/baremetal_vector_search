#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

typedef enum {
    GPIO_FN_INPUT = 0b000,
    GPIO_FN_OUTPUT = 0b001,
    GPIO_FN_ALT0 = 0b100,
    GPIO_FN_ALT1 = 0b101,
    GPIO_FN_ALT2 = 0b110,
    GPIO_FN_ALT3 = 0b111,
    GPIO_FN_ALT4 = 0b011,
    GPIO_FN_ALT5 = 0b010,
} gpio_fn_t;

typedef enum {
    GPIO_PULL_NONE = 0b00,
    GPIO_PULL_UP = 0b01,
    GPIO_PULL_DOWN = 0b10,
} gpio_pull_t;

void gpio_set_function(uint32_t pin, gpio_fn_t fn);
void gpio_set_pull(uint32_t pin, gpio_pull_t pull);
void gpio_set(uint32_t pin);   // -->high
void gpio_clear(uint32_t pin); // --> low

#endif