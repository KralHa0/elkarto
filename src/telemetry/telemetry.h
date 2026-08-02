#pragma once
#include "stm32f4xx_hal.h"

void telemetrySend(uint32_t timestamp, float speed, uint32_t gas, uint32_t brake, uint16_t servoPos, int32_t motorPos, int32_t target, float output);
uint8_t telemetryGetGainUpdate(float *kp, float *ki, float *kd);
