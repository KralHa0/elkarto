#pragma once
#include "stm32f4xx_hal.h"

// Battery voltage divider, PC1 (ADC3 CH11). R1=10k, R2=3.3k.
// NOTE: was PB1/ADC channel 9, but that channel doesn't reach ADC3 on this
// package (only ADC1/ADC2 correctly map channel 9 to PB1), and sharing
// ADC1/ADC2 with gas/brake caused unreliable readings on whichever pot
// shared the peripheral (verified: brake's readings became wildly noisy
// while shared, gas -- on its own dedicated ADC1 -- stayed clean). This pin
// gives battery its own exclusive ADC, matching gas/brake's pattern.
// Requires physically moving the divider's output wire from PB1 to PC1.

void batteryInit(void);
float batteryReadVoltage(void);
