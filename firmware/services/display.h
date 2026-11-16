/* Portable startup/dashboard renderer; owns the display framebuffer. */
#ifndef DISPLAY_H
#define DISPLAY_H
#include <stdbool.h>
#include <stdint.h>
#include "sensors.h"
/** @brief Initializes SH1106 and clears panel RAM before stabilization.
 * @param[in] now_ms Millisecond timestamp, used for nonblocking startup.
 * @return true on initialization and clearing success, false on bus error.
 * @pre Platform initialized; main context only. Bounded synchronous I2C. */
bool display_init(uint32_t now_ms);
/** @brief Advances startup and one-page-at-a-time dashboard transfers.
 * @param[in] now_ms Wrapping millisecond timestamp.
 * @param[in] readings Borrowed snapshot valid during call, or null to skip.
 * @pre display_init succeeded. Main context only.
 * @note Splash alerts are suppressed; failed transfers invalidate display health. */
void display_process(uint32_t now_ms, const sensors_snapshot_t *readings);
/** @brief Sets or clears a dashboard event message without hiding climate data.
 * @param[in] message Borrowed null-terminated event text; null clears.
 * Up to 42 characters copied. Ignored during splash.
 * @note Main context only; rendered on next display_process call. */
void display_set_event(const char *message);
/** @brief Reports whether startup excludes dashboard alerts.
 * @return true during stabilization and the logo splash, false afterwards. */
bool display_is_splash(void);
#endif /* DISPLAY_H */
