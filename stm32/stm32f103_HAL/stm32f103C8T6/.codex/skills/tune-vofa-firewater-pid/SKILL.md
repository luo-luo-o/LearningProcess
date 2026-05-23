---
name: tune-vofa-firewater-pid
description: Analyze and tune this STM32 project's PID loop using VOFA+ FireWater serial telemetry. Use when Codex needs to read or interpret FireWater CSV data, suggest safe Kp/Ki/Kd changes, send P=/I=/D=/T= commands, update the project PID telemetry protocol, or work with the bundled serial PID tuning script.
---

# Tune VOFA FireWater PID

## Workflow

First confirm the serial port, baud rate, mechanical safety limits, and whether the user wants read-only advice or command transmission. Default to read-only analysis. Do not send new PID values unless the user explicitly authorizes it and the command range is reasonable for the mechanism.

Use `references/project-protocol.md` for the current firmware protocol and source locations. The firmware reports VOFA+ FireWater CSV lines and accepts newline-terminated commands.

## Data Interpretation

Expected telemetry is:

```text
pos_target,pos_current,pos_error,pos_output,pos_Kp,pos_Ki,pos_Kd,speed_target,speed_current,speed_error,pwm_output,speed_Kp,speed_Ki,speed_Kd
```

Legacy firmware may report:

```text
target,current
target,current,error,output,Kp,Ki,Kd
pos_target,pos_current,pos_error,speed_target,speed_current,speed_error,pwm_output,pos_Kp,pos_Ki,pos_Kd,speed_Kp,speed_Ki,speed_Kd
```

Evaluate response quality from target steps: rise behavior, overshoot, sustained oscillation, steady-state error, and output saturation. Prefer small changes:

- Increase `Kp` gradually when response is slow and not oscillating.
- Decrease `Kp` when overshoot or oscillation grows.
- Add small `Ki` only for persistent steady-state error; reduce it if the integral response winds up or overshoots.
- Add or increase `Kd` when overshoot needs damping; reduce it if noise causes jitter.

## Script Usage

Use `scripts/serial_firewater_pid.py` for repeatable serial capture and basic metrics.

Read-only capture:

```powershell
python .\.codex\skills\tune-vofa-firewater-pid\scripts\serial_firewater_pid.py --port COM3 --duration 10
```

Authorized target and PID update:

```powershell
python .\.codex\skills\tune-vofa-firewater-pid\scripts\serial_firewater_pid.py --port COM3 --write --target 1560 --pid 0.40 0.00 0.00 --duration 10
```

If `pyserial` is missing, ask the user for permission to install it or use another serial monitor to provide captured lines.
