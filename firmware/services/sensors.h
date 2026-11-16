/* Cooperative sensor scheduling and calibrated sample validity. */
#ifndef SENSORS_H
#define SENSORS_H
#include <stdbool.h>
#include <stdint.h>
#include "mpu6050.h"
typedef struct
{
    float temperature_c;
    float humidity_percent;
    mpu6050_sample_t acceleration;
    bool climate_valid;
    bool acceleration_valid;
    bool is_calibrated;
    uint32_t acceleration_time_ms;
} sensors_snapshot_t;
/** @brief Starts acquisition scheduling and one-second calibration.
 * @param[in] now_ms Current wrapping millisecond clock.
 * @pre Platform initialized, main context only. Calls bounded device I/O.
 * @note Reinitialization discards all previous validity and calibration. */
void sensors_init(uint32_t now_ms);
/** @brief Services due MPU and DHT reads without catch-up bursts.
 * @param[in] now_ms Wrapping millisecond timestamp; call at least every 1 ms.
 * @pre sensors_init completed; main-context single owner.
 * @note Missed slots are skipped; failed reads invalidate the corresponding data.
 * Bounded blocking I/O can consume up to one DHT transaction plus one MPU read. */
void sensors_process(uint32_t now_ms);
/** @brief Returns the main-context-owned latest sensor snapshot.
 * @return Borrowed read-only snapshot, overwritten by sensors_process.
 * @pre Main context only; inspect validity before consuming values. */
const sensors_snapshot_t *sensors_snapshot(void);
#endif /* SENSORS_H */
