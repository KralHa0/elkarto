#include "gas/adc.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

void SysTick_Handler(void) { HAL_IncTick(); }

UART_HandleTypeDef huart1;

void uart1Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart1);
}

void uartPrint(const char *msg) {
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

void testblinker(uint32_t delay) {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
    HAL_Delay(delay);
}

int main(void) {
    HAL_Init();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = GPIO_PIN_4;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    uart1Init();
    adc1Init();

    char buf[32];
    uint32_t delay;
    while (1) {
        uint32_t val = adc1Read();
        snprintf(buf, sizeof(buf), "ADC: %lu\r\n", val);
        uartPrint(buf);
        delay = (val * 1900 / 4095) + 100;
        testblinker(delay);
    }
}
