/* Portable MPU6050 register driver with caller-supplied synchronous I/O. */
#ifndef MPU6050_H
#define MPU6050_H
#include <stdbool.h>
#include <stdint.h>
typedef struct
{
    bool (*read)(uint8_t address, uint8_t reg, uint8_t *bytes, uint16_t length);
    bool (*write)(uint8_t address, uint8_t reg, uint8_t value);
    uint8_t address;
} mpu6050_io_t;
typedef struct
{
    float x_g;
    float y_g;
    float z_g;
} mpu6050_sample_t;
/** @brief Checks identity, wakes the device and selects 100 Hz and +/-4 g.
 * @param[in] io Borrowed callbacks, non-null, main-context synchronous use only.
 * @return true when all transfers and range readback succeed; false otherwise.
 * @note Bounded by callback timeouts. Does not retain io. */
bool mpu6050_init(const mpu6050_io_t *io);
/** @brief Acquires a coherent six-byte acceleration block in g.
 * @param[in] io Borrowed initialized device callbacks.
 * @param[out] sample Updated only on successful acquisition.
 * @return true on success; false on invalid arguments or transfer failure. */
bool mpu6050_read(const mpu6050_io_t *io, mpu6050_sample_t *sample);
#endif /* MPU6050_H */
