#include "stm32f4xx_hal.h"
#include "debug/debug.h"
#include "gas/throttle.h"
#include "gas/adc.h"
#include "brake/brake.h"
#include "brake/adc.h"
#include "brake/encoder.h"
#include "telemetry/telemetry.h"
#include "speedSensor/speedSensor.h"

void SysTick_Handler(void) { HAL_IncTick(); }

#define THROTTLE_PERIOD_MS  20
#define BRAKE_PERIOD_MS     100
#define TELEMETRY_PERIOD_MS 50

int main(void) {
    HAL_Init();
    debugInit();
    throttleInit();
    brakeInit();
    speedSensorInit();
    brakeHome();

    uint32_t lastThrottle = 0, lastBrake = 0, lastTelemetry = 0;

    while (1) {
        uint32_t now = HAL_GetTick();

        if (now - lastThrottle >= THROTTLE_PERIOD_MS) {
            lastThrottle = now;
            runThrottle();
        }
        if (now - lastBrake >= BRAKE_PERIOD_MS) {
            lastBrake = now;
            runBrake();
        }
        if (now - lastTelemetry >= TELEMETRY_PERIOD_MS) {
            lastTelemetry = now;
            telemetrySend(now, speedGetKph(), adc1Read(), brakeAdcRead(), throttleGetAngle(), encoderGetPosition(), brakeGetLastTarget(), brakeGetLastOutput());
        }
    }
}
