#pragma once
#include "stm32f4xx_hal.h"

// L298N H-bridge motor driver
// PA0 -> ENA (PWM, TIM2 CH1)
// PA2 -> IN2
// PA4 -> IN1

void l298nInit();
void l298nForward(uint16_t duty);
void l298nReverse(uint16_t duty);
void l298nCoast();
