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
pio run --target upload -e main

# Open serial monitor (if needed)
pio device monitor

# Clean
pio run --target clean
```

To run a single environment: `pio run -e <env_name>`

## Project Structure

```
platformio.ini          # STM32 PlatformIO project (src_dir = src)
src/                     # Main firmware
  main.c                 # Non-blocking scheduler: throttle (20ms), brake (100ms), telemetry (50ms)
  gas/                    # Throttle: pedal ADC, PCA9685 servo driver, throttle.c ties them together
  brake/                  # Brake: pedal ADC (ADC2), L298N driver, TIM3 encoder, pid.c, brake.c ties them together
  debug/                  # USART1 (PA9/PA10) driver -- debugPrint (TX) and debugReadLine (interrupt-driven RX)
  telemetry/              # CSV telemetry send + "K,<kp>,<ki>,<kd>" gain-update parsing, built on debug/'s UART
  debug_motor_main.c       # Standalone L298N/motor bench-test firmware (bypasses pedal/PID/encoder entirely)
testers/stmTest.c        # Older standalone blinker test, not part of any current build env

esp32/                   # Independent PlatformIO project (ESP32-C3 SuperMini, Arduino framework)
  src/main.cpp            # WiFi AP "gokart" + WebSocket server (port 81) + UART bridge to the STM32
  laptop/monitor.py        # Python WebSocket client: prints telemetry, sends live PID gain updates
```

Two STM32 build environments in the root `platformio.ini`:
- `[env:main]` — the real firmware (`pio run -e main`)
- `[env:motor_debug]` — isolated L298N/motor bench test, only pulls in `debug_motor_main.c` + `brake/l298n.c` (`pio run -e motor_debug`)

The `esp32/` folder is a separate PlatformIO project with its own `platformio.ini` (different toolchain/board) — build/flash it from inside `esp32/`, not from the repo root.

## Hardware Pin Map

| Pin | Function |
|-----|----------|
| PA0 | L298N ENA — brake motor PWM (TIM2 CH1) |
| PA2 | L298N IN2 — brake motor direction |
| PA4 | L298N IN1 — brake motor direction |
| PA6 | TIM3 CH1 — encoder channel A |
| PA7 | TIM3 CH2 — encoder channel B |
| PA9 | USART1 TX — ESP32 RX (telemetry + live PID gain updates) |
| PA10 | USART1 RX — ESP32 TX (telemetry + live PID gain updates) |
| PA13 | ST-Link SWDIO |
| PA14 | ST-Link SWCLK |
| PB0 | Hall effect speed sensor (A3144, 10k pull-up to 3.3V) — not yet implemented in firmware |
| PB1 | ADC1 CH9 — battery voltage divider — not yet implemented in firmware |
| PB2 | Debug/onboard LED (active low) |
| PB6 | I2C1 SCL — PCA9685 + SSD1306 |
| PB7 | I2C1 SDA — PCA9685 + SSD1306 |
| PC0 | ADC1 CH10 — gas pedal potentiometer |
| PC2 | ADC2 CH12 — brake pedal potentiometer |

## Key Subsystems

**Throttle:** Gas pedal pot → PID → PCA9685 (I2C 0x40) PWM0 → MG996R servo. Physical return spring closes throttle on power loss.

**Brake:** Brake pedal pot → PID → L298N → JGB37-520 DC gear motor, driving a spindle that winds/unwinds the brake cable. Bicycle brake calipers are normally-open — their return spring pulls the brake released by default, so the motor must actively work against the spring to apply braking force, and must keep working against it to hold any partial/full-brake position (gearbox is not self-locking, so "hold" is continuous small corrective pulses via the position PID, not a static state). TIM3 in hardware encoder mode (11 PPR, quadrature) for position feedback.
- Apply: IN1=HIGH, IN2=LOW, ENA=PWM (l298nForward)
- Release: IN1=LOW, IN2=HIGH, ENA=PWM (l298nReverse)
- Coast: IN1=LOW, IN2=LOW, ENA=0 (l298nCoast) — no active resistance; spring pulls toward released

Since motor power loss means the spring releases the brake (fail-open, not fail-safe), a separate manual emergency brake lever (mechanical, independent of the STM32/motor system) is being added as the actual power-loss fail-safe.

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
