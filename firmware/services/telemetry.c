/* VPD1 newline-delimited CSV telemetry, millidegrees/permille/milli-g units. */
#include "telemetry.h"
#include "platform_io.h"
#include "app_config.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

/* Sentinel is outside all supported physical ranges; validity remains explicit. */
static int32_t telemetry_scaled(float value, bool valid)
{
    if (!valid || !isfinite(value) || (fabsf(value) > 1000.0F))
    {
        return INT32_MIN;
    }
    return (int32_t)(value * 1000.0F);
}

void telemetry_process(uint32_t now_ms, const sensors_snapshot_t *readings, ride_event_t event)
{
    static uint32_t last_ms;
    static uint32_t sequence;
    static uint32_t dropped;
    char record[256];
    int length;
    if ((readings == NULL) || ((uint32_t)(now_ms - last_ms) < APP_TELEMETRY_PERIOD_MS))
    {
        return;
    }
    last_ms = now_ms;
    length = snprintf(record, sizeof(record),
        "VPD1,%lu,%lu,%u,%ld,%ld,%u,%ld,%ld,%ld,%u,%lu\r\n",
        (unsigned long)sequence, (unsigned long)now_ms,
        (unsigned int)readings->climate_valid,
        (long)telemetry_scaled(readings->temperature_c, readings->climate_valid),
        (long)telemetry_scaled(readings->humidity_percent, readings->climate_valid),
        (unsigned int)readings->acceleration_valid,
        (long)telemetry_scaled(readings->acceleration.x_g, readings->acceleration_valid),
        (long)telemetry_scaled(readings->acceleration.y_g, readings->acceleration_valid),
        (long)telemetry_scaled(readings->acceleration.z_g, readings->acceleration_valid),
        (unsigned int)event, (unsigned long)dropped);
    sequence++;
    if ((length < 0) || ((size_t)length >= sizeof(record)) || !platform_io_telemetry(record))
    {
        dropped++;
    }
}
