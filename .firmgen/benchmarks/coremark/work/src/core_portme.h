/*
 * FirmGen port of EEMBC CoreMark for STM32 HAL2 (new-generation STM32Cube) projects.
 *
 * Derived from CoreMark's barebones/core_portme.h, Copyright 2018 EEMBC, licensed under the
 * Apache License, Version 2.0 (see upstream/LICENSE.md). Written by FirmGen into the benchmark
 * build only; the CoreMark sources it is compiled with are unmodified.
 */
#ifndef CORE_PORTME_H
#define CORE_PORTME_H

#include <stddef.h>

/* COMPILER_VERSION and COMPILER_FLAGS, written by FirmGen from the benchmark's real build. */
#include "firmgen_coremark_build.h"

#define HAS_FLOAT  1
#define HAS_TIME_H 0
#define USE_CLOCK  0
#define HAS_STDIO  1
#define HAS_PRINTF 0
int fg_coremark_printf(const char *format, ...);
#define ee_printf fg_coremark_printf

/* 0: CoreMark sizes the run itself to at least 10 seconds, as its run rules require. */
#ifndef ITERATIONS
#define ITERATIONS 0
#endif

#ifndef MEM_LOCATION
#define MEM_LOCATION "STATIC"
#endif

typedef signed short   ee_s16;
typedef unsigned short ee_u16;
typedef signed int     ee_s32;
typedef double         ee_f32;
typedef unsigned char  ee_u8;
typedef unsigned int   ee_u32;
typedef ee_u32         ee_ptr_int;
typedef size_t         ee_size_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#define align_mem(x) (void *)(4 + (((ee_ptr_int)(x)-1) & ~3))

#define CORETIMETYPE ee_u32
typedef ee_u32 CORE_TICKS;

#define SEED_METHOD SEED_VOLATILE
/* Static, not on the stack: ST's templates reserve as little as 1 KB of stack, and CoreMark's
 * 2 KB data block there would overflow it. */
#define MEM_METHOD MEM_STATIC

#define MULTITHREAD 1
#define USE_PTHREAD 0
#define USE_FORK    0
#define USE_SOCKET  0

#define MAIN_HAS_NOARGC   1
#define MAIN_HAS_NORETURN 0

extern ee_u32 default_num_contexts;

typedef struct CORE_PORTABLE_S
{
    ee_u8 portable_id;
} core_portable;

void portable_init(core_portable *p, int *argc, char *argv[]);
void portable_fini(core_portable *p);

#if !defined(PROFILE_RUN) && !defined(PERFORMANCE_RUN) && !defined(VALIDATION_RUN)
#if (TOTAL_DATA_SIZE == 1200)
#define PROFILE_RUN 1
#elif (TOTAL_DATA_SIZE == 2000)
#define PERFORMANCE_RUN 1
#else
#define VALIDATION_RUN 1
#endif
#endif

#endif /* CORE_PORTME_H */
