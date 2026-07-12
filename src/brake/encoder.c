#include "encoder.h"

static TIM_HandleTypeDef htim3;

// Done: init PA6/PA7 as TIM3 AF, configure TIM3 in encoder mode
void encoderInit() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    //GPIO PA6 and PA7 enable
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &gpio);

    //TIM3 enable
    htim3.Instance = TIM3;
    htim3.Init.Period = 0xFFFF;
    htim3.Init.Prescaler = 0;

    TIM_Encoder_InitTypeDef sConfig;
    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;

    sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC1Filter = 6;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    
    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Filter = 6;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    HAL_TIM_Encoder_Init(&htim3, &sConfig);

    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
}

int32_t encoderGetPosition() {
    return (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
}

void encoderReset(void) {
    __HAL_TIM_SET_COUNTER(&htim3, 0);
}
