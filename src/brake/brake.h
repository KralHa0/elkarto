#pragma once
#include "adc.h"
#include "l298n.h"
#include "encoder.h"
#include "pid.h"

void brakeInit();
void brakeHome(void);
void runBrake();
