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

// Encoder count endpoints -- default fallbacks, overwritten by brakeHome() at boot
static int32_t ENCODER_RELEASED = 0;
static int32_t ENCODER_APPLIED = 3000;

// Homing tuning
static const uint16_t HOMING_DUTY = 200;        // gentle speed, not full power
static const uint32_t HOMING_POLL_MS = 20;       // how often to sample the encoder while homing
static const int32_t HOMING_STALL_COUNTS = 2;    // movement below this between polls counts as "not moving"
static const uint32_t HOMING_STALL_MS = 300;     // how long it must stay still to call it a stall
static const uint32_t HOMING_TIMEOUT_MS = 5000;  // safety cap so a broken encoder can't drive forever

static int32_t mapRange(int32_t x, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax) {
    return outMin + (x - inMin) * (outMax - outMin) / (inMax - inMin);
}

void brakeInit() {
    brakeAdcInit();
    l298nInit();
    encoderInit();
    pidInit(&brakePid, KP, KI, KD, -999.0f, 999.0f);
}

// Drives in one direction (via driveFn) until the encoder stops advancing for
// HOMING_STALL_MS -- i.e. it hit a mechanical limit -- then coasts and returns
// the position it stalled at. Bails out after HOMING_TIMEOUT_MS regardless, in
// case the encoder isn't reporting motion (broken wiring, etc).
static int32_t homeToStall(void (*driveFn)(uint16_t)) {
    driveFn(HOMING_DUTY);

    int32_t lastPos = encoderGetPosition();
    uint32_t stallStart = HAL_GetTick();
    uint32_t homingStart = stallStart;

    while (1) {
        HAL_Delay(HOMING_POLL_MS);
        int32_t pos = encoderGetPosition();
        int32_t moved = pos - lastPos;
        if (moved < 0) moved = -moved;

        if (moved > HOMING_STALL_COUNTS) {
            lastPos = pos;
            stallStart = HAL_GetTick();
        } else if (HAL_GetTick() - stallStart >= HOMING_STALL_MS) {
            break;
        }

        if (HAL_GetTick() - homingStart >= HOMING_TIMEOUT_MS) {
            break;
        }
    }

    l298nCoast();
    return encoderGetPosition();
}

// Automatic calibration: finds the released and fully-applied encoder endpoints
// by driving to each mechanical limit, rather than relying on guessed constants.
// NOTE: the brake will be fully applied with real force at the end of this --
// make sure the wheel is free (off the ground) before calling it.
void brakeHome(void) {
    // Release first -- spring-assisted, so this settles at the natural rest position.
    homeToStall(l298nReverse);
    encoderReset();
    ENCODER_RELEASED = 0;

    // Apply -- winds in against the spring until the caliper/cable can't move further.
    ENCODER_APPLIED = homeToStall(l298nForward);
}

void runBrake() {
    float newKp, newKi, newKd;
    if (telemetryGetGainUpdate(&newKp, &newKi, &newKd)) {
        brakePid.kp = newKp;
        brakePid.ki = newKi;
        brakePid.kd = newKd;
    }

    uint32_t breakPotRaw = brakeAdcRead();
    int32_t targetPosition = mapRange(breakPotRaw, POT_MIN, POT_MAX, ENCODER_RELEASED, ENCODER_APPLIED);
    if (targetPosition < ENCODER_RELEASED) targetPosition = ENCODER_RELEASED;
    if (targetPosition > ENCODER_APPLIED) targetPosition = ENCODER_APPLIED;

    int32_t currentPosition = encoderGetPosition();

    float output = pidUpdate(&brakePid, targetPosition, currentPosition, DT_SECONDS);

    if (output > 0) {
        l298nForward((uint16_t)output);
    } else if (output < 0) {
        l298nReverse((uint16_t)(-output));
    } else {
        l298nCoast();
    }
}
