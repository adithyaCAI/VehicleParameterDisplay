/* Vendor-neutral main-context I/O; platform owns bus and timer handles. */
#ifndef PLATFORM_IO_H
#define PLATFORM_IO_H
#include <stdbool.h>
#include <stdint.h>
/** @brief Initializes shared I2C, microsecond timer and diagnostic UART once.
 * @return true on success, false on any initialization failure.
 * @pre Vendor clock initialization completed. Main context only. */
bool platform_io_init(void);
/** @brief Reads the wrapping 32-bit HAL millisecond clock.
 * @return Milliseconds since reset; elapsed intervals must be below 2^31 ms. */
uint32_t platform_io_now_ms(void);
/** @brief Reads an I2C device register block with exclusive bus ownership.
 * @param[in] address Seven-bit device address, 8 through 119.
 * @param[in] reg Eight-bit register address.
 * @param[out] bytes Borrowed writable destination, valid during call.
 * @param[in] length Byte count, 1 through 128.
 * @return true on completion, false on invalid argument, busy bus or transfer failure.
 * @pre Initialized; main context only. Blocks for a bounded platform timeout. */
bool platform_io_read(uint8_t address, uint8_t reg, uint8_t *bytes, uint16_t length);
/** @brief Writes one register with exclusive bus ownership.
 * @param[in] address Seven-bit device address.
 * @param[in] reg Register address.
 * @param[in] value Register value.
 * @return true on completion; false on busy bus, invalid address or transfer failure.
 * @pre Initialized; main context only. Bounded blocking call. */
bool platform_io_write(uint8_t address, uint8_t reg, uint8_t value);
/** @brief Writes display command/data bytes on the same serialized I2C bus.
 * @param[in] address Seven-bit display address.
 * @param[in,out] bytes Borrowed transfer buffer; HAL does not retain it.
 * @param[in] length Byte count, 1 through 128.
 * @return true on completion, false on invalid arguments, busy or I/O failure.
 * @pre Initialized; main context only. Bounded blocking call. */
bool platform_io_transmit(uint8_t address, uint8_t *bytes, uint16_t length);
/** @brief Captures a DHT22 response on the configured data line.
 * @param[out] frame Borrowed five-byte destination; use only on success.
 * @return true when all forty pulse bits arrived; false on timeout or null.
 * @pre Sensor powered for one second; initialized; main context only.
 * @note Blocks at most 8 ms, leaving interrupts enabled. */
bool platform_io_dht_frame(uint8_t frame[5]);
/** @brief Sweeps addresses 0x08..0x77 and reports ACKs and line levels.
 * @pre Initialized, main context only; run before periodic acquisition starts.
 * @note Bounded diagnostic sweep can take hundreds of milliseconds. */
void platform_io_scan(void);
/** @brief Sends a bounded diagnostic line over ST-Link VCP UART.
 * @param[in] message Borrowed null-terminated text; at most 160 characters sent.
 * @pre Initialized; main context only. Drops output after bounded UART timeout. */
void platform_io_log(const char *message);
/** @brief Drives the board user LED to its logical on/off state.
 * @param[in] on True lights LD4; false turns it off.
 * @pre platform_io_init succeeded; main context only. Nonblocking. */
void platform_io_led(bool on);
/** @brief Queues one telemetry record for polled, nonblocking UART transmission.
 * @param[in] record Borrowed text copied during call; maximum 255 bytes.
 * @return true if copied, false if busy, null or oversized; caller may drop it.
 * @pre Initialized, main context only. */
bool platform_io_telemetry(const char *record);
/** @brief Sends at most one queued UART byte if hardware is ready.
 * @pre Initialized, main context only; nonblocking. */
void platform_io_poll(void);
#endif /* PLATFORM_IO_H */
