#!/usr/bin/env python3
"""Connects to the go-kart's ESP32 telemetry WebSocket, prints incoming data,
and lets you push updated brake PID gains to the STM32 without reflashing.

Usage:
    pip install -r requirements.txt
    python monitor.py                      # connects to ws://192.168.4.1:81
    python monitor.py --host 192.168.4.1 --port 81

While running, type:
    kp <kp> <ki> <kd>
to push new gains to the brake PID (sent to the STM32 as "K,<kp>,<ki>,<kd>").
"""
import argparse
import asyncio
import sys
from datetime import datetime

import websockets

FIELDS = ["timestamp_ms", "speed", "gas", "brake", "servo_pos", "motor_pos", "battery"]


def parse_telemetry(line: str):
    parts = line.split(",")
    if len(parts) != len(FIELDS):
        return None
    try:
        values = [float(p) for p in parts]
    except ValueError:
        return None
    return dict(zip(FIELDS, values))


async def receiver(ws):
    async for message in ws:
        if message.startswith("I,"):
            _, uptime_ms, free_heap, clients = message.split(",")
            print(f"[esp32] uptime={uptime_ms}ms heap={free_heap}B clients={clients}")
            continue

        telemetry = parse_telemetry(message)
        if telemetry is None:
            print(f"[?] {message}")
            continue

        ts = datetime.now().strftime("%H:%M:%S")
        print(
            f"[{ts}] speed={telemetry['speed']:.1f} gas={telemetry['gas']:.0f} "
            f"brake={telemetry['brake']:.0f} servo={telemetry['servo_pos']:.0f} "
            f"motor_pos={telemetry['motor_pos']:.0f} battery={telemetry['battery']:.2f}V"
        )


async def sender(ws):
    loop = asyncio.get_event_loop()
    while True:
        line = await loop.run_in_executor(None, sys.stdin.readline)
        line = line.strip()
        if not line:
            continue
        parts = line.split()
        if parts[0].lower() == "kp" and len(parts) == 4:
            _, kp, ki, kd = parts
            await ws.send(f"K,{kp},{ki},{kd}")
            print(f"-> sent gains kp={kp} ki={ki} kd={kd}")
        else:
            print("unrecognized command. usage: kp <kp> <ki> <kd>")


async def main(host: str, port: int):
    uri = f"ws://{host}:{port}"
    print(f"connecting to {uri} ...")
    async with websockets.connect(uri) as ws:
        print("connected. type: kp <kp> <ki> <kd>  to push new PID gains")
        await asyncio.gather(receiver(ws), sender(ws))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="192.168.4.1", help="ESP32 AP gateway IP")
    parser.add_argument("--port", type=int, default=81)
    args = parser.parse_args()

    try:
        asyncio.run(main(args.host, args.port))
    except KeyboardInterrupt:
        pass
