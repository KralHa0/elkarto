#include "brake.h"

void brakeInit() {
    brakeAdcInit();
    l298nInit();
    encoderInit();
}

void runBrake() {
    // TODO: read brakeAdcRead(), PID against encoderGetPosition(), drive via l298nForward/l298nReverse/l298nCoast
}
