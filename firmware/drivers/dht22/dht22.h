/* Portable DHT22 frame validation; owns no GPIO or retained caller buffers. */
#ifndef DHT22_H
#define DHT22_H
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Validates and decodes an MSB-first DHT22 five-byte frame.
 * @param[in] frame Borrowed five-byte frame, valid during this call.
 * @param[out] temperature_c Destination in Celsius, range -40 to 80.
 * @param[out] humidity_percent Destination in percent, range 0 to 100.
 * @return true for a valid checksum and in-range readings; false for null
 * arguments, checksum failure or out-of-range data. Outputs remain unchanged
 * on failure. Non-blocking; performs no hardware access.
 */
bool dht22_decode(const uint8_t frame[5], float *temperature_c,
                  float *humidity_percent);
#endif /* DHT22_H */
