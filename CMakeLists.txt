cmake_minimum_required(VERSION 3.22)
# Toolchain must be set before project(): -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake (ARM GCC)
# or cmake/iar-arm.cmake (IAR). FirmGen passes the one for the project's selected compiler.
project(vehicleparametersdisplay C ASM)
set(CMAKE_C_STANDARD 11)

# FIRMGEN_COMPILER_SWITCH: every compiler-specific setting below sits in an
# `if(CMAKE_C_COMPILER_ID STREQUAL "IAR")` branch, so this one file builds with either compiler.

# CPU flags for STM32G070xx (Cortex core from the STM32 family). Applied to BOTH compile and link.
# (ARM GCC. For IAR the core and FPU come from cmake/iar-arm.cmake.)
set(CPU_FLAGS "-mcpu=cortex-m0plus -mthumb")
separate_arguments(CPU_FLAGS UNIX_COMMAND "${CPU_FLAGS}")

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    # -e: IAR language extensions, which the HAL and CMSIS headers use (__weak, __packed). C ONLY:
    # to the IAR assembler -e means "big-endian", and a big-endian startup object fails the link
    # against the little-endian C objects (Fatal error[Li003]: endianness mismatch).
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-e>)
else()
    add_compile_options(${CPU_FLAGS} -ffunction-sections -fdata-sections -Wall)
endif()
# USE_HAL_DRIVER selects the HAL; STM32G070xx picks the exact device header in CMSIS.
add_compile_definitions(USE_HAL_DRIVER STM32G070xx)

