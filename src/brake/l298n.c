#include "l298n.h"

// TODO: init PA0 (TIM2 CH1 PWM), PA2, PA4 as GPIO outputs
void l298nInit() {
}

// TODO: IN1=HIGH, IN2=LOW, ENA=duty
void brakeApply(uint16_t duty) {
}

// TODO: IN1=LOW, IN2=HIGH, ENA=duty
void brakeRelease(uint16_t duty) {
}

// TODO: IN1=LOW, IN2=LOW, ENA=0
void brakeHold() {
}
