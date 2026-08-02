#pragma once
#include "adc.h"
#include "l298n.h"
#include "encoder.h"
#include "pid.h"

void brakeInit();
void brakeHome(void);
void runBrake();

// Live tuning visibility -- last computed target position and PID output.
int32_t brakeGetLastTarget(void);
float brakeGetLastOutput(void);
