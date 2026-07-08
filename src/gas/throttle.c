#include "throttle.h"
#include "../debug/debug.h"
#include <stdio.h>

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
    uint16_t angle = (uint16_t)((val * val) / 4095 * 180 / 4095);
    setServoAngle(angle);

    HAL_Delay(20);

    //char buf[40];
    //(buf, sizeof(buf), "Val: %lu  Angle: %u\r\n", val, angle);
    //debugPrint(buf);
}
