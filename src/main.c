#include "stm32f4xx_hal.h"
#include "debug/debug.h"
#include "gas/throttle.h"
#include <stdio.h>

void SysTick_Handler(void) { HAL_IncTick(); }

int main(void) {
    HAL_Init();
    debugInit();
    throttleInit();
    
    while (1) {
       runThrottle();
    }
}
