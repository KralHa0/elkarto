#include "l298n.h"

static TIM_HandleTypeDef htim2;

// Done: init PA0 (TIM2 CH1 PWM), PA2, PA4 as GPIO outputs
void l298nInit() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    GPIO_InitTypeDef gpioa2_4 = {0};
    gpioa2_4.Pin = GPIO_PIN_2 | GPIO_PIN_4;
    gpioa2_4.Mode = GPIO_MODE_OUTPUT_PP;
    gpioa2_4.Pull = GPIO_NOPULL;
    gpioa2_4.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpioa2_4);

    GPIO_InitTypeDef pa0 = {0};
    pa0.Alternate = GPIO_AF1_TIM2;
    pa0.Pull = GPIO_NOPULL;
    pa0.Mode = GPIO_MODE_AF_PP;
    pa0.Pin =  GPIO_PIN_0;
    pa0.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &pa0);

    //htim2
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 15; //16MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999; //1MHz
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim2);

    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0; //safe start
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

}

// IN1=HIGH, IN2=LOW, ENA=duty
void l298nForward(uint16_t duty) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
}

// IN1=LOW, IN2=HIGH, ENA=duty
void l298nReverse(uint16_t duty) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
}

// IN1=LOW, IN2=LOW, ENA=0
void l298nCoast() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
}
