#include "brake.h"

void brakeInit() {
    brakeAdcInit();
    l298nInit();
    encoderInit();
}

void runBrake() {
    // TODO: read brakeAdcRead(), PID against encoderGetPosition(), drive via brakeApply/brakeRelease/brakeHold
}
