/* Main-context orchestration of sensing, startup display and ride alerts. */
#include "app.h"
#include "platform_io.h"
#include "sensors.h"
#include "display.h"
#include "ride_events.h"
#include "telemetry.h"
#include "app_config.h"
#include <stddef.h>
#include "firmgen_telemetry.h"
UART_HandleTypeDef *firmgen_uart_handle(void);   /* firmgen_telemetry */

void app_start(void)
{
    firmgen_telemetry_start(firmgen_uart_handle());
    ride_events_t events;
    ride_event_t previous_event = RIDE_NONE;
    bool previous_led = false;
    bool is_display_ready;
    if (!platform_io_init())
    {
        return;
    }
    platform_io_scan();
    is_display_ready = display_init(platform_io_now_ms());
    sensors_init(platform_io_now_ms());
    ride_events_init(&events, APP_VEHICLE_MODE != 0);
    for (;;)
    {
        firmgen_telemetry_poll();
        uint32_t now_ms = platform_io_now_ms();
        platform_io_poll();
        const sensors_snapshot_t *readings;
        bool led_on = false;
        ride_event_t event = RIDE_NONE;
        sensors_process(now_ms);
        readings = sensors_snapshot();
        if (is_display_ready && !display_is_splash())
        {
            if (readings->acceleration_valid)
            {
                ride_sample_t sample = {readings->acceleration.x_g,
                    readings->acceleration.y_g, readings->acceleration.z_g,
                    readings->acceleration_time_ms};
                ride_events_sample(&events, &sample);
            }
            else
            {
                ride_events_sample(&events, NULL);
            }
            event = ride_events_update(&events, now_ms, &led_on);
        }
        if (event != previous_event)
        {
            display_set_event(ride_events_message(event));
            platform_io_log(event == RIDE_NONE ? "EVENT cleared\r\n" : "EVENT selected: ");
            if (event != RIDE_NONE)
            {
                platform_io_log(ride_events_message(event));
                platform_io_log("\r\n");
            }
            previous_event = event;
        }
        platform_io_led(led_on);
        if (led_on != previous_led)
        {
            platform_io_log(led_on ? "LED command ON\r\n" : "LED command OFF\r\n");
            previous_led = led_on;
        }
        if (is_display_ready)
        {
            display_process(platform_io_now_ms(), readings);
        }
        telemetry_process(platform_io_now_ms(), readings, event);
    }
}
