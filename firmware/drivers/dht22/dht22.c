/* DHT22 checksum and engineering-unit conversion from Aosong section 6. */
#include "dht22.h"
#include <stddef.h>

bool dht22_decode(const uint8_t frame[5], float *temperature_c,
                  float *humidity_percent)
{
    const uint16_t sign_mask = 0x8000U;
    const uint16_t magnitude_mask = 0x7FFFU;
    uint16_t humidity_raw;
    uint16_t temperature_raw;
    uint8_t checksum;
    float decoded_temperature_c;
    float decoded_humidity_percent;

    if ((frame == NULL) || (temperature_c == NULL) || (humidity_percent == NULL))
    {
        return false;
    }
    checksum = (uint8_t)((uint16_t)frame[0] + frame[1] + frame[2] + frame[3]);
    if (checksum != frame[4])
    {
        return false;
    }
    humidity_raw = (uint16_t)(((uint16_t)frame[0] << 8U) | frame[1]);
    temperature_raw = (uint16_t)(((uint16_t)frame[2] << 8U) | frame[3]);
    decoded_humidity_percent = (float)humidity_raw / 10.0F;
    decoded_temperature_c = (float)(temperature_raw & magnitude_mask) / 10.0F;
    if ((temperature_raw & sign_mask) != 0U)
    {
        decoded_temperature_c = -decoded_temperature_c;
    }
    if ((decoded_humidity_percent > 100.0F) ||
        (decoded_temperature_c < -40.0F) || (decoded_temperature_c > 80.0F))
    {
        return false;
    }
    *temperature_c = decoded_temperature_c;
    *humidity_percent = decoded_humidity_percent;
    return true;
}
