#include "brake.h"
#include "../telemetry/telemetry.h"

static PID brakePid;
static const float KP = 0.5f;   // bench-test value, not tuned
static const float KI = 0;      // leave at 0 for now -- avoid integral windup during first test
static const float KD = 0;      // leave at 0 for now
// Must match BRAKE_PERIOD_MS in main.c's scheduler -- runBrake() is called on that cadence.
static const float DT_SECONDS = 0.1f;

// Pedal pot ADC range (measure actual pedal travel, may not be full 0-4095)
static const int32_t POT_MIN = 0;
static const int32_t POT_MAX = 4095;

// Encoder count endpoints (placeholders until automatic homing/calibration exists)
static const int32_t ENCODER_RELEASED = 0;
static const int32_t ENCODER_APPLIED = 3000;

static int32_t mapRange(int32_t x, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax) {
    return outMin + (x - inMin) * (outMax - outMin) / (inMax - inMin);
}

static float lastTarget = 0;
static float lastOutput = 0;
static uint32_t lastPotRaw = 0;

float brakeGetLastTarget(void) { return lastTarget; }
float brakeGetLastOutput(void) { return lastOutput; }
uint32_t brakeGetLastPotRaw(void) { return lastPotRaw; }

void brakeInit() {
    brakeAdcInit();
    l298nInit();
    encoderInit();
    pidInit(&brakePid, KP, KI, KD, -999.0f, 999.0f);
}

void runBrake() {
    float newKp, newKi, newKd;
    if (telemetryGetGainUpdate(&newKp, &newKi, &newKd)) {
        brakePid.kp = newKp;
        brakePid.ki = newKi;
        brakePid.kd = newKd;
    }

    uint32_t breakPotRaw = brakeAdcRead();
    lastPotRaw = breakPotRaw;
    int32_t targetPosition = mapRange(breakPotRaw, POT_MIN, POT_MAX, ENCODER_RELEASED, ENCODER_APPLIED);
    if (targetPosition < ENCODER_RELEASED) targetPosition = ENCODER_RELEASED;
    if (targetPosition > ENCODER_APPLIED) targetPosition = ENCODER_APPLIED;

    int32_t currentPosition = encoderGetPosition();

    float output = pidUpdate(&brakePid, targetPosition, currentPosition, DT_SECONDS);
    lastTarget = (float)targetPosition;
    lastOutput = output;

    if (output > 0) {
        l298nForward((uint16_t)output);
    } else if (output < 0) {
        l298nReverse((uint16_t)(-output));
    } else {
        l298nCoast();
    }
}
