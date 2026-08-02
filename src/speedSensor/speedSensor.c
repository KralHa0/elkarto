#include "speedSensor.h"

// Measure your actual wheel and update this -- placeholder until then.
static const float WHEEL_CIRCUMFERENCE_M = 1.0f;
static const uint32_t SPEED_TIMEOUT_MS = 1500; // no pulse in this long -> report stopped
static const uint32_t MIN_PULSE_PERIOD_MS = 5;  // ignore implausibly fast pulses (noise/bounce)

static volatile uint32_t lastPulseTime;
static volatile uint32_t lastPeriodMs;

void speedSensorInit(){
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio= {0};
    gpio.Pin = GPIO_PIN_0;
    // Both edges -- more robust to whichever transition the sensor actually
    // produces (open-drain sinks are documented active-low/falling, but this
    // catches it either way while diagnosing). Doubles pulses/revolution to 2;
    // accounted for in speedGetKph() below.
    gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void EXTI0_IRQHandler(void){
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void HAL_GPIO_EXTI_Callback(uint16_t pin){
    if(pin != GPIO_PIN_0) return;

    uint32_t now = HAL_GetTick();
    uint32_t period = now - lastPulseTime;

    if (period >= MIN_PULSE_PERIOD_MS){
        lastPeriodMs = period;
        lastPulseTime = now;
    }
}

float speedGetKph(void) {
    if (HAL_GetTick() - lastPulseTime > SPEED_TIMEOUT_MS) {
        return 0.0f;
    }

    // 2 pulses/revolution now (both edges trigger), so halve the raw pulse rate.
    float pulsesPerSec = 1000.0f / (float)lastPeriodMs;
    float revsPerSec = pulsesPerSec / 2.0f;
    float mps = revsPerSec * WHEEL_CIRCUMFERENCE_M;
    return mps * 3.6f;
}

