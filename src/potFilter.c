#include "potFilter.h"

int32_t mapRange(int32_t x, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax) {
    return outMin + (x - inMin) * (outMax - outMin) / (inMax - inMin);
}

uint32_t deadbandFilterApply(DeadbandFilter *filter, uint32_t newValue) {
    int32_t diff = (int32_t)newValue - (int32_t)filter->lastValue;
    if (diff < 0) diff = -diff;
    if (diff >= filter->deadband) filter->lastValue = newValue;
    return filter->lastValue;
}
