#pragma once
#include "stm32f4xx_hal.h"

// Brake pedal potentiometer, PC2 (ADC1 CH12)

void brakeAdcInit();
uint32_t brakeAdcRead();
