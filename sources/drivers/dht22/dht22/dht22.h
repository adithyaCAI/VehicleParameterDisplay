#ifndef DHT22_H
#define DHT22_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DHT22 / AM2302 single-wire driver.
 *
 * Porting notes:
 * - Implement data_drive_low() to pull DATA low (active level: low).
 * - Implement data_release() to put DATA in high-impedance/input mode; do not
 *   actively drive it high. A physical pull-up on DATA is required for idle
 *   high and sensor signaling. The datasheet does not specify pull-up
 *   resistance or a logic-high threshold.
 * - Implement data_read() to sample the DATA line and report its logic level.
 * - Implement delay_us() with timing accuracy adequate for microsecond pulse
 *   measurement, and delay_ms() for the power-up and measurement-interval waits.
 * - Every callback receives ctx and returns a dht22_status_t. The driver uses
 *   no platform SDK and does not access hardware outside these callbacks.
 *
 * The handle storage below is public only so callers can allocate it without
 * dynamic memory. Treat its contents as private; initialize it only through
 * dht22_init() and do not copy a live handle.
 */

typedef enum {
    DHT22_STATUS_OK = 0,
    DHT22_STATUS_INVALID_ARGUMENT,
    DHT22_STATUS_COMMUNICATION_ERROR,
    DHT22_STATUS_TIMEOUT,
    DHT22_STATUS_UNSUPPORTED,
    DHT22_STATUS_DEVICE_IDENTITY_MISMATCH,
    DHT22_STATUS_CHECKSUM_ERROR,
    DHT22_STATUS_NOT_INITIALIZED
} dht22_status_t;

typedef struct {
    void *ctx;

    /* DATA's only driven state is low (active); release means high impedance. */
    dht22_status_t (*data_drive_low)(void *ctx);
    dht22_status_t (*data_release)(void *ctx);
    dht22_status_t (*data_read)(void *ctx, bool *is_high);

    dht22_status_t (*delay_us)(void *ctx, uint32_t duration_us);
    dht22_status_t (*delay_ms)(void *ctx, uint32_t duration_ms);
} dht22_io_t;

/* Protocol and timing constants taken from the supplied component spec. */
#define DHT22_FRAME_BITS                    40u
#define DHT22_FRAME_BYTES                    5u
#define DHT22_START_LOW_MIN_US            1000u
#define DHT22_RESPONSE_WAIT_US               30u
#define DHT22_RESPONSE_PULSE_US              80u
#define DHT22_BIT_LOW_US                     50u
#define DHT22_BIT_ZERO_HIGH_US               27u
#define DHT22_BIT_ONE_HIGH_US                70u
#define DHT22_POWER_ON_DELAY_MS             1000u
#define DHT22_MIN_MEASUREMENT_INTERVAL_MS  2001u

/*
 * Opaque caller-allocated handle storage. Do not inspect or modify _private.
 * The union members provide alignment for the driver's private state.
 */
typedef union {
    long double _align_long_double;
    void *_align_pointer;
    uint64_t _align_u64;
    uint8_t _private[128];
} dht22_handle_t;

typedef struct {
    /* Humidity in 0.1 %RH units: 523 means 52.3 %RH. */
    uint16_t humidity_tenths_percent;

    /* Temperature in 0.1 degC units: -125 means -12.5 degC. */
    int32_t temperature_tenths_celsius;
} dht22_measurement_t;

/*
 * Initialize or reinitialize a caller-allocated handle. This releases DATA
 * and waits at least one second before allowing communication, as required
 * after power is applied. Reinitializing replaces all cached state.
 */
dht22_status_t dht22_init(dht22_handle_t *handle, const dht22_io_t *io);

/* Deinitialize a handle and clear its cached state. */
dht22_status_t dht22_deinit(dht22_handle_t *handle);

/*
 * Read and checksum-validate one 40-bit sensor frame. The driver starts a
 * transaction by holding DATA low for at least 1 ms. Subsequent attempts are
 * delayed by 2001 ms after the preceding attempt was started, enforcing the
 * datasheet's requirement that the collecting period be greater than 2 s.
 * The result is written only when the complete frame and checksum are valid.
 *
 * Temperature decoding uses the common convention in which bit 15 is a sign
 * bit and bits 14..0 are the magnitude in 0.1 degC units. The supplied
 * datasheet does not specify this negative-temperature encoding; confirm it
 * for the specific device variant before relying on negative readings.
 */
dht22_status_t dht22_read_measurement(dht22_handle_t *handle,
                                      dht22_measurement_t *measurement);

#ifdef __cplusplus
}
#endif

#endif /* DHT22_H */
