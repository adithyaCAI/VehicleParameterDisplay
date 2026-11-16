/* Ride dynamics: bounded sample window and independently expiring alerts. */
#include "ride_events.h"
#include "config.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

void ride_events_init(ride_events_t *state, bool vehicle_mode)
{
    if (state != NULL)
    {
        memset(state, 0, sizeof(*state));
        state->vehicle_mode = vehicle_mode;
    }
}

void ride_events_sample(ride_events_t *state, const ride_sample_t *sample)
{
    uint8_t retained = 0U;
    float z_limit;
    float x_limit;
    float positive_y_limit;
    float negative_y_limit;
    if (state == NULL)
    {
        return;
    }
    if ((sample == NULL) || !isfinite(sample->x_g) ||
        !isfinite(sample->y_g) || !isfinite(sample->z_g))
    {
        state->count = 0U;
        return;
    }
    if (state->count > 0U)
    {
        uint32_t elapsed_ms = sample->time_ms - state->history[state->count - 1U].time_ms;
        if (elapsed_ms == 0U)
        {
            return;
        }
        if (elapsed_ms > 0x7FFFFFFFU)
        {
            state->count = 0U;
        }
    }
    z_limit = state->vehicle_mode ? VEHICLE_EVENT_Z_THRESHOLD_G : EVENT_Z_THRESHOLD_G;
    x_limit = state->vehicle_mode ? VEHICLE_EVENT_X_THRESHOLD_G : EVENT_X_THRESHOLD_G;
    positive_y_limit = state->vehicle_mode ? VEHICLE_EVENT_POS_Y_THRESHOLD_G : EVENT_POS_Y_THRESHOLD_G;
    negative_y_limit = state->vehicle_mode ? VEHICLE_EVENT_NEG_Y_THRESHOLD_G : EVENT_NEG_Y_THRESHOLD_G;
    for (uint8_t index = 0U; index < state->count; index++)
    {
        ride_sample_t baseline = state->history[index];
        if ((uint32_t)(sample->time_ms - baseline.time_ms) <= SUDDEN_WINDOW_MS)
        {
            bool detected[5] = {false,
                (sample->y_g - baseline.y_g) >= positive_y_limit,
                fabsf(sample->x_g - baseline.x_g) >= x_limit,
                fabsf(sample->z_g - baseline.z_g) >= z_limit,
                (sample->y_g - baseline.y_g) <= negative_y_limit};
            state->history[retained] = baseline;
            retained++;
            for (uint8_t event = 1U; event < 5U; event++)
            {
                if (detected[event])
                {
                    state->active[event] = true;
                    state->last_event_ms[event] = sample->time_ms;
                }
            }
        }
    }
    if (retained >= 16U)
    {
        memmove(state->history, &state->history[1], 15U * sizeof(state->history[0]));
        retained = 15U;
    }
    state->history[retained] = *sample;
    state->count = (uint8_t)(retained + 1U);
}

ride_event_t ride_events_update(ride_events_t *state, uint32_t now_ms, bool *led_on)
{
    ride_event_t selected = RIDE_NONE;
    if (led_on != NULL)
    {
        *led_on = false;
    }
    if (state == NULL)
    {
        return selected;
    }
    for (uint8_t event = 1U; event < 5U; event++)
    {
        if (state->active[event] &&
            ((uint32_t)(now_ms - state->last_event_ms[event]) >= EVENT_DISPLAY_MS))
        {
            state->active[event] = false;
        }
        if (state->active[event])
        {
            selected = (ride_event_t)event;
        }
    }
    if ((selected != RIDE_NONE) && !state->was_alerting)
    {
        state->blink_start_ms = now_ms;
    }
    state->was_alerting = selected != RIDE_NONE;
    if (led_on != NULL)
    {
        *led_on = state->was_alerting &&
            (((uint32_t)(now_ms - state->blink_start_ms) % APP_ALERT_CYCLE_MS) < APP_ALERT_ON_MS);
    }
    return selected;
}

const char *ride_events_message(ride_event_t event)
{
    switch (event)
    {
        case RIDE_SPEEDING: return "Speeding";
        case RIDE_TURN: return "Wide Turn";
        case RIDE_BUMP: return "Unseen SpeedBreaker";
        case RIDE_BRAKING: return "Sudden Braking";
        case RIDE_NONE: return "";
        default: return "";
    }
}
