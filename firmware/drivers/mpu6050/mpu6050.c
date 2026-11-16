/* MPU6050 register map revision 4.2 sections 4.2, 4.5, 4.17 and 4.28. */
#include "mpu6050.h"
#include <stddef.h>

enum
{
    MPU_SAMPLE_DIVIDER = 0x19,
    MPU_CONFIG = 0x1A,
    MPU_ACCEL_CONFIG = 0x1C,
    MPU_ACCEL_START = 0x3B,
    MPU_POWER = 0x6B,
    MPU_IDENTITY = 0x75,
    MPU_EXPECTED_ID = 0x68,
    MPU_RANGE_4G = 0x08,
    MPU_RANGE_MASK = 0x18,
    MPU_DIVIDER_100HZ = 9,
    MPU_DLPF_ENABLED = 3
};

/* Decode signed big-endian counts without implementation-defined narrowing. */
static float mpu6050_axis_g(const uint8_t *bytes)
{
    uint32_t raw = ((uint32_t)bytes[0] << 8U) | bytes[1];
    int32_t signed_raw = (raw >= 32768U) ? ((int32_t)raw - 65536) : (int32_t)raw;
    return (float)signed_raw / 8192.0F;
}

bool mpu6050_init(const mpu6050_io_t *io)
{
    uint8_t identity = 0U;
    uint8_t range = 0U;
    if ((io == NULL) || (io->read == NULL) || (io->write == NULL))
    {
        return false;
    }
    if (!io->read(io->address, MPU_IDENTITY, &identity, 1U) ||
        (identity != MPU_EXPECTED_ID))
    {
        return false;
    }
    /* Internal oscillator selected, SLEEP cleared; no reset delay is required. */
    if (!io->write(io->address, MPU_POWER, 0U) ||
        !io->write(io->address, MPU_CONFIG, MPU_DLPF_ENABLED) ||
        !io->write(io->address, MPU_SAMPLE_DIVIDER, MPU_DIVIDER_100HZ) ||
        !io->write(io->address, MPU_ACCEL_CONFIG, MPU_RANGE_4G))
    {
        return false;
    }
    return io->read(io->address, MPU_ACCEL_CONFIG, &range, 1U) &&
           ((range & MPU_RANGE_MASK) == MPU_RANGE_4G);
}

bool mpu6050_read(const mpu6050_io_t *io, mpu6050_sample_t *sample)
{
    uint8_t bytes[6];
    if ((io == NULL) || (io->read == NULL) || (sample == NULL))
    {
        return false;
    }
    if (!io->read(io->address, MPU_ACCEL_START, bytes, sizeof(bytes)))
    {
        return false;
    }
    sample->x_g = mpu6050_axis_g(&bytes[0]);
    sample->y_g = mpu6050_axis_g(&bytes[2]);
    sample->z_g = mpu6050_axis_g(&bytes[4]);
    return true;
}
