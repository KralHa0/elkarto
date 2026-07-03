# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

DIY drive-by-wire go-kart firmware for a **WeAct STM32F405RGT6** (Black Pill). Written in C/C++ using PlatformIO + bare metal register access (stm32cube framework for CMSIS headers). Flashed via ST-Link V2 over SWD on the side header (PA13=SWDIO, PA14=SWCLK — the middle header was cut off).

**ST-Link wiring:** 5V → STM32 5V, GND → GND, SWDIO → PA13, SWCLK → PA14. Must use 5V (not 3.3V) — the onboard AMS1117 needs 5V in to regulate; feeding 3.3V into the 3.3V pin back-feeds the LDO and the board won't power up.

**HAL note:** Always define `SysTick_Handler` calling `HAL_IncTick()` before `main()` — it must override the weak default for `HAL_Delay` to work.

## Commands

```bash
# Build
pio run

# Flash via ST-Link
pio run --target upload

# Build + flash
pio run --target upload -e stm_blinker

# Open serial monitor (if needed)
pio device monitor

# Clean
pio run --target clean
```

To run a single environment: `pio run -e <env_name>`

## Project Structure

```
platformio.ini        # PlatformIO config; src_dir points at testers/ for test builds
testers/stmTest.c     # Active blinker tester (PC13 LED, HAL, no RTOS)
src/                  # Main firmware lives here (not yet created)
```

When the main firmware is developed, it will go in `src/` and `platformio.ini` should be updated to add a separate `[env:main]` that points at `src/`.

## Hardware Pin Map

| Pin | Function |
|-----|----------|
| PA0 | L298N ENA — brake motor PWM (TIM2 CH1) |
| PA1 | L298N IN1 — brake motor direction |
| PA2 | L298N IN2 — brake motor direction |
| PA3 | ADC1 CH3 — gas pedal potentiometer |
| PA4 | ADC1 CH4 — brake pedal potentiometer |
| PA5 | ADC1 CH5 — battery voltage divider |
| PA6 | TIM3 CH1 — encoder channel A |
| PA7 | TIM3 CH2 — encoder channel B |
| PB0 | Hall effect speed sensor (A3144, 10k pull-up to 3.3V) |
| PB6 | I2C1 SCL — PCA9685 + SSD1306 |
| PB7 | I2C1 SDA — PCA9685 + SSD1306 |
| PB2 | Onboard LED (active low) |

## Key Subsystems

**Throttle:** Gas pedal pot → PID → PCA9685 (I2C 0x40) PWM0 → MG996R servo. Physical return spring closes throttle on power loss.

**Brake:** Brake pedal pot → PID → L298N → JGB37-520 DC gear motor. TIM3 in hardware encoder mode (11 PPR, quadrature) for position feedback. Physical return spring applies brake on power loss.
- Apply: IN1=HIGH, IN2=LOW, ENA=PWM
- Release: IN1=LOW, IN2=HIGH, ENA=PWM
- Hold: IN1=LOW, IN2=LOW, ENA=0

**Speed:** A3144 hall sensor on PB0 + neodymium magnet on wheel spoke → timer interrupt → speed calc.

**Battery voltage:**
```c
v_adc     = (adc_raw / 4095.0f) * 3.3f;
v_battery = v_adc * (10000.0f + 3300.0f) / 3300.0f;  // R1=10k, R2=3.3k
```

**Display:** SSD1306 0.96" OLED at I2C 0x3C — speed, mode, throttle/brake position, battery.

**E-stop:** Latching mushroom button (NC contact) cuts 12V relay coil independently of software. STM32 GPIO can also cut the relay via software kill (1N4007 flyback diode on coil).

## Power Rail

`Battery+ → 30A fuse → E-stop NC → Relay → 12V bus → L298N & buck converter (12V→5V) → STM32 5V pin → onboard AMS1117 → 3.3V bus → PCA9685 VCC, OLED, encoder, pots`
