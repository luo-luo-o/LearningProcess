#!/usr/bin/env python3
"""Capture VOFA+ FireWater PID telemetry and optionally send safe commands."""

from __future__ import annotations

import argparse
import statistics
import sys
import time
from dataclasses import dataclass


FORWARD_PORT = "COM25"


@dataclass
class Sample:
    target: float
    current: float
    error: float | None = None
    output: float | None = None
    pos_output: float | None = None
    kp: float | None = None
    ki: float | None = None
    kd: float | None = None
    target_speed: float | None = None
    current_speed: float | None = None
    speed_error: float | None = None
    speed_kp: float | None = None
    speed_ki: float | None = None
    speed_kd: float | None = None


def parse_line(line: str) -> Sample | None:
    parts = [p.strip() for p in line.strip().split(",") if p.strip()]
    if len(parts) not in (2, 7, 13, 14):
        return None
    try:
        values = [float(p) for p in parts]
    except ValueError:
        return None
    if len(values) == 2:
        target, current = values
        return Sample(target=target, current=current, error=target - current)
    if len(values) == 13:
        (
            target,
            current,
            error,
            target_speed,
            current_speed,
            speed_error,
            pwm_output,
            pos_kp,
            pos_ki,
            pos_kd,
            speed_kp,
            speed_ki,
            speed_kd,
        ) = values
        return Sample(
            target=target,
            current=current,
            error=error,
            output=pwm_output,
            kp=pos_kp,
            ki=pos_ki,
            kd=pos_kd,
            target_speed=target_speed,
            current_speed=current_speed,
            speed_error=speed_error,
            speed_kp=speed_kp,
            speed_ki=speed_ki,
            speed_kd=speed_kd,
        )
    if len(values) == 14:
        (
            target,
            current,
            error,
            pos_output,
            pos_kp,
            pos_ki,
            pos_kd,
            target_speed,
            current_speed,
            speed_error,
            pwm_output,
            speed_kp,
            speed_ki,
            speed_kd,
        ) = values
        return Sample(
            target=target,
            current=current,
            error=error,
            output=pwm_output,
            pos_output=pos_output,
            kp=pos_kp,
            ki=pos_ki,
            kd=pos_kd,
            target_speed=target_speed,
            current_speed=current_speed,
            speed_error=speed_error,
            speed_kp=speed_kp,
            speed_ki=speed_ki,
            speed_kd=speed_kd,
        )
    return Sample(*values)


def summarize(samples: list[Sample]) -> None:
    if not samples:
        print("No valid FireWater samples captured.")
        return

    errors = [s.error if s.error is not None else s.target - s.current for s in samples]
    currents = [s.current for s in samples]
    outputs = [s.output for s in samples if s.output is not None]
    target_speeds = [s.target_speed for s in samples if s.target_speed is not None]
    current_speeds = [s.current_speed for s in samples if s.current_speed is not None]
    speed_errors = [s.speed_error for s in samples if s.speed_error is not None]
    target = samples[-1].target
    final_error = errors[-1]
    peak_abs_error = max(abs(e) for e in errors)

    if target >= samples[0].current:
        overshoot = max(0.0, max(currents) - target)
    else:
        overshoot = max(0.0, target - min(currents))

    print(f"samples={len(samples)}")
    print(f"target={target:.3f} current={samples[-1].current:.3f} final_error={final_error:.3f}")
    print(f"peak_abs_error={peak_abs_error:.3f} overshoot={overshoot:.3f}")
    if outputs:
        print(f"output_min={min(outputs):.3f} output_max={max(outputs):.3f}")
    if target_speeds and current_speeds and speed_errors:
        print(
            "speed "
            f"target={target_speeds[-1]:.3f} current={current_speeds[-1]:.3f} "
            f"error={speed_errors[-1]:.3f}"
        )
        print(f"speed_error_abs_peak={max(abs(e) for e in speed_errors):.3f}")
    if len(errors) >= 5:
        tail = errors[-min(20, len(errors)) :]
        print(f"tail_error_mean={statistics.fmean(tail):.3f}")


def write_command(serial_port, command: str) -> None:
    serial_port.write((command.rstrip() + "\n").encode("ascii"))
    print(f"sent: {command}")


def open_forward_port(serial_module, baud: int):
    try:
        return serial_module.Serial(FORWARD_PORT, baud, timeout=0, write_timeout=0)
    except (serial_module.SerialException, OSError, ValueError) as exc:
        print(f"{FORWARD_PORT} forwarding unavailable: {exc}")
        return None


def close_serial(serial_module, serial_port) -> None:
    try:
        serial_port.close()
    except (serial_module.SerialException, OSError):
        pass


def forward_raw_line(serial_module, forward_port, raw: bytes) -> bool:
    if not raw:
        return True

    try:
        written = forward_port.write(raw)
    except (serial_module.SerialException, OSError, TimeoutError) as exc:
        print(f"{FORWARD_PORT} forwarding stopped: {exc}")
        close_serial(serial_module, forward_port)
        return False

    if written != len(raw):
        print(f"{FORWARD_PORT} forwarding stopped: short write")
        close_serial(serial_module, forward_port)
        return False

    return True


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="Serial port, for example COM3")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--duration", type=float, default=10.0)
    parser.add_argument("--write", action="store_true", help="Allow sending target/PID commands")
    parser.add_argument("--target", type=int, help="Send T=<target> before capture; requires --write")
    parser.add_argument("--pid", nargs=3, type=float, metavar=("KP", "KI", "KD"), help="Send indexed PID #1 P/I/D before capture; requires --write")
    parser.add_argument("--speed-pid", nargs=3, type=float, metavar=("KP", "KI", "KD"), help="Send indexed PID #2 P/I/D before capture; requires --write")
    parser.add_argument("--max-target-speed", type=int, help="Send M=<speed> before capture; requires --write")
    args = parser.parse_args()

    if (args.target is not None or args.pid is not None or args.speed_pid is not None or args.max_target_speed is not None) and not args.write:
        parser.error("--target, --pid, --speed-pid, and --max-target-speed require --write")

    try:
        import serial
    except ImportError:
        print("pyserial is required: python -m pip install pyserial", file=sys.stderr)
        return 2

    samples: list[Sample] = []
    with serial.Serial(args.port, args.baud, timeout=0.2) as ser:
        forward_port = open_forward_port(serial, args.baud)
        time.sleep(0.2)
        ser.reset_input_buffer()
        try:
            if args.pid:
                kp, ki, kd = args.pid
                write_command(ser, f"1P={kp:.6f}")
                write_command(ser, f"1I={ki:.6f}")
                write_command(ser, f"1D={kd:.6f}")
            if args.speed_pid:
                kp, ki, kd = args.speed_pid
                write_command(ser, f"2P={kp:.6f}")
                write_command(ser, f"2I={ki:.6f}")
                write_command(ser, f"2D={kd:.6f}")
            if args.max_target_speed is not None:
                write_command(ser, f"M={args.max_target_speed}")
            if args.target is not None:
                write_command(ser, f"T={args.target}")

            deadline = time.monotonic() + args.duration
            while time.monotonic() < deadline:
                raw = ser.readline()
                if forward_port is not None and not forward_raw_line(serial, forward_port, raw):
                    forward_port = None

                sample = parse_line(raw.decode("ascii", errors="ignore"))
                if sample is not None:
                    samples.append(sample)
        finally:
            if forward_port is not None:
                close_serial(serial, forward_port)

    summarize(samples)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
