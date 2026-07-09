#include "stm32f4xx_hal.h"
#include "brake/l298n.h"

// Standalone bench test for the L298N/brake motor wiring, bypassing pedal, PID,
// and encoder entirely. Build/flash with: pio run -e motor_debug --target upload
// Cycles: forward -> coast -> reverse -> coast, fixed duty, forever.

void SysTick_Handler(void) { HAL_IncTick(); }

int main(void) {
    HAL_Init();
    l298nInit();

    while (1) {
        l298nForward(500); // ~50% duty
        HAL_Delay(3000);

        l298nCoast();
        HAL_Delay(1000);

        l298nReverse(500); // ~50% duty
        HAL_Delay(3000);

        l298nCoast();
        HAL_Delay(1000);
    }
}
