/* FirmGen Cortex-M port. Vendor SystemInit supplies the reset clock.
 * No application handlers, RTOS, UART pin guesses, or persistent writes.
 * Output is read through SWD HOTPLUG without halting the timed workload. */
#include "coremark.h"
#include FIRMGEN_COREMARK_DEVICE_HEADER

volatile char fg_coremark_output[4096];
static unsigned output_used;
static volatile ee_u32 milliseconds;
static ee_u32 start_ticks, stop_ticks;
volatile ee_s32 seed1_volatile = 0;
volatile ee_s32 seed2_volatile = 0;
volatile ee_s32 seed3_volatile = 0x66;
volatile ee_s32 seed4_volatile = ITERATIONS;
volatile ee_s32 seed5_volatile = 0;
ee_u32 default_num_contexts = 1;

int fg_coremark_write(const char *data, unsigned length)
{
    unsigned i;
    if (length >= sizeof(fg_coremark_output) - output_used) return -1;
    for (i = 0; i < length; ++i) fg_coremark_output[output_used++] = data[i];
    fg_coremark_output[output_used] = '\0';
    __DMB();
    return (int)length;
}
void SysTick_Handler(void) { ++milliseconds; }
void start_time(void) { start_ticks = milliseconds; }
void stop_time(void) { stop_ticks = milliseconds; }
CORE_TICKS get_time(void) { return stop_ticks - start_ticks; }
secs_ret time_in_secs(CORE_TICKS ticks) { return (secs_ret)ticks / 1000.0; }
void portable_init(core_portable *p, int *argc, char *argv[])
{
    (void)argc; (void)argv;
    SystemCoreClockUpdate();
    if (!SystemCoreClock || SysTick_Config(SystemCoreClock / 1000U))
        for (;;) {}
    __enable_irq();
    /* Allow the host to attach its non-halting memory reader before calibration. */
    while (milliseconds < 5000U) {}
    ee_printf("FIRMGEN_COREMARK_BEGIN clock_hz=%lu tick_hz=1000\n",
              (unsigned long)SystemCoreClock);
    ee_printf("Clock mode: vendor reset clock; not application PLL configuration\n");
    p->portable_id = 1;
}
void portable_fini(core_portable *p)
{
    p->portable_id = 0;
    ee_printf("FIRMGEN_COREMARK_END\n");
    for (;;) {}
}
