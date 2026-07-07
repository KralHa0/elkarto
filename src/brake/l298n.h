#pragma once
#include "stm32f4xx_hal.h"

// L298N brake motor driver
// PA0 -> ENA (PWM, TIM2 CH1)
// PA2 -> IN2
// PA4 -> IN1

void l298nInit();
void brakeApply(uint16_t duty);
void brakeRelease(uint16_t duty);
void brakeHold();
