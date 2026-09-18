#include "gpio.h"
#include "peripherals.h"
#include "uart.h"

static void delay(uint32_t count) {
    while (count--) {
        __asm__("nop");
    }
}

void main(void) {
    gpio_set_function(ACT_LED_PIN, GPIO_FN_OUTPUT);
    gpio_set(ACT_LED_PIN);

    uart_init();

    uart_puts("Heeeeeeeeeeyyyyy\n");

    while (1) {
        gpio_set(ACT_LED_PIN);
        delay(50000000);
        gpio_clear(ACT_LED_PIN);
        delay(50000000);

        int16_t c = uart_try_getc();
        while (c >= 0) {
            uart_putc((uint8_t)c);
            c = uart_try_getc();
        }
    }
}