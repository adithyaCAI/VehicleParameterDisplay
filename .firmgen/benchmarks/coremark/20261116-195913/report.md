# CoreMark report

| | |
|---|---|
| Status | **PASS** (`COREMARK_PASS`) |
| Run | 20261116-195913 |
| Date | 2026-11-16 19:59:13 |
| Board | NUCLEO-G070RB |
| Platform | STM32 classic / CMSIS reset clock / SWD capture |
| Compiler | ARM GCC |
| Compiler version | arm-none-eabi-gcc.exe (GNU Tools for STM32 14.3.rel1.20251027-0700) 14.3.1 20250623 |
| Compiler flags | `-mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -O3 -std=gnu11 -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -Wall` |
| Build preset | Release-arm-gcc |

## Result

| | |
|---|---|
| CoreMark (iterations/s) | **31.449** |
| CoreMark/MHz | 1.966 |
| Core clock | 16 MHz (SystemCoreClock) |
| Iterations | 400 |
| Timed run | 12.719 s (12719 ticks at 1000 Hz) |
| Run type | 2K performance |
| Data in | STATIC |
| CoreMark source | https://github.com/eembc/coremark @ 1f483d5b8316 |

CoreMark's own report line:

```
CoreMark 1.0 : 31.449013 / arm-none-eabi-gcc.exe (GNU Tools for STM32 14.3.rel1.20251027-0700) 14.3.1 20250623 -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -O3 -std=gnu11 -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -Wall / STATIC
```

A CoreMark score measured by FirmGen on this board. It follows CoreMark's run and validation rules but is not an EEMBC-certified result.

The target and build sections identify the benchmark's clock mode and optimization. Classic STM32 uses the vendor reset clock, HAL2 uses generated board initialization, and ESP-IDF runs one CPU with power management disabled. Application tasks are excluded. Port/runtime overhead remains part of the measurement; this is not application timing validation.

## Checks

- ✓ **Board output**: the CoreMark image started and printed its banner
- ✓ **Runtime diagnostics**: no runtime fault reported
- ✓ **Run completed**: CoreMark finished and printed its results
- ✓ **Run type**: 2K performance run
- ✓ **CRC validation**: seedcrc 0xe9f5, crclist 0xe714, crcmatrix 0x1fd7, crcstate 0x8e3a — "Correct operation validated."
- ✓ **Run time**: 12.719 s timed (at least 10 s required)
- ✓ **Timer cross-check**: host clock 16.21 s from start to finish; the board's timer implies 16.22 s
- ✓ **Score consistency**: printed 31.449 / computed 31.449 iterations/s

## Application

✓ Restored. C:\Users\Adith\OneDrive\Desktop\VPD\vehicleparametersdisplay\.firmgen\benchmarks\coremark\restore\345f3211bd6c3517853020a0da08f5de0da2065a0580cac4dfcda7701cbb882b.elf flashed back, verified and reset. Runtime startup identity was not independently verified.

## Method

Built separately with the project's target toolchain and board support. The build preset and compiler flags above describe this benchmark, not the application. Classic STM32 uses the vendor reset clock; HAL2 uses generated board initialization; ESP-IDF runs on one CPU with dynamic power management disabled; TI uses the labeled C28x adaptation. Application sources and the normal build directory are not modified.

## Board output

```
FIRMGEN_COREMARK_BEGIN clock_hz=16000000 tick_hz=1000
Clock mode: vendor reset clock; not application PLL configuration
2K performance run parameters for coremark.
CoreMark Size    : 666
Total ticks      : 12719
Total time (secs): 12.719000
Iterations/Sec   : 31.449013
Iterations       : 400
Compiler version : arm-none-eabi-gcc.exe (GNU Tools for STM32 14.3.rel1.20251027-0700) 14.3.1 20250623
Compiler flags   : -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -O3 -std=gnu11 -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -Wall
Memory location  : STATIC
seedcrc          : 0xe9f5
[0]crclist       : 0xe714
[0]crcmatrix     : 0x1fd7
[0]crcstate      : 0x8e3a
[0]crcfinal      : 0x25b5
Correct operation validated. See README.md for run and reporting rules.
CoreMark 1.0 : 31.449013 / arm-none-eabi-gcc.exe (GNU Tools for STM32 14.3.rel1.20251027-0700) 14.3.1 20250623 -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -O3 -std=gnu11 -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -Wall / STATIC
FIRMGEN_COREMARK_END
```
