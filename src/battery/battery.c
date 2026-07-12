#include "battery.h"

static ADC_HandleTypeDef hadc3;

void batteryInit(void) {
    __HAL_RCC_ADC3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &gpio);

    hadc3.Instance = ADC3;
    hadc3.Init.Resolution = ADC_RESOLUTION_12B;
    HAL_ADC_Init(&hadc3);

    ADC_ChannelConfTypeDef channel = {0};
    channel.Channel = ADC_CHANNEL_9;
    channel.Rank = 1;
    channel.SamplingTime = ADC_SAMPLETIME_84CYCLES;
    HAL_ADC_ConfigChannel(&hadc3, &channel);
}

float batteryReadVoltage(void) {
    HAL_ADC_Start(&hadc3);
    HAL_ADC_PollForConversion(&hadc3, HAL_MAX_DELAY);
    uint32_t adcRaw = HAL_ADC_GetValue(&hadc3);

    float vAdc = (adcRaw / 4095.0f) * 3.3f;
    float vBattery = vAdc * (10000.0f + 3300.0f) / 3300.0f;
    return vBattery;
}
