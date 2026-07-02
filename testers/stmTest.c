#include "stm32f4xx.h"

/* External LED: PA6, active high, 330 ohm series resistor */

int main(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    GPIOA->MODER &= ~(3U << (6 * 2));
    GPIOA->MODER |=  (1U << (6 * 2)); /* PA6 output */

    while (1) {
        GPIOA->BSRR = (1U << 6);         /* SET PA6 - LED on */
        for (volatile uint32_t i = 0; i < 3000000; i++);
        GPIOA->BSRR = (1U << (6 + 16));  /* RESET PA6 - LED off */
        for (volatile uint32_t i = 0; i < 3000000; i++);
    }
}
