/* Versioned serial telemetry formatting; owns no UART hardware. */
#ifndef TELEMETRY_H
#define TELEMETRY_H
#include <stdint.h>
#include "sensors.h"
#include "ride_events.h"
/** @brief Schedules telemetry with explicit scaled units and invalid sentinels.
 * @param[in] now_ms Wrapping millisecond timestamp; intervals below 2^31 ms.
 * @param[in] readings Borrowed snapshot, null suppresses transmission.
 * @param[in] event Current alert classification.
 * @note Main context only; one record per configured period. A full output queue
 * drops the record and increments the cumulative drop count; no catch-up burst.
 * Formatting is bounded and the platform copies the record synchronously. */
void telemetry_process(uint32_t now_ms, const sensors_snapshot_t *readings, ride_event_t event);
#endif /* TELEMETRY_H */
