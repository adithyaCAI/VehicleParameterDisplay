#include "logger.h"

#include <stdarg.h>
#include <string.h>

#define LOGGER_LINE_MAX 192U

/* The print mutex of ST's example (application/app_logger.c). Weak: absent -> no lock. */
extern void AppLoggerLock(void) __attribute__((weak));
extern void AppLoggerUnlock(void) __attribute__((weak));

void logger_write(char level, const char *tag, const char *fmt, ...)
{
    char line[LOGGER_LINE_MAX];
    int head = snprintf(line, sizeof(line), "%c %s: ", level, (tag != NULL) ? tag : "");
    size_t length;
    va_list args;

    if (head < 0)
    {
        return;
    }
    length = ((size_t)head < sizeof(line)) ? (size_t)head : (sizeof(line) - 1U);
    va_start(args, fmt);
    (void)vsnprintf(&line[length], sizeof(line) - length, fmt, args);
    va_end(args);
    length = strlen(line);
    if (length > (sizeof(line) - 2U))
    {
        length = sizeof(line) - 2U;
    }
    line[length] = '\n';
    line[length + 1U] = '\0';

    if (AppLoggerLock != NULL)
    {
        AppLoggerLock();
    }
    (void)fputs(line, stdout);
    if (AppLoggerUnlock != NULL)
    {
        AppLoggerUnlock();
    }
}
