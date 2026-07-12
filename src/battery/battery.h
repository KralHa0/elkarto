#pragma once
#include "stm32f4xx_hal.h"

// Battery voltage divider, PB1 (ADC3 CH9). R1=10k, R2=3.3k.

void batteryInit(void);
float batteryReadVoltage(void);
