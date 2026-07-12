#include "speedSensor.h"

static uint32_t lastPulseTime;
static uint32_t lastPeriodMs;

void speedSensorInit(){
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio= {0};
    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_IT_FALLING;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void EXTI0_IRQhandler(void){
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void HAL_GPIO_EXTI_CALLBACK(uint16_t pin){
    if(pin != GPIO_PIN_0) return;

    uint32_t now = HAL_GetTick();
    uint32_t period = now - lastPulseTime;
    lastPulseTime = now;

    if (period > 0){
        lastPeriodMs = period;
    }
}

