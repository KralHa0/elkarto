#include "adc.h"
static ADC_HandleTypeDef hadc2;

// Done: init PC2 as analog input, configure ADC2 CH12
void brakeAdcInit() {
    __HAL_RCC_ADC2_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_2;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &gpio);

    hadc2.Instance = ADC2;
    hadc2.Init.Resolution = ADC_RESOLUTION_12B;
    HAL_ADC_Init(&hadc2);

    ADC_ChannelConfTypeDef channel = {0};
    channel.Channel = ADC_CHANNEL_12;
    channel.Rank = 1;
    channel.SamplingTime = ADC_SAMPLETIME_84CYCLES;
    HAL_ADC_ConfigChannel(&hadc2, &channel);
}

// Done: read brake pedal position
uint32_t brakeAdcRead() {
    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc2);
}
