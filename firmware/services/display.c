/* SH1106 startup state and fixed eight-row dashboard, no vendor dependency. */
#include "display.h"
#include "sh1106.h"
#include "platform_io.h"
#include "config.h"
#include "logo.h"
#include "display_font.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

enum { DISPLAY_ADDRESS = 0x3C, STABILIZE_MS = 100, REFRESH_MS = 200 };
typedef enum { DISPLAY_STABILIZING, DISPLAY_SPLASH, DISPLAY_DASHBOARD, DISPLAY_FAILED } display_state_t;
static display_state_t state;
static uint32_t state_start_ms;
static uint32_t last_refresh_ms;
static uint8_t framebuffer[1024];
static uint8_t next_page;
static char event_message[43];
static bool is_dirty;

/* Bound text to one 21-character row; glyph bytes directly match page order. */
static void display_text(uint8_t row, const char *text)
{
    for (uint16_t column = 0U; (column < 21U) && (text[column] != '\0'); column++)
    {
        unsigned char character = (unsigned char)text[column];
        if ((character < 32U) || (character > 126U))
        {
            character = (unsigned char)'?';
        }
        memcpy(&framebuffer[(uint16_t)row * 128U + column * 6U],
               display_font[character - 32U], 6U);
    }
}

/* Fixed-point decimal formatting avoids newlib float-printf linkage. */
static void display_value(char *output, size_t capacity, float value, bool valid)
{
    if (!valid || !isfinite(value) || (fabsf(value) > 1000.0F))
    {
        (void)snprintf(output, capacity, "--");
    }
    else
    {
        int32_t tenths = (int32_t)(fabsf(value) * 10.0F + 0.5F);
        (void)snprintf(output, capacity, "%s%ld.%ld", value < 0.0F ? "-" : "",
                       (long)(tenths / 10), (long)(tenths % 10));
    }
}

/* Redraw a coherent snapshot; validity prevents stale readings looking healthy. */
static void display_render(const sensors_snapshot_t *readings)
{
    char line[32];
    char value[16];
    memset(framebuffer, 0, sizeof(framebuffer));
    display_text(0U, "Vehicle Parameters");
    display_value(value, sizeof(value), readings->temperature_c, readings->climate_valid);
    (void)snprintf(line, sizeof(line), "Temp:%s degC%s", value,
        readings->climate_valid && (readings->temperature_c >
        (APP_VEHICLE_MODE ? VEHICLE_TEMPERATURE_HIGH_C : TEMPERATURE_HIGH_C)) ? "!" : "");
    display_text(1U, line);
    display_value(value, sizeof(value), readings->humidity_percent, readings->climate_valid);
    (void)snprintf(line, sizeof(line), "Humidity:%s %%%s", value,
        readings->climate_valid && (readings->humidity_percent >
        (APP_VEHICLE_MODE ? VEHICLE_HUMIDITY_HIGH_PERCENT : HUMIDITY_HIGH_PERCENT)) ? "!" : "");
    display_text(2U, line);
    if (event_message[0] != '\0')
    {
        char first_line[22];
        memcpy(first_line, event_message, 21U);
        first_line[21] = '\0';
        display_text(3U, first_line);
        if (strlen(event_message) > 21U)
        {
            display_text(4U, &event_message[21]);
        }
    }
    display_value(value, sizeof(value), readings->acceleration.x_g, readings->acceleration_valid);
    (void)snprintf(line, sizeof(line), "X:%s", value);
    display_text(5U, line);
    display_value(value, sizeof(value), readings->acceleration.y_g, readings->acceleration_valid);
    (void)snprintf(line, sizeof(line), "Y:%s", value);
    display_text(6U, line);
    display_value(value, sizeof(value), readings->acceleration.z_g, readings->acceleration_valid);
    (void)snprintf(line, sizeof(line), "Z:%s g", value);
    display_text(7U, line);
}

bool display_init(uint32_t now_ms)
{
    state = DISPLAY_FAILED;
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(event_message, 0, sizeof(event_message));
    if (!sh1106_init(platform_io_transmit, DISPLAY_ADDRESS))
    {
        platform_io_log("FAULT OLED init\r\n");
        return false;
    }
    for (uint8_t page = 0U; page < 8U; page++)
    {
        if (!sh1106_page(platform_io_transmit, DISPLAY_ADDRESS, page, &framebuffer[(uint16_t)page * 128U]))
        {
            platform_io_log("FAULT OLED clear\r\n");
            return false;
        }
    }
    state_start_ms = platform_io_now_ms();
    (void)now_ms;
    state = DISPLAY_STABILIZING;
    platform_io_log("OLED initialized; stabilizing\r\n");
    return true;
}

void display_process(uint32_t now_ms, const sensors_snapshot_t *readings)
{
    if ((state == DISPLAY_FAILED) || (readings == NULL))
    {
        return;
    }
    if (state == DISPLAY_STABILIZING)
    {
        if ((uint32_t)(now_ms - state_start_ms) < STABILIZE_MS)
        {
            return;
        }
        for (uint8_t page = 0U; page < 8U; page++)
        {
            if (!sh1106_page(platform_io_transmit, DISPLAY_ADDRESS, page, &display_logo[(uint16_t)page * 128U]))
            {
                state = DISPLAY_FAILED;
                platform_io_log("FAULT OLED logo write\r\n");
                return;
            }
        }
        if (!sh1106_enable(platform_io_transmit, DISPLAY_ADDRESS))
        {
            state = DISPLAY_FAILED;
            platform_io_log("FAULT OLED enable\r\n");
            return;
        }
        state = DISPLAY_SPLASH;
        state_start_ms = platform_io_now_ms();
        platform_io_log("OLED logo transfer complete; splash started\r\n");
        return;
    }
    if (state == DISPLAY_SPLASH)
    {
        if ((uint32_t)(now_ms - state_start_ms) < LOGO_DISPLAY_MS)
        {
            return;
        }
        state = DISPLAY_DASHBOARD;
        is_dirty = true;
        platform_io_log("OLED dashboard transition\r\n");
    }
    if (is_dirty || ((next_page == 8U) && ((uint32_t)(now_ms - last_refresh_ms) >= REFRESH_MS)))
    {
        display_render(readings);
        next_page = 0U;
        last_refresh_ms = now_ms;
        is_dirty = false;
    }
    if (next_page < 8U)
    {
        if (!sh1106_page(platform_io_transmit, DISPLAY_ADDRESS, next_page, &framebuffer[(uint16_t)next_page * 128U]))
        {
            state = DISPLAY_FAILED;
            platform_io_log("FAULT OLED dashboard write\r\n");
            return;
        }
        next_page++;
        /* Page eight is the idle sentinel until the next refresh is due. */
    }
}

void display_set_event(const char *message)
{
    if (state != DISPLAY_DASHBOARD)
    {
        return;
    }
    (void)snprintf(event_message, sizeof(event_message), "%s", message == NULL ? "" : message);
    is_dirty = true;
}

bool display_is_splash(void)
{
    return (state == DISPLAY_STABILIZING) || (state == DISPLAY_SPLASH);
}
