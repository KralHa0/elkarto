#include "telemetry.h"
#include "../debug/debug.h"
#include <stdio.h>

void telemetrySend(uint32_t timestamp, float speed, uint32_t gas, uint32_t brake, uint16_t servoPos, int32_t motorPos, int32_t target, float output) {
    char buf[112];
    snprintf(buf, sizeof(buf), "%lu,%.2f,%lu,%lu,%u,%ld,%ld,%.2f\n",
             (unsigned long)timestamp, speed, (unsigned long)gas, (unsigned long)brake,
             servoPos, (long)motorPos, (long)target, output);
    debugPrint(buf);
}

uint8_t telemetryGetGainUpdate(float *kp, float *ki, float *kd) {
    char line[64];
    if (!debugReadLine(line, sizeof(line))) return 0;
    if (line[0] != 'K' || line[1] != ',') return 0;
    return sscanf(line, "K,%f,%f,%f", kp, ki, kd) == 3;
}
