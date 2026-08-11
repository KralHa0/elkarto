#include "throttle.h"
#include "../debug/debug.h"
#include "../potFilter.h"
#include <stdio.h>

static uint16_t currentAngle = 0;

// Pedal pot ADC range, measured from testfiles/01.08.2026-14.10.txt
static const int32_t POT_MIN = 3400;
static const int32_t POT_MAX = 3500;

// Servo angle at pedal released / fully pressed -- set to the throttle
// body's actual mechanical travel, not the servo's full 0-180 range.
static const uint16_t ANGLE_MIN = 120;
static const uint16_t ANGLE_MAX = 210;

static DeadbandFilter potFilter = { .deadband = 8 };

void throttleInit(){
    gasAdcInit();
    pcaInit();

    char buf[32];
    snprintf(buf, sizeof(buf), "PCA: %s\r\n",
    pcaIsReady() ? "OK" : "FAIL");
    debugPrint(buf);
}

void runThrottle(){
    uint32_t potVal = deadbandFilterApply(&potFilter, adc1Read());

    int32_t mapped = mapRange((int32_t)potVal, POT_MIN, POT_MAX, 0, 4095);
    if (mapped < 0) mapped = 0;
    if (mapped > 4095) mapped = 4095;

    int32_t curve = (mapped * mapped) / 4095; // 0..4095, quadratic ease-in
    currentAngle = (uint16_t)(ANGLE_MIN + (curve * (ANGLE_MAX - ANGLE_MIN)) / 4095);
    setServoAngle(currentAngle);
}

uint16_t throttleGetAngle(void) {
    return currentAngle;
}
