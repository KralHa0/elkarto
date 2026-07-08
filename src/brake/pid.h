#pragma once
#include "stm32f4xx_hal.h"

typedef struct
{
    float kp,  ki, kd;
    float integral;
    float prevError;
    float outMin, outMax;
} PID;

void pidInit(PID *pid, float kp, float ki, float kd, float outMin, float outMax);
float pidUpdata(PID *pid, float setpoint, float measurment, float dt);
