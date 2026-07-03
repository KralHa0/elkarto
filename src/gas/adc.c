#include "adc.h"

ADC_HandleTypeDef hadc;

void adc1Init(void) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio);

    hadc.Instance = ADC1;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    HAL_ADC_Init(&hadc);

    ADC_ChannelConfTypeDef channel = {0};
    channel.Channel = ADC_CHANNEL_3;
    channel.Rank = 1;
    channel.SamplingTime = ADC_SAMPLETIME_84CYCLES;
    HAL_ADC_ConfigChannel(&hadc, &channel);
}

uint32_t adc1Read(void) {
    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc);
}