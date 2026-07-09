#pragma once
#include "adc.h"
#include "pca9685.h"

void throttleInit();
void runThrottle();
uint16_t throttleGetAngle(void);