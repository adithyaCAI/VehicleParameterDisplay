#include "dht22.h"

#include <string.h>

/* Bounded polling policy: wider than the approximate pulses in the datasheet. */
#define DHT22_PHASE_TIMEOUT_US  120u
#define DHT22_POLL_STEP_US        1u
#define DHT22_BIT_THRESHOLD_US  (DHT22_BIT_ZERO_HIGH_US + \
                                 ((DHT22_BIT_ONE_HIGH_US - DHT22_BIT_ZERO_HIGH_US) / 2u))
#define DHT22_PRIVATE_MAGIC UINT32_C(0x44323248)

typedef struct {
    uint32_t magic;
    dht22_io_t io;
    bool measurement_attempted;
} dht22_private_t;

typedef char dht22_private_fits_storage[
    (sizeof(dht22_private_t) <= sizeof(((dht22_handle_t *)0)->_private)) ? 1 : -1];

static dht22_private_t *dht22_private(dht22_handle_t *handle)
{
    return (dht22_private_t *)(void *)handle->_private;
}

static bool dht22_io_is_valid(const dht22_io_t *io)
{
    return (io != NULL) &&
           (io->data_drive_low != NULL) &&
           (io->data_release != NULL) &&
           (io->data_read != NULL) &&
           (io->delay_us != NULL) &&
           (io->delay_ms != NULL);
}

static dht22_status_t dht22_read_level(dht22_private_t *state, bool *is_high)
{
    return state->io.data_read(state->io.ctx, is_high);
}

/* Wait for a requested logic level, delaying in bounded 1 us increments. */
static dht22_status_t dht22_wait_level(dht22_private_t *state,
                                       bool target_high,
                                       uint32_t timeout_us)
{
    uint32_t elapsed_us = 0u;
    bool is_high = false;
    dht22_status_t status;

    for (;;) {
        status = dht22_read_level(state, &is_high);
        if (status != DHT22_STATUS_OK) {
            return status;
        }
        if (is_high == target_high) {
            return DHT22_STATUS_OK;
        }
        if (elapsed_us >= timeout_us) {
            return DHT22_STATUS_TIMEOUT;
        }
        status = state->io.delay_us(state->io.ctx, DHT22_POLL_STEP_US);
        if (status != DHT22_STATUS_OK) {
            return status;
        }
        elapsed_us += DHT22_POLL_STEP_US;
    }
}

/* Wait for a high pulse to end and return its measured width in microseconds. */
static dht22_status_t dht22_measure_high_pulse(dht22_private_t *state,
                                               uint32_t *pulse_width_us)
{
    uint32_t elapsed_us = 0u;
    bool is_high = true;
    dht22_status_t status;

    if (pulse_width_us == NULL) {
        return DHT22_STATUS_INVALID_ARGUMENT;
    }

    while (is_high) {
        if (elapsed_us >= DHT22_PHASE_TIMEOUT_US) {
            return DHT22_STATUS_TIMEOUT;
        }
        status = state->io.delay_us(state->io.ctx, DHT22_POLL_STEP_US);
        if (status != DHT22_STATUS_OK) {
            return status;
        }
        elapsed_us += DHT22_POLL_STEP_US;
        status = dht22_read_level(state, &is_high);
        if (status != DHT22_STATUS_OK) {
            return status;
        }
    }

    *pulse_width_us = elapsed_us;
    return DHT22_STATUS_OK;
}

static dht22_status_t dht22_release_line(dht22_private_t *state,
                                         dht22_status_t operation_status)
{
    dht22_status_t release_status = state->io.data_release(state->io.ctx);

    if (operation_status != DHT22_STATUS_OK) {
        return operation_status;
    }
    return release_status;
}

dht22_status_t dht22_init(dht22_handle_t *handle, const dht22_io_t *io)
{
    dht22_private_t *state;
    dht22_status_t status;

    if ((handle == NULL) || !dht22_io_is_valid(io)) {
        return DHT22_STATUS_INVALID_ARGUMENT;
    }

    memset(handle, 0, sizeof(*handle));
    state = dht22_private(handle);
    state->io = *io;

    status = state->io.data_release(state->io.ctx);
    if (status != DHT22_STATUS_OK) {
        memset(handle, 0, sizeof(*handle));
        return status;
    }

    status = state->io.delay_ms(state->io.ctx, DHT22_POWER_ON_DELAY_MS);
    if (status != DHT22_STATUS_OK) {
        memset(handle, 0, sizeof(*handle));
        return status;
    }

    state->measurement_attempted = false;
    state->magic = DHT22_PRIVATE_MAGIC;
    return DHT22_STATUS_OK;
}

