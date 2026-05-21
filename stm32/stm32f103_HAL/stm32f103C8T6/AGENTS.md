# Repository Guidelines

## Project Structure & Module Organization
This is STM32F103C8T6 firmware generated from STM32CubeMX and built with CMake. `Core/Inc` and `Core/Src` hold the application entry point, HAL setup, MSP code, and interrupt handlers; keep CubeMX-survivable edits inside `/* USER CODE BEGIN */` blocks. Custom drivers live in `Hardware/OLED`, `Hardware/CAN`, and `Hardware/servo`; register new sources/includes in `CMakeLists.txt`. `Drivers/` is HAL/CMSIS vendor code. `cmake/stm32cubemx` is generated glue. `stm32f103C8T6.ioc`, `startup_stm32f103xb.s`, and `STM32F103XX_FLASH.ld` define peripheral setup, startup, and memory layout.

## Build, Test, and Development Commands
Use `.vscode/tasks.json` as the compile, build, and flash reference. The default STM32 flash task depends on `CMake: build`, then runs STM32CubeProgrammer CLI with `-c port=SWD mode=UR -w ${command:cmake.launchTargetPath} -v -rst`. In VS Code, select a CMake preset, build, then run the default STM32 task. CLI equivalents are useful for diagnostics:

- `cmake --preset Debug`: configure `build/Debug`.
- `cmake --build --preset Debug`: compile and generate `.elf`, `.hex`, and `.bin`.
- `cmake --build --preset Debug --target clean`: remove Debug build outputs.

## Coding Style & Naming Conventions
The firmware is C11 plus ARM assembly. Match local style and `.clang-format` where practical: LLVM base, Allman braces, 120-column limit. Prefer module-prefixed APIs such as `Servo_Init` or `CAN_Send_Msg`, typedefs ending in `_t`, and uppercase macros/include guards. Keep generated-section changes minimal.

## Testing Guidelines
There is no automated test framework. At minimum, run `CMake: build`; when hardware behavior changes, also run the STM32 flash task from `.vscode/tasks.json`. Document target-hardware validation for affected peripherals such as OLED, CAN, UART/VOFA, timers, DMA, or servo control.

## Commit & Pull Request Guidelines
Recent history uses short Conventional Commit-style subjects, for example `feat: add OLED display support` and `chore: optimize paths`. Use a lowercase type (`feat`, `fix`, `chore`, `refactor`) followed by a concise summary. Pull requests should describe firmware behavior changes, list build/flash results, note hardware or toolchain assumptions, and link related issues. Include serial, OLED, or CAN logs only when useful for validation.

## Security & Configuration Tips
Do not commit `build/`, personal toolchain paths, credentials, or captured device data. After regenerating from CubeMX, review `Core/`, `cmake/stm32cubemx/`, and `stm32f103C8T6.ioc` before mixing generated updates with hand-written driver edits.

## Agent-Specific Modification Workflow
For each requested change, first analyze requirements and affected files, then propose a concise plan. Wait for explicit user confirmation before editing. For code changes, try to use subagents for clear, non-overlapping scopes; otherwise keep the change local and state why.
