#ifndef LOGGER_H
#define LOGGER_H

/* Logging: LOGI / LOGW / LOGE / LOGD(TAG, fmt, ...). One call = one whole line, written under a
   lock so lines from different tasks never interleave. UART/VCP routing belongs in
   platforms/stm32/, not here. Do not call printf directly from a task: use these. */
#include <stdio.h>

void logger_write(char level, const char *tag, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

#define LOGI(tag, fmt, ...) logger_write('I', (tag), fmt, ##__VA_ARGS__)
#define LOGW(tag, fmt, ...) logger_write('W', (tag), fmt, ##__VA_ARGS__)
#define LOGE(tag, fmt, ...) logger_write('E', (tag), fmt, ##__VA_ARGS__)
#define LOGD(tag, fmt, ...) logger_write('D', (tag), fmt, ##__VA_ARGS__)

#endif /* LOGGER_H */
