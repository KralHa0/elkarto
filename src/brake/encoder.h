#pragma once
#include "stm32f4xx_hal.h"

// Brake motor quadrature encoder, TIM3 hardware encoder mode
// PA6 -> CH A
// PA7 -> CH B

void encoderInit();
int32_t encoderGetPosition();
