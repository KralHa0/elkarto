#pragma once
#include "stm32f4xx_hal.h"

void pcaInit();
void setServoAngle(uint16_t angle);
uint8_t pcaIsReady(void);