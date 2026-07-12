#pragma once
#include "stm32f4xx_hal.h"

// L298N H-bridge motor driver -- Channel B (OUT3/OUT4)
// PA0 -> ENB (PWM, TIM2 CH1)
// PA2 -> IN4
// PA4 -> IN3

void l298nInit();
void l298nForward(uint16_t duty);
void l298nReverse(uint16_t duty);
void l298nCoast();
