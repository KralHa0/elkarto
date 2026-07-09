#include "pid.h"

void pidInit(PID *pid, float kp, float ki, float kd, float outMin, float outMax){
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0;
    pid->prevError = 0;
    pid->outMin = outMin;
    pid->outMax = outMax;
}

float pidUpdate(PID *pid, float setpoint, float measurment, float dt){
    float error = setpoint - measurment;
    pid->integral += error * dt;
    float derivative = (error - pid->prevError) / dt;
    pid->prevError = error;

    float output = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;

    if (output > pid->outMax){output = pid->outMax; pid->integral -= error *dt;}
    if (output < pid->outMin){output = pid->outMin; pid->integral -= error *dt;}

    return output;
}