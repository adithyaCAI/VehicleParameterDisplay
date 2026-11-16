# Requirement traceability

Generated 2026-11-16T14:06:27.726901200+00:00
Specification frozen 2026-11-16T13:39:41.943Z

33 of 33 requirements have code recorded against them — 0 with tests passing, 0 with tests written but not passing, 33 implemented with no test at all. 0 have nothing recorded.

**Host unit tests.** imported from Windows 10 (AMD64) (Ceedling 1.1.6 (Unity + CMock)), run at 2026-11-16T13:46:51.104166+00:00 — 189 passed, 14 failed, 9 ignored. NOT GREEN: requirement verdicts: 11 PASSED, 3 FAILED, 11 PARTIAL, 6 BLOCKED, 2 NOT_RUN, 7 firmware source file(s) compiled into it

**Unit-test verdicts by requirement.** 11 PASSED, 11 PARTIAL, 3 FAILED, 6 BLOCKED, 2 NOT_RUN. Only PASSED means every criterion was asserted and passed with nothing else owed; PARTIAL rows say what they still need.

Everything below is derived from what the agent recorded with `requirement_trace` and from the frozen specification. A blank cell means nothing was recorded, which is not the same as nothing being needed.

## Board & Firmware Configuration

gate: not decided · tasks: 3/3 done · recorded in: 8eeeb85d-cd97-4429-8f7b-89539bdd7e8a

| Req | Statement | Verified by | Acceptance criteria | Files | Symbols | Tests | Status |
|---|---|---|---|---|---|---|---|
| `HW-001` | The firmware shall target the STM32-NUCLEO-G070RB development board. | hardware / instrument | The firmware image executes on a NUCLEO-G070RB board. (expected: The board is a NUCLEO-G070RB and the program counter advances between samples.) | CMakeLists.txt<br>STM32G070RBTX_FLASH.ld<br>firmware/bsp/board.h | — | — | implemented, untested · unit tests: NOT_RUN - still needs hardware / instrument |
| `HW-003` | The firmware project shall provide a configurable LOGO_DISPLAY_MS macro in config.h with a default value of 2000. | inspection | The default definition of LOGO_DISPLAY_MS in config.h is 2000, and the logo display timing uses that macro. (expected: The default is 2000 ms and the logo display timing uses LOGO_DISPLAY_MS rather than a separate fixed duration.) | firmware/configs/app_config.h<br>firmware/configs/config.h | — | — | implemented, untested · unit tests: PARTIAL - still needs inspection |
| `HW-004` | The firmware project shall define the climate and ride-event thresholds, sudden window, and event display time as configurable parameters in config.h. | inspection | config.h exposes configurable high-temperature and high-humidity thresholds for both demo and in-vehicle settings. (expected: Demo thresholds are 32 degC and 75%; in-vehicle thresholds are 45 degC and 80%.)<br>config.h exposes configurable Z, X, +Y, and -Y ride-event thresholds for both demo and in-vehicle settings. (expected: Demo thresholds are Z = 0.30 g, X = 0.25 g, +Y = +0.25 g, and -Y = -0.25 g; each in-vehicle threshold is in the range 0.4–0.5 g, with the -Y threshold retaining its negative direction.)<br>config.h exposes configurable sudden-window and event-display-time parameters with the specified durations. (expected: The sudden window is 100 ms and the event display time is 2 s.) | firmware/configs/app_config.h<br>firmware/configs/config.h | — | — | implemented, untested · unit tests: PARTIAL - still needs inspection |
| `FR-019` | The firmware shall use 32 degC as the configured high-temperature threshold. | inspection | The configured high-temperature threshold used by the firmware is 32 degC. (expected: 32 degC.) | firmware/configs/app_config.h<br>firmware/configs/config.h | — | — | implemented, untested · unit tests: PARTIAL - still needs inspection |
| `FR-020` | The firmware shall use 75% as the configured high-humidity threshold. | inspection | The configured high-humidity threshold used by the firmware is 75%. (expected: 75%.) | firmware/configs/app_config.h<br>firmware/configs/config.h | — | — | implemented, untested · unit tests: PARTIAL - still needs inspection |

Observed on the board:
- `HW-001` — Fresh build and ST-Link flash succeeded. Serial capture observation:299a1ef0-1eaf-4e1a-a995-d7c80372f892 contained only a NUL byte, no readable startup marker. SWD debug attach reported target running, no HardFault breakpoint hit, PC=0x080008F0, VTOR=0x08000000 and matches_elf=true. This establishes a flashed image matching the ELF, but NOT the required PC advancement between two samples or physical NUCLEO board identity. HW-001 hardware acceptance remains pending. No sensor/display functionality exists in this configuration-only category.

