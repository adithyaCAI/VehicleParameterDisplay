/* Portable SH1106 page writer; caller owns I2C transport and framebuffer. */
#ifndef SH1106_H
#define SH1106_H
#include <stdbool.h>
#include <stdint.h>
typedef bool (*sh1106_send_t)(uint8_t address, uint8_t *bytes, uint16_t length);
/** @brief Configures display off using module datasheet section 4.4.2.
 * @param[in] send Synchronous borrowed I2C callback, not retained.
 * @param[in] address Seven-bit module address.
 * @return true if commands transferred, false for null callback or I/O error. */
bool sh1106_init(sh1106_send_t send, uint8_t address);
/** @brief Writes one 128-column page with RAM column offset exactly two.
 * @param[in] send Synchronous callback, not retained.
 * @param[in] address Seven-bit address.
 * @param[in] page Page index 0..7.
 * @param[in] pixels Borrowed 128-byte page; not retained.
 * @return true on completion, false for invalid arguments or I/O error.
 * @note Uses two bounded 64-byte data chunks. */
bool sh1106_page(sh1106_send_t send, uint8_t address, uint8_t page, const uint8_t *pixels);
/** @brief Turns the initialized display on after stabilization.
 * @param[in] send Synchronous callback.
 * @param[in] address Seven-bit address.
 * @return true on transfer completion, false otherwise.
 * @pre Screen cleared and panel supply stabilized for 100 ms. */
bool sh1106_enable(sh1106_send_t send, uint8_t address);
#endif
