#include "brake.h"
#include "../telemetry/telemetry.h"
#include "../potFilter.h"

static PID brakePid;
static const float KP = 8.0f;
static const float KI = 0.4f; // needed to fully close the gap against the spring's constant pull -- P alone settles short
static const float KD = 0.5f;
static const float DT_SECONDS = 0.1f; // must match BRAKE_PERIOD_MS in main.c's scheduler

// Pedal pot ADC range, measured from testfiles/01.08.2026-14.10.txt
static const int32_t POT_MIN = 460;
static const int32_t POT_MAX = 660;

// Encoder count endpoints -- default fallbacks, overwritten by brakeHome() at boot
static int32_t ENCODER_RELEASED = 0;
static int32_t ENCODER_APPLIED = 3000;

// Homing duty -- apply needs real torque against the spring; release is spring-assisted.
static const uint16_t HOMING_APPLY_DUTY = 900;
static const uint16_t HOMING_RELEASE_DUTY = 200;

// Time-based homing (not stall-detection -- gearbox backlash near the limit made
// stall detection oscillate instead of concluding "stalled"). Tune durations on
// the bench to comfortably exceed full travel time.
static const uint32_t HOMING_APPLY_MS = 3000;
static const uint32_t HOMING_RELEASE_MS = 2000;

static DeadbandFilter potFilter = { .deadband = 8 };

static int32_t lastTarget = 0;
static float lastOutput = 0;

// Caps commanded PWM change per tick, independent of raw PID output. Protects
// the cable/spindle from a violent full-speed snap on a sudden large error
// (e.g. right after homing, or a driver stomping the pedal).
static const float MAX_OUTPUT_STEP = 400.0f; // per BRAKE_PERIOD_MS tick

int32_t brakeGetLastTarget(void) { return lastTarget; }
float brakeGetLastOutput(void) { return lastOutput; }

void brakeInit() {
    brakeAdcInit();
    l298nInit();
    encoderInit();
    pidInit(&brakePid, KP, KI, KD, -999.0f, 999.0f);
}

// Drives in one direction at a fixed duty for a fixed duration, then coasts
// and returns wherever the encoder ended up.
static int32_t homeForDuration(void (*driveFn)(uint16_t), uint16_t duty, uint32_t durationMs) {
    driveFn(duty);
    HAL_Delay(durationMs);
    l298nCoast();
    return encoderGetPosition();
}

// Finds the released and fully-applied encoder endpoints by driving to each
// mechanical limit. Wheel must be free (off the ground) before calling.
void brakeHome(void) {
    homeForDuration(l298nReverse, HOMING_RELEASE_DUTY, HOMING_RELEASE_MS);
    encoderReset();
    ENCODER_RELEASED = 0;

    ENCODER_APPLIED = homeForDuration(l298nForward, HOMING_APPLY_DUTY, HOMING_APPLY_MS) + 50;

    // Return to released before handing off to the PID loop -- otherwise the
    // loop's first tick sees the largest possible error (current=applied,
    // target=released-pedal) and slams to max output against the spring.
    homeForDuration(l298nReverse, HOMING_RELEASE_DUTY, HOMING_RELEASE_MS);
}

void runBrake() {
    float newKp, newKi, newKd;
    if (telemetryGetGainUpdate(&newKp, &newKi, &newKd)) {
        brakePid.kp = newKp;
        brakePid.ki = newKi;
        brakePid.kd = newKd;
    }

    uint32_t potVal = deadbandFilterApply(&potFilter, brakeAdcRead());
    int32_t targetPosition = mapRange((int32_t)potVal, POT_MIN, POT_MAX, ENCODER_RELEASED, ENCODER_APPLIED);

    // Clamp order-independently -- ENCODER_APPLIED can end up below
    // ENCODER_RELEASED if the encoder counts down during apply.
    int32_t lo = ENCODER_RELEASED < ENCODER_APPLIED ? ENCODER_RELEASED : ENCODER_APPLIED;
    int32_t hi = ENCODER_RELEASED < ENCODER_APPLIED ? ENCODER_APPLIED : ENCODER_RELEASED;
    if (targetPosition < lo) targetPosition = lo;
    if (targetPosition > hi) targetPosition = hi;

    int32_t currentPosition = encoderGetPosition();

    float rawOutput = pidUpdate(&brakePid, targetPosition, currentPosition, DT_SECONDS);
    float delta = rawOutput - lastOutput;
    if (delta > MAX_OUTPUT_STEP) delta = MAX_OUTPUT_STEP;
    if (delta < -MAX_OUTPUT_STEP) delta = -MAX_OUTPUT_STEP;
    float output = lastOutput + delta;

    lastTarget = targetPosition;
    lastOutput = output;

    if (output > 0) {
        l298nForward((uint16_t)output);
    } else if (output < 0) {
        l298nReverse((uint16_t)(-output));
    } else {
        l298nCoast();
    }
}
