#include "gpio.h"
#include "peripherals.h"

#define GPIO_MAX_PIN 57 

void gpio_set_function(uint32_t pin, gpio_fn_t fn) {
    if (pin > GPIO_MAX_PIN) return; // later add some error handling!!!

    uint32_t reg = pin / GPFSEL_PINS_PER_REG;
    uint32_t shift = GPFSEL_BITS_PER_PIN * (pin % GPFSEL_PINS_PER_REG);

    uint32_t val = GPFSEL(reg);
    val &= ~(FIELD_MASK(GPFSEL_BITS_PER_PIN) << shift);
    val |= (uint32_t)fn << shift;
    GPFSEL(reg) = val;
}

void gpio_set(uint32_t pin) {
    if (pin > GPIO_MAX_PIN) return;
    GPSET(pin / 32) = 1u << (pin % 32); // 1 bit per pin, write-1 only
}

void gpio_set_pull(uint32_t pin, gpio_pull_t pull) {
    if (pin > GPIO_MAX_PIN) return; // later add some error handling!!!

    uint32_t reg = pin / PUP_PDN_PINS_PER_REG; // 16 pins per PUP_PND register
    uint32_t shift = PUP_PDN_BITS_PER_PIN * (pin % PUP_PDN_PINS_PER_REG); // 2 bits/pin

    uint32_t val = PUP_PDN_CNTRL_REG(reg);
    val &= ~(FIELD_MASK(PUP_PDN_BITS_PER_PIN) << shift);
    val |= (uint32_t)pull << shift;
    PUP_PDN_CNTRL_REG(reg) = val;  
}

void gpio_clear(uint32_t pin) {
    if (pin > GPIO_MAX_PIN) return;
    GPCLR(pin / 32) = 1u << (pin % 32);
}