#pragma once
#include "stm32f4xx_hal.h"

void debugInit(void);
void debugPrint(const char *msg);
void debugBlink(uint32_t delay);
uint8_t debugReadLine(char *buf, uint8_t maxLen);
