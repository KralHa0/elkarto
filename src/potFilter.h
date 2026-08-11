#pragma once
#include <stdint.h>

int32_t mapRange(int32_t x, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax);

// Ignores raw ADC changes smaller than `deadband` counts, to stop jitter from
// pot/ADC noise that reads "close but not identical" tick to tick.
typedef struct {
    uint32_t lastValue;
    int32_t deadband;
} DeadbandFilter;

uint32_t deadbandFilterApply(DeadbandFilter *filter, uint32_t newValue);
