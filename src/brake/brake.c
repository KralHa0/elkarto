#include "brake.h"
#include "../telemetry/telemetry.h"

static PID brakePid;
static const float KP = 8.0f;   // bench-test value, not tuned
// KI=0 (P-only) can never fully close the gap against the spring's constant
// pull -- output shrinks as error shrinks, so the motor settles wherever the
// diminishing P force balances the spring, short of the real target. KI
// accumulates the persistent error over time and keeps pushing output up
// until it's strong enough to close the gap and hold there.
static const float KI = 0.4f;
static const float KD = 0.5f;
// Must match BRAKE_PERIOD_MS in main.c's scheduler -- runBrake() is called on that cadence.
static const float DT_SECONDS = 0.1f;

// Pedal pot ADC range, measured from testfiles/01.08.2026-14.10.txt
// (raw sweep was 3334-3489; padded 10 counts in from each end so the pedal
// doesn't need to reach the absolute physical stop to hit full range)
static const int32_t POT_MIN = 460;
static const int32_t POT_MAX = 660;

// Encoder count endpoints -- default fallbacks, overwritten by brakeHome() at boot
static int32_t ENCODER_RELEASED = 0;
static int32_t ENCODER_APPLIED = 3000;

// Homing tuning -- apply needs real torque to wind the cable in against the
// spring; release is spring-assisted, so gentle is enough and safer.
static const uint16_t HOMING_APPLY_DUTY = 900;
static const uint16_t HOMING_RELEASE_DUTY = 200;

// Time-based homing: drive for a fixed duration known to be enough to reach
// the mechanical limit, instead of trying to detect a stall via the encoder
// (unreliable -- gearbox backlash/judder right at the real limit kept
// resetting stall detection, so it just oscillated there instead of homing).
// Measure how long full travel actually takes on the bench and set these
// with a bit of safety margin.
static const uint32_t HOMING_APPLY_MS = 3000;
static const uint32_t HOMING_RELEASE_MS = 2000;

static int32_t mapRange(int32_t x, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax) {
    return outMin + (x - inMin) * (outMax - outMin) / (inMax - inMin);
}

// Deadband: ignore raw ADC changes smaller than this many counts. With
// POT_MIN/POT_MAX this narrow, a few counts of ordinary ADC noise swing
// target by a large fraction of the whole travel -- the PID then chases a
// jittering setpoint instead of holding a firm one, which feels like weak,
// intermittently-loose braking even though homing (open-loop, no target)
// pulls fine. Mirrors the same fix already applied to the throttle pot.
static const int32_t ADC_DEADBAND = 8;
static uint32_t lastPotVal = 0;

static int32_t lastTarget = 0;
static float lastOutput = 0;

// Caps how much the commanded PWM can change per control tick, independent of
// what the raw PID math produces. Protects the cable/spindle from a violent
// full-speed snap whenever the position error is suddenly huge (e.g. right after
// homing, or a driver stomping the pedal) -- ramps to full output over a few
// ticks instead of slamming there in one.
static const float MAX_OUTPUT_STEP = 400.0f; // per BRAKE_PERIOD_MS tick

int32_t brakeGetLastTarget(void) { return lastTarget; }
float brakeGetLastOutput(void) { return lastOutput; }

void brakeInit() {
    brakeAdcInit();
    l298nInit();
    encoderInit();
    pidInit(&brakePid, KP, KI, KD, -999.0f, 999.0f);
}

// Drives in one direction (via driveFn) at a fixed duty for a fixed duration,
// then coasts and returns wherever the encoder ended up.
static int32_t homeForDuration(void (*driveFn)(uint16_t), uint16_t duty, uint32_t durationMs) {
    driveFn(duty);
    HAL_Delay(durationMs);
    l298nCoast();
    return encoderGetPosition();
}

// Automatic calibration: finds the released and fully-applied encoder endpoints
// by driving to each mechanical limit, rather than relying on guessed constants.
// NOTE: the brake will be fully applied with real force at the end of this --
// make sure the wheel is free (off the ground) before calling it.
void brakeHome(void) {
    // Release first -- spring-assisted, so this settles at the natural rest position.
    homeForDuration(l298nReverse, HOMING_RELEASE_DUTY, HOMING_RELEASE_MS);
    encoderReset();
    ENCODER_RELEASED = 0;

    // Apply -- winds in against the spring until the caliper/cable reaches its limit.
    ENCODER_APPLIED = homeForDuration(l298nForward, HOMING_APPLY_DUTY, HOMING_APPLY_MS) + 50;

    // Return to released before handing off to the PID loop. Without this, homing
    // ends with the brake fully applied (max cable tension) while an untouched
    // pedal implies "released" -- runBrake()'s very first tick would then see the
    // largest possible position error, slam the output to max, and violently
    // overshoot against the spring (this is what was popping the cable nipple out
    // right after homing).
    homeForDuration(l298nReverse, HOMING_RELEASE_DUTY, HOMING_RELEASE_MS);
}

void runBrake() {
    float newKp, newKi, newKd;
    if (telemetryGetGainUpdate(&newKp, &newKi, &newKd)) {
        brakePid.kp = newKp;
        brakePid.ki = newKi;
        brakePid.kd = newKd;
    }

    uint32_t breakPotRaw = brakeAdcRead();
    int32_t diff = (int32_t)breakPotRaw - (int32_t)lastPotVal;
    if (diff < 0) diff = -diff;
    if (diff >= ADC_DEADBAND) lastPotVal = breakPotRaw;

    int32_t targetPosition = mapRange((int32_t)lastPotVal, POT_MIN, POT_MAX, ENCODER_RELEASED, ENCODER_APPLIED);
    // Clamp to [min, max] regardless of which of RELEASED/APPLIED is numerically
    // larger -- if the encoder counts down during apply, ENCODER_APPLIED ends up
    // negative (less than ENCODER_RELEASED), and clamping in a fixed RELEASED-then-
    // APPLIED order breaks: the first clamp pushes back up past 0, the second then
    // pulls it right back down to ENCODER_APPLIED, pinning the target constant.
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
