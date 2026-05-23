# Project Protocol Reference

## Source Locations

- `Hardware/servo/pid.h`: `PID_TypeDef` stores `Kp`, `Ki`, `Kd`, `Target`, `Current`, `Error`, `Last_Error`, `Integral`, and clamps.
- `Hardware/servo/pid.c`: `PID_Calc`, `PID_Reset`, `PID_ParseCommand`, indexed `PID_ParseIndexedCommand`, and optional PID VOFA list helpers.
- `Hardware/servo/servo.c`: `Servo_Task` runs cascade position/speed PID; `Servo_SendTelemetry` sends the application-selected PID list through a UART write callback.
- `Core/Src/main.c`: USART1 setup, interrupt receive command dispatcher, and 20 Hz telemetry scheduling from the main loop.

## Serial Settings

- Interface: USART1
- Baud: `115200`
- Frame: `8N1`
- RX buffer: `64` bytes
- Command terminator: `\n` or `\r`

## FireWater Telemetry

Current target protocol:

```text
pos_target,pos_current,pos_error,pos_output,pos_Kp,pos_Ki,pos_Kd,speed_target,speed_current,speed_error,pwm_output,speed_Kp,speed_Ki,speed_Kd\n
```

All channels are decimal text. `pos_output` is the position-loop output and should match `speed_target`. `speed_target` and `speed_current` use encoder pulses per 10 ms `Servo_Task` tick. `pwm_output` is clamped to the PWM ARR range. Telemetry is sent from the main loop at 20 Hz so USART RX interrupts can still accept tuning commands.

## Accepted Commands

```text
T=<int>\n
P=<float>\n
I=<float>\n
D=<float>\n
1P=<float>\n
1I=<float>\n
1D=<float>\n
2P=<float>\n
2I=<float>\n
2D=<float>\n
PP=<float>\n
PI=<float>\n
PD=<float>\n
VP=<float>\n
VI=<float>\n
VD=<float>\n
M=<int>\n
```

`T=` calls `Servo_SetTargetPos`. Indexed commands tune the application-selected PID list: currently `1P=`, `1I=`, and `1D=` tune the position PID, while `2P=`, `2I=`, and `2D=` tune the speed PID. The index is defined only by the order in which `servo.c` adds PID pointers to the VOFA list, so fixed PIDs can be left out of the tunable list in the application layer. Legacy `P=`, `I=`, and `D=` tune the position loop. `PP=`, `PI=`, and `PD=` also tune the position loop. `VP=`, `VI=`, and `VD=` tune the speed loop. `M=` calls `Servo_SetMaxTargetSpeed`.

## Safety Notes

The position loop output is clamped to `[-servo->max_target_speed, servo->max_target_speed]` and becomes the speed-loop target. The speed loop output is clamped to `[-servo->max_speed, servo->max_speed]`, where `max_speed` comes from the PWM timer ARR. Make small tuning changes and watch for output saturation, growing oscillation, and unexpected movement.

The servo task suppresses output inside a small settle window: position error within 8 encoder pulses and speed within 1 pulse per 10 ms tick resets the speed PID and drives PWM to zero. `Servo_SetTargetPos` also resets both PID dynamic states so old speed-loop integral does not carry into the next step command.
