#pragma once
#include "adc.h"
#include "l298n.h"
#include "encoder.h"
#include "pid.h"

void brakeInit();
void runBrake();

// Temporary debug getters -- last computed target position, PID output, and raw pot reading
float brakeGetLastTarget(void);
float brakeGetLastOutput(void);
uint32_t brakeGetLastPotRaw(void);
