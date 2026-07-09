#include "throttle.h"
#include "../debug/debug.h"
#include <stdio.h>

static uint16_t currentAngle = 0;

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
    currentAngle = (uint16_t)((val * val) / 4095 * 180 / 4095);
    setServoAngle(currentAngle);
}

uint16_t throttleGetAngle(void) {
    return currentAngle;
}
