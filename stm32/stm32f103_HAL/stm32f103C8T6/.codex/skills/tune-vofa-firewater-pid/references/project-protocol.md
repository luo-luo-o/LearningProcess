# Project Protocol Reference

## Source Locations

- `Hardware/servo/pid.h`: `PID_TypeDef` stores `Kp`, `Ki`, `Kd`, `Target`, `Current`, `Error`, `Last_Error`, `Integral`, clamps, and UART binding.
- `Hardware/servo/pid.c`: `PID_Calc`, `PID_SendToVofa`, and `PID_ParseCommand`.
- `Hardware/servo/servo.c`: `Servo_Task` runs the position loop and calls `PID_SendToVofa`.
- `Core/Src/main.c`: USART1 setup and interrupt receive command dispatcher.

## Serial Settings

- Interface: USART1
- Baud: `115200`
- Frame: `8N1`
- RX buffer: `64` bytes
- Command terminator: `\n` or `\r`

## FireWater Telemetry

Current target protocol:

```text
target,current,error,output,Kp,Ki,Kd\n
```

All channels are decimal text. VOFA+ FireWater should be configured as CSV/plain text with channels in this exact order.

## Accepted Commands

```text
T=<int>\n
P=<float>\n
I=<float>\n
D=<float>\n
```

`T=` calls `Servo_SetTargetPos`. `P=`, `I=`, and `D=` are parsed by `PID_ParseCommand`.

## Safety Notes

PID output is clamped to `[-servo->max_speed, servo->max_speed]`, where `max_speed` comes from the PWM timer ARR. `Integral_Max` is configured through `Servo_ConfigPID`; current startup configuration uses `500.0f`. Make small tuning changes and watch for output saturation, growing oscillation, and unexpected movement.
