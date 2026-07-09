#include "debug.h"
#include <string.h>

static UART_HandleTypeDef huart1;

static uint8_t rxByte;
static char lineBuf[64];
static uint8_t lineLen = 0;
static volatile uint8_t lineReady = 0;

void debugInit(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart1.Instance        = USART1;
    huart1.Init.BaudRate   = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits   = UART_STOPBITS_1;
    huart1.Init.Parity     = UART_PARITY_NONE;
    huart1.Init.Mode       = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart1);

    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);

    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef ledgpio = {0};
    ledgpio.Pin   = GPIO_PIN_2;
    ledgpio.Mode  = GPIO_MODE_OUTPUT_PP;
    ledgpio.Pull  = GPIO_NOPULL;
    ledgpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &ledgpio);
}

void debugPrint(const char *msg) {
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

void debugBlink(uint32_t delay) {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
    HAL_Delay(delay);
}

uint8_t debugReadLine(char *buf, uint8_t maxLen) {
    if (!lineReady) return 0;

    uint8_t len = lineLen < (maxLen - 1) ? lineLen : (maxLen - 1);
    memcpy(buf, lineBuf, len);
    buf[len] = '\0';

    lineLen = 0;
    lineReady = 0;
    return 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance != USART1) return;

    if (!lineReady) {
        if (rxByte == '\n') {
            lineReady = 1;
        } else if (rxByte != '\r' && lineLen < sizeof(lineBuf) - 1) {
            lineBuf[lineLen++] = (char)rxByte;
        }
    }

    HAL_UART_Receive_IT(&huart1, &rxByte, 1);
}

void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart1);
}
