/* Sensor service: timed reads, flat/still qualification and invalidation. */
#include "sensors.h"
#include "dht22.h"
#include "platform_io.h"
#include "app_config.h"
#include <math.h>
#include <stdio.h>

static const mpu6050_io_t imu_io = {platform_io_read, platform_io_write, APP_MPU_ADDRESS};
static sensors_snapshot_t snapshot;
static mpu6050_sample_t baseline;
static mpu6050_sample_t sums;
static mpu6050_sample_t offsets;
static uint32_t calibration_start_ms;
static uint32_t calibration_count;
static uint32_t last_imu_ms;
static uint32_t last_dht_ms;
static uint32_t startup_ms;
static uint32_t last_report_ms;
static bool is_imu_ready;

/* Consecutive valid, approximately +Z-up samples qualify the offset estimate. */
static void sensors_calibrate(uint32_t now_ms, const mpu6050_sample_t *sample)
{
    bool is_flat = (fabsf(sample->x_g) <= APP_CALIBRATION_FLAT_G) &&
                   (fabsf(sample->y_g) <= APP_CALIBRATION_FLAT_G) &&
                   (fabsf(sample->z_g - 1.0F) <= APP_CALIBRATION_FLAT_G);
    bool is_still = (calibration_count == 0U) ||
        ((fabsf(sample->x_g - baseline.x_g) <= APP_CALIBRATION_STILL_G) &&
         (fabsf(sample->y_g - baseline.y_g) <= APP_CALIBRATION_STILL_G) &&
         (fabsf(sample->z_g - baseline.z_g) <= APP_CALIBRATION_STILL_G));
    if (!is_flat || !is_still)
    {
        calibration_count = 0U;
        return;
    }
    if (calibration_count == 0U)
    {
        calibration_start_ms = now_ms;
        baseline = *sample;
        sums = (mpu6050_sample_t){0};
    }
    sums.x_g += sample->x_g;
    sums.y_g += sample->y_g;
    sums.z_g += sample->z_g;
    calibration_count++;
    if ((uint32_t)(now_ms - calibration_start_ms) >= APP_CALIBRATION_MS)
    {
        offsets.x_g = sums.x_g / (float)calibration_count;
        offsets.y_g = sums.y_g / (float)calibration_count;
        offsets.z_g = (sums.z_g / (float)calibration_count) - 1.0F;
        snapshot.is_calibrated = true;
        platform_io_log("MPU calibration complete\r\n");
    }
}

void sensors_init(uint32_t now_ms)
{
    snapshot = (sensors_snapshot_t){0};
    snapshot.temperature_c = NAN;
    snapshot.humidity_percent = NAN;
    snapshot.acceleration = (mpu6050_sample_t){NAN, NAN, NAN};
    calibration_count = 0U;
    last_imu_ms = now_ms;
    startup_ms = now_ms;
    last_dht_ms = now_ms + APP_DHT_STARTUP_MS - APP_DHT_PERIOD_MS;
    last_report_ms = now_ms;
    is_imu_ready = mpu6050_init(&imu_io);
    platform_io_log(is_imu_ready ? "MPU init OK range=4g rate=100Hz\r\n" : "FAULT MPU init/readback\r\n");
}

void sensors_process(uint32_t now_ms)
{
    if ((uint32_t)(now_ms - last_imu_ms) >= APP_MPU_PERIOD_MS)
    {
        mpu6050_sample_t sample;
        last_imu_ms += ((uint32_t)(now_ms - last_imu_ms) / APP_MPU_PERIOD_MS) * APP_MPU_PERIOD_MS;
        snapshot.acceleration_valid = false;
        if (is_imu_ready && mpu6050_read(&imu_io, &sample))
        {
            if (!snapshot.is_calibrated)
            {
                sensors_calibrate(now_ms, &sample);
            }
            if (snapshot.is_calibrated)
            {
                snapshot.acceleration.x_g = sample.x_g - offsets.x_g;
                snapshot.acceleration.y_g = sample.y_g - offsets.y_g;
                snapshot.acceleration.z_g = sample.z_g - offsets.z_g;
                snapshot.acceleration_time_ms = now_ms;
                snapshot.acceleration_valid = true;
            }
        }
        else
        {
            calibration_count = 0U;
            snapshot.acceleration = (mpu6050_sample_t){NAN, NAN, NAN};
        }
    }
    if (((uint32_t)(now_ms - startup_ms) >= APP_DHT_STARTUP_MS) &&
        ((uint32_t)(now_ms - last_dht_ms) >= APP_DHT_PERIOD_MS))
    {
        uint8_t frame[5];
        last_dht_ms += ((uint32_t)(now_ms - last_dht_ms) / APP_DHT_PERIOD_MS) * APP_DHT_PERIOD_MS;
        snapshot.climate_valid = platform_io_dht_frame(frame) &&
            dht22_decode(frame, &snapshot.temperature_c, &snapshot.humidity_percent);
        if (!snapshot.climate_valid)
        {
            snapshot.temperature_c = NAN;
            snapshot.humidity_percent = NAN;
        }
        platform_io_log(snapshot.climate_valid ? "DHT frame/checksum OK\r\n" : "FAULT DHT response/checksum\r\n");
    }
    if ((uint32_t)(now_ms - last_report_ms) >= 2000U)
    {
        char report[128];
        last_report_ms = now_ms;
        (void)snprintf(report, sizeof(report), "SENSORS climate=%u imu=%u calibrated=%u\r\n",
            (unsigned int)snapshot.climate_valid, (unsigned int)snapshot.acceleration_valid,
            (unsigned int)snapshot.is_calibrated);
        platform_io_log(report);
    }
}

const sensors_snapshot_t *sensors_snapshot(void)
{
    return &snapshot;
}
