<!-- FirmGen source overview v1 -->
# vehicleparametersdisplay

Source-based overview for the generated stm32 firmware. This page records the current source structure and statically identified interfaces. Building the firmware or generating this report does not establish runtime correctness.

Application entry source: `Core/Src/main.c`.

## Modules and runtime entry points

- `Core/Inc` — Core/Inc (module, 3 source/header files).
- `Core/Src` — Core/Src (module, 4 source/header files).
- `Core/Startup/iar` — Core/Startup/iar (module, 1 source/header files).
- `app` — app (application, 2 source/header files).
- `bsp` — bsp (board, 2 source/header files).
- `configs` — configs (configuration, 5 source/header files).
- `drivers/dht22` — drivers/dht22 (driver, 2 source/header files).
- `drivers/mpu6050` — drivers/mpu6050 (driver, 2 source/header files).
- `drivers/sh1106` — drivers/sh1106 (driver, 2 source/header files).
- `interfaces` — interfaces (module, 1 source/header files).
- `modules` — modules (module, 2 source/header files).
- `osal` — osal (abstraction, 1 source/header files).
- `platforms/stm32` — platforms/stm32 (platform, 2 source/header files).
- `services` — services (module, 8 source/header files).
- `telemetry` — telemetry (module, 3 source/header files).
- `utils/helpers` — utils/helpers (utility, 2 source/header files).
- `utils/logger` — utils/logger (utility, 2 source/header files).
- Runtime entry `app_start` — app_start in `firmware/app/app.c`.

## Pins and interfaces found in source

- `PA5` — PA5 (gpio_output) at `firmware/platforms/stm32/platform_io.c`.
- `PB4` — PB4 (gpio_output) at `firmware/platforms/stm32/platform_io.c`.
- `PB8` — PB8 (gpio_input) at `firmware/platforms/stm32/platform_io.c`.
- `PB9` — PB9 (gpio_input) at `firmware/platforms/stm32/platform_io.c`.
- uart_device — `firmware/platforms/stm32/platform_io.c`.

This is a static inventory, not verified wiring. Check the Firmware Topology panel and board wiring for unresolved connections.

## Build, flash, and documentation

Open this project in FirmGen and select its configured board/toolchain. Use **Build** to compile, **Flash** to program the connected board, and **Monitor** to inspect UART output. Debug/Production telemetry switching rebuilds and reflashes the firmware.

The API report is in `api/html/index.html` beside this overview. Review application behavior on hardware separately. Expand this overview with intended behavior, operating instructions, and validation evidence as these become available.

## Analyzed source inventory

- `CMakeLists.txt`
- `Core/Inc/main.h`
- `Core/Inc/stm32g0xx_hal_conf.h`
- `Core/Inc/stm32g0xx_it.h`
- `Core/Src/main.c`
- `Core/Src/stm32g0xx_hal_msp.c`
- `Core/Src/stm32g0xx_it.c`
- `Core/Src/system_stm32g0xx.c`
- `Core/Startup/iar/firmgen_iar_syscalls.c`
- `firmware/app/app.c`
- `firmware/app/app.h`
- `firmware/bsp/board.c`
- `firmware/bsp/board.h`
- `firmware/configs/app_config.h`
- `firmware/configs/build_config.h`
- `firmware/configs/config.h`
- `firmware/configs/feature_config.h`
- `firmware/configs/platform_config.h`
- `firmware/drivers/dht22/dht22.c`
- `firmware/drivers/dht22/dht22.h`
- `firmware/drivers/mpu6050/mpu6050.c`
- `firmware/drivers/mpu6050/mpu6050.h`
- `firmware/drivers/sh1106/sh1106.c`
- `firmware/drivers/sh1106/sh1106.h`
- `firmware/interfaces/platform_io.h`
- `firmware/modules/ride_events.c`
- `firmware/modules/ride_events.h`
- `firmware/osal/osal.h`
- `firmware/platforms/stm32/osal_baremetal.c`
- `firmware/platforms/stm32/platform_io.c`
- `firmware/services/display.c`
- `firmware/services/display.h`
- `firmware/services/display_font.h`
- `firmware/services/logo.h`
- `firmware/services/sensors.c`
- `firmware/services/sensors.h`
- `firmware/services/telemetry.c`
- `firmware/services/telemetry.h`
- `firmware/telemetry/fg_telemetry_profile.h`
- `firmware/telemetry/firmgen_telemetry.c`
- `firmware/telemetry/firmgen_telemetry.h`
- `firmware/utils/helpers/helpers.c`
- `firmware/utils/helpers/helpers.h`
- `firmware/utils/logger/logger.c`
- `firmware/utils/logger/logger.h`
