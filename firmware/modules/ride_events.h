/* Portable ride-event history, priority and alert timing; no hardware ownership. */
#ifndef RIDE_EVENTS_H
#define RIDE_EVENTS_H
#include <stdbool.h>
#include <stdint.h>
typedef enum { RIDE_NONE, RIDE_SPEEDING, RIDE_TURN, RIDE_BUMP, RIDE_BRAKING } ride_event_t;
typedef struct
{
    float x_g;
    float y_g;
    float z_g;
    uint32_t time_ms;
} ride_sample_t;
typedef struct
{
    ride_sample_t history[16];
    uint8_t count;
    uint32_t last_event_ms[5];
    bool active[5];
    uint32_t blink_start_ms;
    bool was_alerting;
    bool vehicle_mode;
} ride_events_t;
/** @brief Clears history and alerts and selects demo or vehicle thresholds.
 * @param[out] state Caller-owned state; null is ignored.
 * @param[in] vehicle_mode True selects vehicle thresholds; false selects demo.
 * @note Nonblocking; call before any other event API. */
void ride_events_init(ride_events_t *state, bool vehicle_mode);
/** @brief Classifies changes against all retained samples no older than 100 ms.
 * @param[in,out] state Initialized caller-owned state; null is ignored.
 * @param[in] sample Borrowed sample; null clears history without creating events.
 * @note Duplicate timestamps are ignored. Invalid/nonfinite samples clear history.
 * History capacity supports the specified 100 Hz stream, with no catch-up bursts. */
void ride_events_sample(ride_events_t *state, const ride_sample_t *sample);
/** @brief Expires held events and selects the highest-priority message.
 * @param[in,out] state Initialized state, or null for no event.
 * @param[in] now_ms Wrapping milliseconds; call gaps must remain below 2^31 ms.
 * @param[out] led_on Optional logical LED result, off if no active event.
 * @return Highest priority event, or RIDE_NONE. No I/O; nonblocking.
 * @note Each event expires at 2000 ms since its last detection. Blink is ON
 * for the first 500 ms then OFF for 500 ms; repeats do not reset its phase. */
ride_event_t ride_events_update(ride_events_t *state, uint32_t now_ms, bool *led_on);
/** @brief Gets immutable display text for an event.
 * @param[in] event Event classification.
 * @return Static string; empty for no event or invalid enumeration. */
const char *ride_events_message(ride_event_t event);
#endif
