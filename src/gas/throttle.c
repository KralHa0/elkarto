#include "throttle.h"
#include "../debug/debug.h"
#include <stdio.h>

static uint16_t currentAngle = 0;

// Pedal pot ADC range, measured from testfiles/01.08.2026-14.10.txt
// POT_MIN set to 2320, not the raw sweep min of 1072 (a one-off noise spike)
// or the released-plateau average of ~1976 (still too low a delta vs. the
// brake pot's range for pedals with similar physical travel).
// POT_MAX padded 10 counts in from the raw max so the pedal doesn't need to
// reach the absolute physical stop to hit full range.
static const int32_t POT_MIN = 3400;
static const int32_t POT_MAX = 3500;

// Servo angle at pedal released / fully pressed. Set these to match the
// throttle body's actual mechanical travel instead of assuming the servo's
// full 0-180 theoretical range -- e.g. to avoid over-rotating past the
// throttle body's stop, or to set a nonzero idle angle.
static const uint16_t ANGLE_MIN = 120;
static const uint16_t ANGLE_MAX = 210;

// Deadband: ignore raw ADC changes smaller than this many counts, to stop
// servo jitter from ADC/pot noise that's "close but not identical" read to read.
static const int32_t ADC_DEADBAND = 8;
static uint32_t lastVal = 0;

static int32_t mapRange(int32_t x, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax) {
    return outMin + (x - inMin) * (outMax - outMin) / (inMax - inMin);
}

void throttleInit(){
    gasAdcInit();
    pcaInit();

    char buf[32];
    snprintf(buf, sizeof(buf), "PCA: %s\r\n",
    pcaIsReady() ? "OK" : "FAIL");
    debugPrint(buf);
}

void runThrottle(){
    uint32_t val = adc1Read();
    int32_t diff = (int32_t)val - (int32_t)lastVal;
    if (diff < 0) diff = -diff;
    if (diff >= ADC_DEADBAND) {
        lastVal = val;
    }

    int32_t mapped = mapRange((int32_t)lastVal, POT_MIN, POT_MAX, 0, 4095);
    if (mapped < 0) mapped = 0;
    if (mapped > 4095) mapped = 4095;

    int32_t curve = (mapped * mapped) / 4095; // 0..4095, quadratic ease-in
    currentAngle = (uint16_t)(ANGLE_MIN + (curve * (ANGLE_MAX - ANGLE_MIN)) / 4095);
    setServoAngle(currentAngle);
}

uint16_t throttleGetAngle(void) {
    return currentAngle;
}