dht22_status_t dht22_deinit(dht22_handle_t *handle)
{
    dht22_private_t *state;
    dht22_status_t status;

    if (handle == NULL) {
        return DHT22_STATUS_INVALID_ARGUMENT;
    }

    state = dht22_private(handle);
    if (state->magic != DHT22_PRIVATE_MAGIC) {
        memset(handle, 0, sizeof(*handle));
        return DHT22_STATUS_NOT_INITIALIZED;
    }

    status = state->io.data_release(state->io.ctx);
    memset(handle, 0, sizeof(*handle));
    return status;
}

dht22_status_t dht22_read_measurement(dht22_handle_t *handle,
                                      dht22_measurement_t *measurement)
{
    dht22_private_t *state;
    uint8_t frame[DHT22_FRAME_BYTES];
    uint8_t checksum;
    uint16_t humidity_raw;
    uint16_t temperature_raw;
    uint32_t pulse_width_us;
    uint32_t bit_index;
    dht22_status_t status;

    if ((handle == NULL) || (measurement == NULL)) {
        return DHT22_STATUS_INVALID_ARGUMENT;
    }

    state = dht22_private(handle);
    if (state->magic != DHT22_PRIVATE_MAGIC) {
        return DHT22_STATUS_NOT_INITIALIZED;
    }

    if (state->measurement_attempted) {
        status = state->io.delay_ms(state->io.ctx,
                                    DHT22_MIN_MEASUREMENT_INTERVAL_MS);
        if (status != DHT22_STATUS_OK) {
            return status;
        }
    }

    status = state->io.data_release(state->io.ctx);
    if (status != DHT22_STATUS_OK) {
        return status;
    }
    status = state->io.data_drive_low(state->io.ctx);
    if (status != DHT22_STATUS_OK) {
        return dht22_release_line(state, status);
    }

    status = state->io.delay_us(state->io.ctx, DHT22_START_LOW_MIN_US);
    if (status != DHT22_STATUS_OK) {
        state->measurement_attempted = true;
        return dht22_release_line(state, status);
    }

    status = state->io.data_release(state->io.ctx);
    state->measurement_attempted = true;
    if (status != DHT22_STATUS_OK) {
        return status;
    }

    status = state->io.delay_us(state->io.ctx, DHT22_RESPONSE_WAIT_US);
    if (status != DHT22_STATUS_OK) {
        return dht22_release_line(state, status);
    }

    /* Sensor response: approximately 80 us low followed by 80 us high. */
    status = dht22_wait_level(state, false, DHT22_PHASE_TIMEOUT_US);
    if (status == DHT22_STATUS_OK) {
        status = dht22_wait_level(state, true, DHT22_PHASE_TIMEOUT_US);
    }
    if (status == DHT22_STATUS_OK) {
        status = dht22_wait_level(state, false, DHT22_PHASE_TIMEOUT_US);
    }
    if (status != DHT22_STATUS_OK) {
        return dht22_release_line(state, status);
    }

    memset(frame, 0, sizeof(frame));
    for (bit_index = 0u; bit_index < DHT22_FRAME_BITS; ++bit_index) {
        /* Each bit starts low; its following high-pulse width encodes the bit. */
        status = dht22_wait_level(state, true, DHT22_PHASE_TIMEOUT_US);
        if (status != DHT22_STATUS_OK) {
            return dht22_release_line(state, status);
        }
        status = dht22_measure_high_pulse(state, &pulse_width_us);
        if (status != DHT22_STATUS_OK) {
            return dht22_release_line(state, status);
        }

        frame[bit_index / 8u] = (uint8_t)(frame[bit_index / 8u] << 1u);
        if (pulse_width_us > DHT22_BIT_THRESHOLD_US) {
            frame[bit_index / 8u] = (uint8_t)(frame[bit_index / 8u] | 1u);
        }
    }

    status = state->io.data_release(state->io.ctx);
    if (status != DHT22_STATUS_OK) {
        return status;
    }

    checksum = (uint8_t)(frame[0] + frame[1] + frame[2] + frame[3]);
    if (checksum != frame[4]) {
        return DHT22_STATUS_CHECKSUM_ERROR;
    }

    humidity_raw = (uint16_t)(((uint16_t)frame[0] << 8u) | (uint16_t)frame[1]);
    temperature_raw = (uint16_t)(((uint16_t)frame[2] << 8u) | (uint16_t)frame[3]);

    {
        dht22_measurement_t decoded;
        uint16_t magnitude = (uint16_t)(temperature_raw & UINT16_C(0x7FFF));

        decoded.humidity_tenths_percent = humidity_raw;
        if ((temperature_raw & UINT16_C(0x8000)) != 0u) {
            decoded.temperature_tenths_celsius = -(int32_t)magnitude;
        } else {
            decoded.temperature_tenths_celsius = (int32_t)magnitude;
        }
        *measurement = decoded;
    }

    return DHT22_STATUS_OK;
}
