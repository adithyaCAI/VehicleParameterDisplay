/* Heap-free output for the formats used by the pinned CoreMark sources.
 * Only reporting uses this code; the timed workload is unchanged. Output is
 * streamed in bounded chunks, including arbitrarily long compiler flag strings.
 */
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

/* Each port implements a raw transport, bypassing libc's FILE/float machinery. */
extern int fg_coremark_write(const char *data, unsigned length);

typedef struct {
    char bytes[64];
    unsigned used;
    int count;
    int failed;
} output;

static void flush(output *out)
{
    if (out->used && !out->failed &&
        fg_coremark_write(out->bytes, out->used) != (int)out->used)
        out->failed = 1;
    out->used = 0;
}

static void put(output *out, char ch)
{
    if (out->failed) return;
    out->bytes[out->used++] = ch;
    ++out->count;
    if (out->used == sizeof(out->bytes)) flush(out);
}

static void text(output *out, const char *s)
{
    if (!s) s = "(null)";
    while (*s) put(out, *s++);
}

static void number(output *out, uint64_t value, unsigned base, unsigned width, char padding)
{
    char digits[32];
    unsigned n = 0;
    do {
        digits[n++] = "0123456789abcdef"[value % base];
        value /= base;
    } while (value);
    while (width > n) { put(out, padding); --width; }
    while (n) put(out, digits[--n]);
}

static void decimal(output *out, double value, unsigned precision)
{
    uint64_t whole;
    unsigned scale = 1, fractional, i;
    if (value < 0) { put(out, '-'); value = -value; }
    /* Explicit domain covers all benchmark times/scores and avoids undefined
     * conversions for NaN, infinity or an out-of-range double. */
    if (!(value >= 0 && value <= 1000000000000.0)) {
        text(out, "FIRMGEN_COREMARK_RUNTIME_ERROR: invalid output value");
        return;
    }
    for (i = 0; i < precision; ++i) scale *= 10;
    whole = (uint64_t)value;
    fractional = (unsigned)((value - (double)whole) * scale + 0.5);
    if (fractional >= scale) { ++whole; fractional = 0; }
    number(out, whole, 10, 0, ' ');
    if (precision) { put(out, '.'); number(out, fractional, 10, precision, '0'); }
}

int fg_coremark_printf(const char *format, ...)
{
    output out = {{0}, 0, 0, 0};
    va_list args;
    va_start(args, format);
    while (*format) {
        unsigned width = 0, precision = 6;
        int is_long = 0;
        char padding = ' ';
        if (*format != '%') { put(&out, *format++); continue; }
        ++format;
        if (*format == '0') { padding = '0'; ++format; }
        while (*format >= '0' && *format <= '9' && width <= 64)
            width = width * 10 + (unsigned)(*format++ - '0');
        if (*format == '.') {
            ++format; precision = 0;
            while (*format >= '0' && *format <= '9' && precision <= 9)
                precision = precision * 10 + (unsigned)(*format++ - '0');
        }
        if (*format == 'l') { is_long = 1; ++format; }
        if (width > 64 || precision > 9 || !*format) goto unsupported;
        switch (*format++) {
        case '%': put(&out, '%'); break;
        case 's': text(&out, va_arg(args, const char *)); break;
        case 'c': put(&out, (char)va_arg(args, int)); break;
        case 'd':
        case 'i': {
            long value = is_long ? va_arg(args, long) : va_arg(args, int);
            uint64_t magnitude;
            if (value < 0) {
                put(&out, '-'); if (width) --width;
                magnitude = (uint64_t)(-(value + 1)) + 1;
            } else magnitude = (uint64_t)value;
            number(&out, magnitude, 10, width, padding);
            break;
        }
        case 'u':
        case 'x': {
            unsigned base = format[-1] == 'x' ? 16 : 10;
            uint64_t value = is_long ? va_arg(args, unsigned long) : va_arg(args, unsigned int);
            number(&out, value, base, width, padding);
            break;
        }
        case 'f': decimal(&out, va_arg(args, double), precision); break;
        default:
unsupported:
            text(&out, "FIRMGEN_COREMARK_RUNTIME_ERROR: unsupported output format\n");
            flush(&out); va_end(args); return -1;
        }
    }
    flush(&out);
    va_end(args);
    return out.failed ? -1 : out.count;
}