# HAL/core/app sources. The entry point is Core/Src/main.c — do NOT glob src/*.c: a leftover
# src/main.c from an older scaffold would define a second main() and break the link.
# *_template.c files were excluded at copy time (they duplicate symbols).
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Core/Src/*.c
    ${CMAKE_SOURCE_DIR}/firmware/*.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/Src/*.c)
# firmware/{tests,docs,scripts}/ hold host test code, documentation and helpers -- never
# firmware sources. Generated unit-test bundles land in firmware/docs/unit_tests/, and the
# globs above use CONFIGURE_DEPENDS, so without this they enter the image on the NEXT build.
list(FILTER PROJECT_SOURCES EXCLUDE REGEX "/firmware/(tests|docs|scripts)/")

# Every folder under firmware/ that holds a header goes on the include path, so headers resolve by
# bare filename from any layer and a NEW layer folder needs no edit here. Listing the layers by
# hand is what broke `#include "board.h"` when bsp/ was added.
file(GLOB_RECURSE FW_HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/firmware/*.h)
list(FILTER FW_HEADERS EXCLUDE REGEX "/firmware/(tests|docs|scripts)/")
set(FW_INCLUDE_DIRS "")
foreach(_hdr ${FW_HEADERS})
    get_filename_component(_dir "${_hdr}" DIRECTORY)
    list(APPEND FW_INCLUDE_DIRS "${_dir}")
endforeach()
if(FW_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES FW_INCLUDE_DIRS)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    set(STARTUP ${CMAKE_SOURCE_DIR}/Core/Startup/iar/startup_stm32g070xx.s ${CMAKE_SOURCE_DIR}/Core/Startup/iar/firmgen_iar_syscalls.c)
    set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/stm32g070xx_flash.icf)
else()
    set(STARTUP ${CMAKE_SOURCE_DIR}/Core/Startup/startup_stm32g070rbtx.s)
    set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32G070RBTX_FLASH.ld)
endif()

add_executable(${PROJECT_NAME}.elf ${PROJECT_SOURCES} ${STARTUP})

target_include_directories(${PROJECT_NAME}.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${FW_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32G0xx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include)

# FIRMGEN_RTOS_SWITCH: -DFIRMGEN_RTOS=embos builds on SEGGER embOS (FirmGen passes it with the paths
# of the embOS files in your embOS installation), -DFIRMGEN_RTOS=freertos on the FreeRTOS of the
# STM32Cube package (FIRMGEN_FREERTOS_DIR / FIRMGEN_FREERTOS_PORT); anything else is bare metal. The
# FirmGen glue is Core/RTOS/firmgen_<rtos>.c; FIRMGEN_RTOS switches main() and the IRQ handlers over.
if(FIRMGEN_RTOS STREQUAL "embos")
    foreach(_fg_v FIRMGEN_EMBOS_INC FIRMGEN_EMBOS_LIB FIRMGEN_EMBOS_OSERROR)
        if(NOT EXISTS "${${_fg_v}}")
            message(FATAL_ERROR "embOS: ${_fg_v} is not set or does not exist ('${${_fg_v}}').")
        endif()
    endforeach()
    # SEGGER's start-up for the family (RTOSInit) when embOS ships one, else FirmGen's.
    if(FIRMGEN_EMBOS_RTOSINIT)
        set(_fg_embos_hw ${FIRMGEN_EMBOS_RTOSINIT})
    else()
        set(_fg_embos_hw ${CMAKE_SOURCE_DIR}/Core/RTOS/firmgen_embos_hw.c)
    endif()
    target_sources(${PROJECT_NAME}.elf PRIVATE
        ${CMAKE_SOURCE_DIR}/Core/RTOS/firmgen_embos.c
        ${_fg_embos_hw}
        ${FIRMGEN_EMBOS_OSERROR})
    target_include_directories(${PROJECT_NAME}.elf PRIVATE ${FIRMGEN_EMBOS_INC} ${FIRMGEN_EMBOS_EXTRA_INC})
    # One library mode everywhere RTOS.h is included, matching the library linked below.
    target_compile_definitions(${PROJECT_NAME}.elf PRIVATE FIRMGEN_RTOS FIRMGEN_EMBOS OS_LIBMODE_DP OS_VIEW_IFSELECT=OS_VIEW_DISABLED)
    target_link_libraries(${PROJECT_NAME}.elf ${FIRMGEN_EMBOS_LIB})
    # embOS names the vector table and the main stack's bounds after SEGGER's own startup and linker
    # script (__Vectors / _vectors, __stack_start__ / __stack_end__). ST's call them g_pfnVectors
    # (GNU), __vector_table (IAR) and _estack - _Min_Stack_Size (GNU .ld), so they are mapped here.
    if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
        set_source_files_properties(${_fg_embos_hw} PROPERTIES COMPILE_DEFINITIONS "__Vectors=__vector_table")
    else()
        target_link_options(${PROJECT_NAME}.elf PRIVATE
            "-Wl,--defsym=__Vectors=g_pfnVectors"
            "-Wl,--defsym=_vectors=g_pfnVectors"
            "-Wl,--defsym=__stack_end__=_estack"
            "-Wl,--defsym=__stack_start__=_estack-_Min_Stack_Size")
    endif()
elseif(FIRMGEN_RTOS STREQUAL "freertos")
    foreach(_fg_v FIRMGEN_FREERTOS_DIR FIRMGEN_FREERTOS_PORT)
        if(NOT EXISTS "${${_fg_v}}")
            message(FATAL_ERROR "FreeRTOS: ${_fg_v} is not set or does not exist ('${${_fg_v}}').")
        endif()
    endforeach()
    # The kernel, its port for this compiler and core (port.c, and portasm.c / portasm.s where the
    # port has one), and heap_4. The MPU wrappers are for MPU ports only.
    file(GLOB _fg_rtos_src ${FIRMGEN_FREERTOS_DIR}/*.c ${FIRMGEN_FREERTOS_PORT}/*.c ${FIRMGEN_FREERTOS_PORT}/*.s)
    list(REMOVE_DUPLICATES _fg_rtos_src)
    list(FILTER _fg_rtos_src EXCLUDE REGEX "mpu_wrappers")
    target_sources(${PROJECT_NAME}.elf PRIVATE
        ${CMAKE_SOURCE_DIR}/Core/RTOS/firmgen_freertos.c
        ${_fg_rtos_src}
        ${FIRMGEN_FREERTOS_DIR}/portable/MemMang/heap_4.c)
    target_include_directories(${PROJECT_NAME}.elf PRIVATE
        ${CMAKE_SOURCE_DIR}/Core/RTOS ${FIRMGEN_FREERTOS_DIR}/include ${FIRMGEN_FREERTOS_PORT})
    target_compile_definitions(${PROJECT_NAME}.elf PRIVATE FIRMGEN_RTOS FIRMGEN_FREERTOS)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    # CMake's IAR support appends ".elf" to executables; the target name already ends in .elf.
    set_target_properties(${PROJECT_NAME}.elf PROPERTIES SUFFIX "")
    target_link_options(${PROJECT_NAME}.elf PRIVATE
        "SHELL:--config ${LINKER_SCRIPT}"
        "SHELL:--map ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map")
    # .hex/.bin next to the .elf for flashing. CMake locates ielftool next to the IAR compiler.
    add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_IAR_ELFTOOL} --silent --ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
        COMMAND ${CMAKE_IAR_ELFTOOL} --silent --bin  $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin)
else()
    target_link_options(${PROJECT_NAME}.elf PRIVATE
        ${CPU_FLAGS}
        -T${LINKER_SCRIPT}
        -Wl,--gc-sections
        -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
        --specs=nano.specs --specs=nosys.specs
        -lc -lm)

    # .hex/.bin next to the .elf for flashing.
    add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin)
endif()

# >>> FIRMGEN_USER_COMPONENTS
# User-provided components (custom linker script, existing libraries/drivers).
# Generated from firmgen_user_components.json; see .craftif/user_components.cmake.
include("${CMAKE_CURRENT_LIST_DIR}/.craftif/user_components.cmake" OPTIONAL)
if(COMMAND firmgen_attach_user_components)
  firmgen_attach_user_components(${PROJECT_NAME}.elf)
endif()
# <<< FIRMGEN_USER_COMPONENTS