Notes recorded with these traces:
- `HW-001` — STM32G070xx Cortex-M0+ vendor startup and NUCLEO-G070RB board configuration build successfully. Probe PC-advancement check is still owed; no flash performed.
- `HW-003` — LOGO_DISPLAY_MS aliases APP_LOGO_DISPLAY_MS, default 2000U; editable authoritative value in app_config.h. Timing consumer belongs to category 3 and is not yet implemented; full inspection acceptance remains pending.
- `HW-004`, `FR-019`, `FR-020` — Demo defaults 32 C, 75 percent, Z .30g, X/+Y .25g, -Y -.25g; vehicle defaults 45 C, 80 percent and signed .45g axis thresholds; sudden window 100ms and event hold 2000ms. Config declarations inspected; behavior consumers arrive in subsequent categories.

## Sensor Acquisition & I2C Bus

gate: deferred · tasks: 5/5 done · recorded in: 3310ad84-e4ea-4ce6-9fef-0502d5c2a5ec, 8eeeb85d-cd97-4429-8f7b-89539bdd7e8a

| Req | Statement | Verified by | Acceptance criteria | Files | Symbols | Tests | Status |
|---|---|---|---|---|---|---|---|
| `IF-001` | The firmware shall acquire temperature and humidity from a DHT22 sensor connected through PB4. | hardware / instrument | A DHT22 acquisition uses PB4 and returns temperature and humidity matching the sensor's response. (expected: The acquisition signal is present on PB4, and both returned readings match the DHT22 response.) | firmware/configs/app_config.h<br>firmware/drivers/dht22/dht22.c<br>firmware/platforms/stm32/platform_io.c<br>firmware/services/sensors.c | dht22_decode, platform_io_dht_frame, sensors_process | — | implemented, untested · unit tests: BLOCKED - still needs hardware / instrument |
| `IF-002` | The firmware shall interface with an MPU6050 3-axis accelerometer and gyroscope over I2C on PB8 (SCL) and PB9 (SDA), sharing the bus with the OLED. | inspection | The I2C configuration assigns PB8 to SCL and PB9 to SDA for the bus used by both the MPU6050 and OLED. (expected: PB8 is configured as SCL, PB9 as SDA, and both devices are assigned to that same I2C bus.) | firmware/configs/app_config.h<br>firmware/interfaces/platform_io.h<br>firmware/platforms/stm32/platform_io.c | platform_io_init, platform_io_read, platform_io_transmit, platform_io_write | — | implemented, untested · unit tests: BLOCKED - still needs inspection |
| `IF-007` | The firmware shall coordinate access to the shared I2C bus so transactions to the MPU6050 and OLED do not overlap or corrupt one another. | host test | Concurrent MPU6050 and OLED requests produce no overlapping I2C transactions, and both operations complete with their own expected results. (expected: The maximum number of active transactions is 1; both operations complete and each receives its own expected response.) | firmware/configs/app_config.h<br>firmware/interfaces/platform_io.h<br>firmware/platforms/stm32/platform_io.c | platform_io_init, platform_io_read, platform_io_transmit, platform_io_write | — | implemented, untested · unit tests: BLOCKED |
| `FR-002` | While displaying the logo, the firmware shall initialize the DHT22 and perform MPU6050 zeroing in the background without extending the splash duration. | host test | DHT22 initialization and MPU6050 zeroing complete while the splash is active, and the splash ends at its configured deadline. (expected: Both sensor operations complete after splash start and before splash completion; splash completion occurs at the configured duration, without an extension for sensor work.) | firmware/app/app.c<br>firmware/services/display.c<br>firmware/services/sensors.c | app_start, display_process, sensors_init, sensors_process | — | implemented, untested · unit tests: PASSED |
| `FR-005` | The firmware shall read temperature in degrees Celsius and humidity in percent from the DHT22 every 2 seconds. | host test | The DHT22 read scheduler initiates consecutive reads 2000 ms apart. (expected: Each consecutive initiation time differs by 2000 ms.)<br>A successful DHT22 read publishes its temperature and humidity in the corresponding Celsius and percent fields. (expected: The temperature field is 23.5 °C and the humidity field is 48%.) | firmware/configs/app_config.h<br>firmware/drivers/dht22/dht22.c<br>firmware/platforms/stm32/platform_io.c<br>firmware/services/sensors.c | dht22_decode, platform_io_dht_frame, sensors_process | — | implemented, untested · unit tests: BLOCKED |
| `FR-007` | The firmware shall read the MPU6050 X, Y, and Z acceleration at 100 Hz using the +/-4 g range and convert the readings to g units. | host test | The MPU6050 configuration selects the ±4 g accelerometer range. (expected: The full-scale selection is ±4 g (FS_SEL = 1).)<br>The MPU6050 sampling logic requests acceleration samples at 100 Hz. (expected: Successive requests are separated by 10 ms (100 samples per second).)<br>Each acceleration axis is converted from raw MPU6050 counts to g using the configured ±4 g scale. (expected: The axis supplied with 8192 counts is 1.0 g, and the other two axes are 0.0 g, subject to the implementation's numeric representation precision.) | firmware/app/app.c<br>firmware/drivers/mpu6050/mpu6050.c<br>firmware/services/sensors.c<br>firmware/services/sensors.h | mpu6050_init, mpu6050_read, sensors_init, sensors_process, sensors_snapshot | — | implemented, untested · unit tests: PASSED |
| `FR-008` | At power-up, the firmware shall zero the MPU6050 readings after the board has been flat and still for 1 second, producing X = 0 g, Y = 0 g, and Z = 1 g. | host test | After one second of flat-and-still calibration samples, the calibrated acceleration is X = 0 g, Y = 0 g, and Z = 1 g. (expected: X = 0 g, Y = 0 g, and Z = 1 g.)<br>The first sensor screen becomes the active screen when the configured splash completes, and it presents the calibrated X, Y, and Z values as valid. (expected: The active screen is the first sensor screen, and it presents valid values X = 0 g, Y = 0 g, Z = 1 g.) | firmware/app/app.c<br>firmware/drivers/mpu6050/mpu6050.c<br>firmware/services/sensors.c<br>firmware/services/sensors.h | mpu6050_init, mpu6050_read, sensors_init, sensors_process, sensors_snapshot | — | implemented, untested · unit tests: PARTIAL |
| `FR-009` | The firmware shall map X acceleration to lateral cornering, Y acceleration to longitudinal acceleration/braking, and Z acceleration to vertical bumps. | host test | The displayed acceleration values are assigned to lateral X, longitudinal Y, and vertical Z axes. (expected: Lateral uses X, longitudinal uses Y, and vertical uses Z.)<br>The event-processing inputs use X for lateral cornering, Y for longitudinal acceleration/braking, and Z for vertical bumps. (expected: Lateral event input uses X, longitudinal event input uses Y, and vertical event input uses Z.) | firmware/app/app.c<br>firmware/drivers/mpu6050/mpu6050.c<br>firmware/services/sensors.c<br>firmware/services/sensors.h | mpu6050_init, mpu6050_read, sensors_init, sensors_process, sensors_snapshot | — | implemented, untested · unit tests: PASSED |
| `DIAG-001` | The firmware shall detect failed DHT22 or MPU6050 acquisitions and shall not use failed acquisition results as valid displayed measurements or inputs to event detection. | host test | A failed DHT22 or MPU6050 acquisition is marked invalid and is not presented as a valid displayed measurement. (expected: For each failed acquisition, the result is invalid and is not presented as a valid measurement.)<br>A failed DHT22 or MPU6050 acquisition does not cause an event to be triggered from that failed result. (expected: Failed-result values are excluded from event-detection inputs, and neither injected failure triggers an event from those values.) | firmware/app/app.c<br>firmware/drivers/mpu6050/mpu6050.c<br>firmware/services/sensors.c<br>firmware/services/sensors.h | mpu6050_init, mpu6050_read, sensors_init, sensors_process, sensors_snapshot | — | implemented, untested · unit tests: FAILED |

Observed on the board:
- `IF-001`, `FR-005` — Fresh successful build, ST-Link flash and 15-second COM15 reset capture observation:ec3a495b-0f94-4021-b6f5-b628f25660f7. Ignored pre-BOOT buffered record. Current boot reports DHT PB4 internal-pullups, followed by repeated DHT frame/checksum OK and climate_valid=1 telemetry: 23.8 degC/54.2 percent initially, then 23.7 degC/54.3 percent, later 23.7 degC/53.6 percent. No DHT failure appeared. This supports real acquisition and valid decoding, consistent with earlier successful startup evidence. IF-001 hardware criterion is only partially established: no independent PB4 waveform/instrument capture was available to compare the transmitted response bits with returned readings. Do not treat serial decoded values alone as proof of waveform/value agreement; that physical comparison remains owed.
- `FR-002` — Fresh reset capture observation:800c3a09-6646-4f46-bd4c-79aa51f0cc20: OLED splash started, then DHT frame/checksum OK and MPU calibration complete before OLED dashboard transition. At 1200ms climate and acceleration valid; near X=0,Y=0,Z=1g. Existing cooperative integration already supplies the deferred behavior; no logic change needed. Fake-clock deadline criteria remain untested.

Notes recorded with these traces:
- `IF-001`, `FR-005` — User explicitly retained 2000ms despite Aosong section 7 strict >2s guidance. Hardware waveform and response/value comparison not measured.
- `IF-002`, `IF-007` — Single main-context bus owner with busy guard. OLED uses this shared contract in category 3; simultaneous-request completion not yet tested.
- `FR-007`, `FR-008`, `FR-009`, `DIAG-001` — 10ms MPU scheduling, 8192 counts/g, flat/still calibration over 1000ms. Invalid acquisitions clear validity and use NaN. Axis consumers in category 3/4 remain to be wired; host behavior evidence pending.

## OLED Display & Startup UI

gate: reviewed · tasks: 5/5 done · recorded in: 8eeeb85d-cd97-4429-8f7b-89539bdd7e8a

| Req | Statement | Verified by | Acceptance criteria | Files | Symbols | Tests | Status |
|---|---|---|---|---|---|---|---|
| `IF-003` | The firmware shall drive a 1.3-inch I2C OLED using the SH1106 driver over PB8 (SCL) and PB9 (SDA), shared with the MPU6050. | inspection | The display configuration selects the SH1106 driver and assigns I2C SCL to PB8 and SDA to PB9 on the bus also used by the MPU6050. (expected: SH1106 selected; SCL=PB8; SDA=PB9; the display and MPU6050 use the same I2C bus.)<br>The display component specification identifies a 1.3-inch OLED. (expected: Display diagonal is 1.3 in.) | firmware/drivers/sh1106/sh1106.c<br>firmware/platforms/stm32/platform_io.c<br>firmware/services/display.c<br>sources/PRD.md | display_init, sh1106_init, sh1106_page | — | implemented, untested · unit tests: PARTIAL - still needs inspection |
| `HW-002` | The firmware project shall contain the CraftifAI logo as a flash-resident 1-bit monochrome bitmap array in logo.h, formatted for the SH1106 as 128x64 pixels, 8 pages by 128 columns, with LSB as the top row. | inspection | The logo.h asset declares a flash-resident 1-bit bitmap containing 1024 bytes in SH1106 page order. (expected: A const/flash-resident bitmap array with 8 × 128 = 1024 bytes, each byte representing one page column.)<br>The logo asset definition specifies that each byte's least-significant bit maps to the top row of its 8-pixel page. (expected: Bit 0 maps to the top row, and the asset dimensions are 128 columns by 8 pages (128×64 pixels).) | firmware/services/display.c<br>firmware/services/logo.h<br>sources/craftiailogo.png | display_process | — | implemented, untested · unit tests: NOT_RUN - still needs inspection |
| `FR-001` | The firmware shall display the 124x26-pixel logo artwork centred on the display with a black border, white on a black background. | inspection | The splash rendering code places the 124×26 logo at coordinates that center it within a 128×64 display. (expected: Logo dimensions are 124×26 pixels and its top-left coordinate is x=2, y=19.)<br>The splash renderer clears the display to black and draws the logo as white pixels without scaling or inversion. (expected: Background and border pixels are black; set logo bits render white; source bitmap coordinates map one-to-one to display pixels.) | firmware/services/display.c<br>firmware/services/logo.h<br>sources/craftiailogo.png | display_process | — | implemented, untested · unit tests: PARTIAL - still needs inspection |
| `FR-003` | The firmware shall implement splash timing non-blockingly using HAL_GetTick rather than a blocking delay. | inspection | The splash elapsed-time check uses HAL_GetTick and contains no blocking delay call. (expected: Elapsed time is determined using HAL_GetTick; no blocking delay is used to hold the splash.) | firmware/app/app.c<br>firmware/platforms/stm32/platform_io.c<br>firmware/services/display.c | app_start, display_process | — | implemented, untested · unit tests: BLOCKED - still needs inspection |
| `FR-004` | During the logo splash, the firmware shall show no alerts and shall not blink the LED. | host test | While the splash state is active, processing an alert does not add an alert to the rendered display and does not command the LED to blink. (expected: No alert message is rendered, and no blink command or blinking LED state is produced during the splash interval.) | firmware/app/app.c<br>firmware/platforms/stm32/platform_io.c<br>firmware/services/display.c | app_start, display_process | — | implemented, untested · unit tests: PASSED |
| `IF-004` | The SH1106 driver shall apply a 2-column offset when writing the logo and all other screens to account for its 132 columns of RAM and 128 visible columns. | host test | The SH1106 driver adds two columns to the starting column for writes that render both the logo and a non-logo screen. (expected: For each rendered screen, the driver uses visible column c at SH1106 RAM column c+2; the offset is applied exactly once.) | firmware/drivers/sh1106/sh1106.c<br>firmware/platforms/stm32/platform_io.c<br>firmware/services/display.c<br>sources/PRD.md | display_init, sh1106_init, sh1106_page | — | implemented, untested · unit tests: PASSED |
| `FR-012` | When an event occurs, the firmware shall immediately show its message on the OLED while keeping Temperature and Humidity visible. | host test | When an event is processed, the next rendered screen contains its message and retains the current temperature and humidity values. (expected: The event message and both previously supplied temperature and humidity values are present in the rendered screen.)<br>The event message is included in the first display update generated after the event is processed. (expected: The first display update after event processing contains the event message.) | firmware/services/display.c<br>firmware/services/display.h | display_process, display_render, display_set_event | — | implemented, untested · unit tests: PASSED |
| `FR-016` | After the splash, the firmware shall display a Vehicle Parameters screen containing temperature from the DHT22 in degC, humidity from the DHT22 in percent, and X, Y, and Z acceleration values in g. | host test | After the splash transition, the rendered screen is titled Vehicle Parameters and includes DHT22 temperature in degC, DHT22 humidity in %, and X, Y, and Z acceleration values with g shown for Z. (expected: The screen contains the title Vehicle Parameters, the supplied temperature with degC, the supplied humidity with %, the supplied X/Y/Z values, and g for Z.)<br>The first sensor screen following the splash renders valid zero values for X, Y, and Z when the initial acceleration values are zero. (expected: The first sensor screen displays valid zero readings for X, Y, and Z.) | firmware/services/display.c<br>firmware/services/display.h | display_process, display_render, display_set_event | — | implemented, untested · unit tests: PASSED |
| `IF-006` | The firmware shall initialize the SH1106 display interface before its first display write, early enough for the logo to appear within 100 ms of power-up. | hardware / instrument | The SH1106 interface is initialized before the first logo display write, and the logo becomes visible within 100 ms of power-up. (expected: Initialization precedes the first logo write, and visible logo onset is no later than 100 ms after power-up.) | firmware/app/app.c<br>firmware/platforms/stm32/platform_io.c<br>firmware/services/display.c | app_start, display_process | — | implemented, untested · unit tests: FAILED |
| `FR-018` | On power-up or reset, the firmware shall display the CraftifAI logo on the OLED for 2 seconds before displaying sensor data or a heading, then switch to the Vehicle Parameters screen. | hardware / instrument | On power-up, the logo becomes visible within 100 ms, remains the displayed content for 1.9–2.1 seconds, and precedes any sensor data or heading. (expected: Logo onset is at or before 100 ms; logo duration is 1.9–2.1 s; no sensor data or heading appears before the logo.)<br>After splash completion, the OLED changes to the Vehicle Parameters screen, and resetting during normal operation causes the logo splash to play again. (expected: The post-splash screen is Vehicle Parameters; after reset, the logo is displayed again before the normal screen.) | firmware/app/app.c<br>firmware/platforms/stm32/platform_io.c<br>firmware/services/display.c | app_start, display_process | — | implemented, untested · unit tests: FAILED |
| `FR-021` | The firmware shall display the acceleration values on the Vehicle Parameters screen using the labels "X:", "Y:", and "Z:", with the Z value followed by " g". | host test | The Vehicle Parameters renderer labels the acceleration readings exactly X:, Y:, and Z:, and appends the exact suffix ' g' to the Z value. (expected: The acceleration labels are exactly X:, Y:, and Z:, and the Z value is followed by ' g'.) | firmware/services/display.c<br>firmware/services/display.h | display_process, display_render, display_set_event | — | implemented, untested · unit tests: PASSED |

Observed on the board:
- `FR-003`, `FR-004`, `IF-006`, `FR-018` — Review fresh flash/reset capture observation:2e5ec9f2-a5b9-4bd3-aafb-c9f593b400e6 reproduced prior checkpoint sequence: OLED initialized; stabilizing -> OLED logo transfer complete; splash started -> MPU calibration complete -> OLED dashboard transition, with repeated DHT checksum OK and climate=1 imu=1 calibrated=1. No transfer failure printed; no regression in captured sequence. Serial cannot observe visible logo onset/duration, panel appearance or reset replay; these hardware criteria remain pending external visual/timestamp and bus-trace evidence. <=100ms onset intentionally not met under user's datasheet-sequence decision.

Notes recorded with these traces:
- `IF-003`, `IF-004` — 1.3-inch part specified by PRD; shared I2C1 callback; each page starts at RAM column 2. Host offset tests pending.
- `HW-002`, `FR-001` — Const 1024-byte page bitmap, LSB top. Source dark artwork bounds (31,171),370x77 aspect-preserving fitted in 124x26 region at (2,19), white on black.
- `FR-003`, `FR-004`, `IF-006`, `FR-018` — HAL_GetTick through platform clock; 2000ms splash, no hold delay. User chose datasheet 100ms stabilization over <=100ms visible onset; IF-006 and FR-018 onset criteria not satisfied. Visual/timestamp/reset evidence pending. LED alerts not yet implemented and splash suppression must be retained by category 4.
- `FR-012`, `FR-016`, `FR-021` — Dashboard title, temperature degC, humidity percent and X:/Y:/Z: with Z suffix g. Invalid readings show --. Event setter currently uncalled pending category 4; first-update alert acceptance therefore pending integration.

## Ride Event Detection & LED Alerts

gate: deferred · tasks: 5/5 done · recorded in: 8eeeb85d-cd97-4429-8f7b-89539bdd7e8a

| Req | Statement | Verified by | Acceptance criteria | Files | Symbols | Tests | Status |
|---|---|---|---|---|---|---|---|
| `FR-006` | When temperature or humidity exceeds its configured high limit, the firmware shall display an exclamation mark next to the corresponding value. | hardware / instrument | When the displayed temperature is greater than its configured high limit, the OLED shows an exclamation mark adjacent to the temperature value. (expected: In each mode, the temperature value is displayed with an adjacent "!" while it is above that mode's configured limit.)<br>When the displayed humidity is greater than its configured high limit, the OLED shows an exclamation mark adjacent to the humidity value. (expected: In each mode, the humidity value is displayed with an adjacent "!" while it is above that mode's configured limit.) | firmware/configs/app_config.h<br>firmware/services/display.c | display_process, display_render | — | implemented, untested · unit tests: PARTIAL - still needs hardware / instrument |
| `FR-010` | The firmware shall detect an event when its associated axis changes by at least the configured threshold within 100 ms. | host test | A change meeting the associated event threshold and arriving exactly 100 ms after the baseline sample is detected as an event. (expected: The qualifying event is detected.)<br>A change meeting the associated event threshold but arriving more than 100 ms after the baseline sample is not detected as a change within the configured sudden-change window. (expected: The sample pair does not trigger detection based on the 100 ms window.) | firmware/app/app.c<br>firmware/configs/app_config.h<br>firmware/modules/ride_events.c<br>firmware/modules/ride_events.h | app_start, ride_events_message, ride_events_sample, ride_events_update | — | implemented, untested · unit tests: PASSED |
| `FR-011` | The firmware shall detect and identify Unseen SpeedBreaker when Z changes by at least 0.30 g, Wide Turn when X changes by at least 0.25 g, Speeding when Y changes positively by at least 0.25 g, and Sudden Braking when Y changes negatively by at least 0.25 g. | host test | A Z-axis change of at least 0.30 g identifies an Unseen SpeedBreaker in demo mode. (expected: The identified event type is Unseen SpeedBreaker.)<br>An X-axis change of at least 0.25 g identifies a Wide Turn in demo mode. (expected: The identified event type is Wide Turn.)<br>A positive Y-axis change of at least 0.25 g identifies Speeding in demo mode. (expected: The identified event type is Speeding.)<br>A negative Y-axis change with magnitude at least 0.25 g identifies Sudden Braking in demo mode. (expected: The identified event type is Sudden Braking.)<br>Each in-vehicle event threshold is configurable within its stated 0.4–0.5 g range. (expected: Each of the four configured thresholds is between 0.4 g and 0.5 g, inclusive.) | firmware/app/app.c<br>firmware/configs/app_config.h<br>firmware/modules/ride_events.c<br>firmware/modules/ride_events.h | app_start, ride_events_message, ride_events_sample, ride_events_update | — | implemented, untested · unit tests: PASSED |
| `IF-005` | The firmware shall control the inbuilt user LED LD4 through PA5. | inspection | The LD4 control configuration maps the LED output to GPIO PA5. (expected: LD4 is controlled through PA5.) | firmware/app/app.c<br>firmware/bsp/board.h<br>firmware/modules/ride_events.c<br>firmware/platforms/stm32/platform_io.c | app_start, platform_io_init, platform_io_led, ride_events_update | — | implemented, untested · unit tests: PARTIAL - still needs inspection |
| `FR-013` | While an event alert is active, the firmware shall blink LD4 at 1 Hz with 500 ms on and 500 ms off. | hardware / instrument | While an event alert is active, LD4 has 500 ms on intervals alternating with 500 ms off intervals. (expected: Each on interval is 500 ms and each following off interval is 500 ms, producing a 1 Hz cycle.) | firmware/app/app.c<br>firmware/bsp/board.h<br>firmware/modules/ride_events.c<br>firmware/platforms/stm32/platform_io.c | app_start, platform_io_init, platform_io_led, ride_events_update | — | implemented, untested · unit tests: PARTIAL - still needs hardware / instrument |
| `FR-014` | The firmware shall continue displaying the event message and blinking the LED for 2 seconds after the last event. | hardware / instrument | After the last event, the OLED continues showing the event message until the 2-second post-event interval ends, then clears the message. (expected: The event message remains displayed before the 2-second deadline and is no longer displayed at the deadline.)<br>After the last event, LD4 continues blinking until the 2-second post-event interval ends, then stops blinking. (expected: Blinking continues before the 2-second deadline and ceases at the deadline.) | firmware/app/app.c<br>firmware/bsp/board.h<br>firmware/modules/ride_events.c<br>firmware/platforms/stm32/platform_io.c | app_start, platform_io_init, platform_io_led, ride_events_update | — | implemented, untested · unit tests: PARTIAL - still needs hardware / instrument |
| `FR-015` | When event messages overlap, the firmware shall prioritize them in this order: Sudden Braking, Unseen SpeedBreaker, Wide Turn, then Speeding. | host test | When multiple listed events are active simultaneously, the selected event message is the highest-priority active event in the specified order. (expected: Sudden Braking is selected over Unseen SpeedBreaker, Wide Turn, and Speeding; Unseen SpeedBreaker is selected over Wide Turn and Speeding; and Wide Turn is selected over Speeding.) | firmware/app/app.c<br>firmware/configs/app_config.h<br>firmware/modules/ride_events.c<br>firmware/modules/ride_events.h | app_start, ride_events_message, ride_events_sample, ride_events_update | — | implemented, untested · unit tests: PASSED |

Observed on the board:
- `FR-006`, `IF-005`, `FR-013`, `FR-014` — Fresh build/flash and 20-second reset capture observation:58c1b3ea-68a5-4a70-aac8-4f8a289921dc: MPU init/calibration OK, DHT checksum OK, OLED dashboard transition, valid climate 23.8 C and humidity 54.8 down to 53.3 percent. All captured telemetry event codes were 0; no qualifying motion event occurred. These below-limit readings do not exercise FR-006 above-limit markers. No physical OLED image or PA5 waveform was measured, so FR-006 (both modes), FR-013 500ms intervals, and FR-014 message/LED expiry remain pending, not passed. Source inspection maps LD4 to PA5; logs alone cannot establish physical outputs. Build succeeded with six newlib syscall linker warnings. No firmware changes made.

Notes recorded with these traces:
- `FR-006` — Independent strict greater-than climate markers only for valid measurements; mode selects climate thresholds. Physical high-limit trials not performed.
- `FR-010`, `FR-011`, `FR-015` — Retains up to 16 samples; compares pairs aged <=100ms, signed Y and absolute X/Z thresholds. Independent 2000ms event expiry; descending enum selects Braking > Bump > Turn > Speeding. Null/nonfinite samples reset detection history. Demo default, vehicle selection APP_VEHICLE_MODE. Host tests not run.
- `IF-005`, `FR-013`, `FR-014` — PA5 LD4 output initialized off. Logical 500ms on/500ms off, held events expire after 2000ms since each type last detected; splash excludes event processing and LED on. No event injection or PA5 waveform measurement in this capture.

## Dashboard Data Streaming

gate: deferred · tasks: 2/2 done · recorded in: 8eeeb85d-cd97-4429-8f7b-89539bdd7e8a

| Req | Statement | Verified by | Acceptance criteria | Files | Symbols | Tests | Status |
|---|---|---|---|---|---|---|---|
| `FR-017` | The firmware shall stream cabin comfort and ride data to a dashboard for driver-behaviour review or ride-comfort tracking. | hardware / instrument | The dashboard receives and makes available both cabin-comfort data and ride data from the firmware during operation. (expected: Both cabin-comfort data and ride data from the firmware are available at the dashboard.) | firmware/app/app.c<br>firmware/configs/app_config.h<br>firmware/platforms/stm32/platform_io.c<br>firmware/services/telemetry-format.md<br>firmware/services/telemetry.c<br>firmware/services/telemetry.h | app_start, platform_io_poll, platform_io_telemetry, telemetry_process | — | implemented, untested · unit tests: BLOCKED - still needs hardware / instrument |

Observed on the board:
- `FR-017` — Fresh build, ST-Link flash and 15-second COM15 115200 reset capture observation:602c4286-c502-40b7-83c4-06681c582026. Disregarded leading pre-BOOT buffered VPD1 line. After BOOT, sequences 0..74 received with nominal 200ms spacing (2ms dashboard-transition jitter). Both validity bits become 1 at sequence 9: VPD1,9,2000,1,23800,53100,1,0,-1,1002,0,0 = 23.8 C, 53.1% RH, X=0g Y=-0.001g Z=1.002g. Subsequent records include changing climate and ride readings; reported dropped count stays zero. This establishes reception of both data types at the serial monitor, NOT ingestion or availability in a dashboard application. No dashboard application supplied/observed, so FR-017 hardware acceptance remains pending. Build succeeded with six newlib syscall-stub linker warnings.

Notes recorded with these traces:
- `FR-017` — Selected local ST-Link VCP transport because no dashboard interface specified. VPD1 CRLF CSV at nominal 5Hz, explicit validity, scaled units, sequence, uptime and drop count. A supplied dashboard application's ingestion is not established; criterion remains pending.

## Gaps

Listed rather than omitted. A matrix showing only what was built cannot be told apart from one showing full coverage.

### Host tests you could write (15)

These have a `host` acceptance criterion and cite no test. They are tested at the end, from Artifacts -> Unit Tests.

- `IF-007` (Sensor Acquisition & I2C Bus)
- `FR-002` (Sensor Acquisition & I2C Bus)
- `FR-005` (Sensor Acquisition & I2C Bus)
- `FR-007` (Sensor Acquisition & I2C Bus)
- `FR-008` (Sensor Acquisition & I2C Bus)
- `FR-009` (Sensor Acquisition & I2C Bus)
- `DIAG-001` (Sensor Acquisition & I2C Bus)
- `FR-004` (OLED Display & Startup UI)
- `IF-004` (OLED Display & Startup UI)
- `FR-012` (OLED Display & Startup UI)
- `FR-016` (OLED Display & Startup UI)
- `FR-021` (OLED Display & Startup UI)
- `FR-010` (Ride Event Detection & LED Alerts)
- `FR-011` (Ride Event Detection & LED Alerts)
- `FR-015` (Ride Event Detection & LED Alerts)

### Acceptance needs non-host evidence (18)

No `host` acceptance criterion is recorded. Host tests cannot settle these acceptance claims, but implementation calculations and failure recovery can still need regression tests. Each line identifies the required acceptance evidence; do not substitute a host result for it.

- `HW-001` — checked by hardware / instrument · _Board & Firmware Configuration_
- `HW-003` — checked by inspection · _Board & Firmware Configuration_
- `HW-004` — checked by inspection · _Board & Firmware Configuration_
- `FR-019` — checked by inspection · _Board & Firmware Configuration_
- `FR-020` — checked by inspection · _Board & Firmware Configuration_
- `IF-001` — checked by hardware / instrument · _Sensor Acquisition & I2C Bus_
- `IF-002` — checked by inspection · _Sensor Acquisition & I2C Bus_
- `IF-003` — checked by inspection · _OLED Display & Startup UI_
- `HW-002` — checked by inspection · _OLED Display & Startup UI_
- `FR-001` — checked by inspection · _OLED Display & Startup UI_
- `FR-003` — checked by inspection · _OLED Display & Startup UI_
- `IF-006` — checked by hardware / instrument · _OLED Display & Startup UI_
- `FR-018` — checked by hardware / instrument · _OLED Display & Startup UI_
- `FR-006` — checked by hardware / instrument · _Ride Event Detection & LED Alerts_
- `IF-005` — checked by inspection · _Ride Event Detection & LED Alerts_
- `FR-013` — checked by hardware / instrument · _Ride Event Detection & LED Alerts_
- `FR-014` — checked by hardware / instrument · _Ride Event Detection & LED Alerts_
- `FR-017` — checked by hardware / instrument · _Dashboard Data Streaming_

### Implemented, never tested (33)

Code exists and no test is recorded against it.

- `HW-001` (Board & Firmware Configuration)
- `HW-003` (Board & Firmware Configuration)
- `HW-004` (Board & Firmware Configuration)
- `FR-019` (Board & Firmware Configuration)
- `FR-020` (Board & Firmware Configuration)
- `IF-001` (Sensor Acquisition & I2C Bus)
- `IF-002` (Sensor Acquisition & I2C Bus)
- `IF-007` (Sensor Acquisition & I2C Bus)
- `FR-002` (Sensor Acquisition & I2C Bus)
- `FR-005` (Sensor Acquisition & I2C Bus)
- `FR-007` (Sensor Acquisition & I2C Bus)
- `FR-008` (Sensor Acquisition & I2C Bus)
- `FR-009` (Sensor Acquisition & I2C Bus)
- `DIAG-001` (Sensor Acquisition & I2C Bus)
- `IF-003` (OLED Display & Startup UI)
- `HW-002` (OLED Display & Startup UI)
- `FR-001` (OLED Display & Startup UI)
- `FR-003` (OLED Display & Startup UI)
- `FR-004` (OLED Display & Startup UI)
- `IF-004` (OLED Display & Startup UI)
- `FR-012` (OLED Display & Startup UI)
- `FR-016` (OLED Display & Startup UI)
- `IF-006` (OLED Display & Startup UI)
- `FR-018` (OLED Display & Startup UI)
- `FR-021` (OLED Display & Startup UI)
- `FR-006` (Ride Event Detection & LED Alerts)
- `FR-010` (Ride Event Detection & LED Alerts)
- `FR-011` (Ride Event Detection & LED Alerts)
- `IF-005` (Ride Event Detection & LED Alerts)
- `FR-013` (Ride Event Detection & LED Alerts)
- `FR-014` (Ride Event Detection & LED Alerts)
- `FR-015` (Ride Event Detection & LED Alerts)
- `FR-017` (Dashboard Data Streaming)
