#include "stm32f4xx_hal.h"

/* External LED: PA6, active high, 330 ohm series resistor */

void SysTick_Handler(void) {
    HAL_IncTick();
}

int main(void) {
    HAL_Init();

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = GPIO_PIN_6;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    while (1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
        HAL_Delay(500);
    }
}
