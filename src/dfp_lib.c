#include "dfp_lib.h"
#include "fmt_parser.h"
#include <stdint.h>

/// According to <https://en.cppreference.com/w/c/language/conversion>:
/// In a function call expression when the call is made to
/// 1) a function without a prototype (until C23)
/// 2) a variadic function, where the argument expression is one of the trailing arguments that are matched against the ellipsis parameter
/// Each argument of integer type undergoes integer promotion (see below),
/// and each argument of type float is implicitly converted to the type double.

/// Using a global variable to avoid `malloc` (to support more platforms)
static struct dfp default_dfp;

static int dfp_putc(struct dfp *self, int c);

static int dfp_print_int_with_width(struct dfp *self, unsigned long long value, int width) {
    /// The decimal string of 2**64 have 20 characters, so 32B is enough for any 64-bit integer.
    /// And integer won't be bigger than 64-bit in the near future.
#define INT_BUFFER_SIZE 32
    char buffer[INT_BUFFER_SIZE];
    int start, dry_width, i;

    if (value == 0 && width == 0) {
        dfp_putc(self, '0');
        return 1;
    }

    buffer[INT_BUFFER_SIZE - 1] = '\0';
    start = INT_BUFFER_SIZE - 2;

    /// `start` will never be 0 in this function since `long long` will never use up `buffer`.
    /// `start` is an empty pointer. It always points to the next position to store.
    while (value > 0) {
        buffer[start--] = value % 10 + '0';
        value /= 10;
    }

    dry_width = INT_BUFFER_SIZE - 2 - start;
    if (dry_width < width) {
        for (i = width - dry_width; i > 0; i--)
            buffer[start--] = '0';
    }

    self->puts(buffer + start + 1);
    return INT_BUFFER_SIZE - 2 - start;
}

static int dfp_print_int(struct dfp *self, unsigned long long value) {
    return dfp_print_int_with_width(self, value, 0);
}

static int dfp_print_int_signed(struct dfp *self, long long value) {
    int n = 0;

    if (value < 0) {
        value = -value;
        n += dfp_putc(self, '-');
    }

    n += dfp_print_int(self, value);
    return n;
}

#ifndef NO_FLOAT
/// Caution: This is a dirty implementation. Value bigger than 2**64 will be wrong.
static int dfp_print_float(struct dfp *self, double value) {
    long long tmp;
    int n = 0;

    tmp = (long long)value;
    n += dfp_print_int_signed(self, tmp);

    dfp_putc(self, '.');
    n += 1;

    tmp = (long long)((value - tmp) * 1000000);
    n += dfp_print_int(self, tmp);

    return n;
}
#endif

static int dfp_step(struct dfp *self, struct fmt_parser_chunk *chunk, int *error) {
    if (chunk->type == FMT_CHAR)
        return dfp_putc(self, chunk->c);
    if (chunk->type == FMT_PLACEHOLDER_C)
        return dfp_putc(self, va_arg(self->va, int));
    if (chunk->type == FMT_PLACEHOLDER_LLD)
        return dfp_print_int_signed(self, va_arg(self->va, long long));
    if (chunk->type == FMT_PLACEHOLDER_LD)
        return dfp_print_int_signed(self, va_arg(self->va, long));
    if (chunk->type == FMT_PLACEHOLDER_D)
        return dfp_print_int_signed(self, va_arg(self->va, int));
    if (chunk->type == FMT_PLACEHOLDER_LLU)
        return dfp_print_int(self, va_arg(self->va, unsigned long long));
    if (chunk->type == FMT_PLACEHOLDER_LU)
        return dfp_print_int(self, va_arg(self->va, unsigned long));
    if (chunk->type == FMT_PLACEHOLDER_U)
        return dfp_print_int(self, va_arg(self->va, unsigned int));
    if (chunk->type == FMT_PLACEHOLDER_P)
        return dfp_print_int(self, va_arg(self->va, uintptr_t));
#ifndef NO_FLOAT
    if (chunk->type == FMT_PLACEHOLDER_F)
        return dfp_print_float(self, va_arg(self->va, double));
#endif
    if (chunk->type == FMT_PLACEHOLDER_S)
        return self->puts(va_arg(self->va, const char *));

    *error = 1;
    return 0;
}

int dfp_vprintf(struct dfp *self, const char *fmt, va_list va) {
    struct fmt_parser parser;
    struct fmt_parser_chunk chunk;
    int n = 0;
    int error = 0;

    if (fmt_parser_init(&parser, fmt))
        return -1;

    va_copy(self->va, va);
    while (!fmt_parser_step(&parser, &chunk) && !error)
        n += dfp_step(self, &chunk, &error);

    /// dfp error (caused by `dfp_step`) or parser error (unfinished fmt)
    if (error || !fmt_parser_finished(&parser))
        n = -1;

    return n;
}

/// Unlike `putc` in standard C, `DFP_putc` returns 1
static int dfp_putc(struct dfp *self, int c) {
    char buf[2] = {0};
    buf[0] = c;
    self->puts(buf);
    return 1;
}

int dfp_init(struct dfp *self, dfp_puts_fn puts) {
    self->puts = puts;
    return 0;
}

int dfp_replace_puts(struct dfp *self, dfp_puts_fn puts) {
    self->puts = puts;
    return 0;
}

int DFP_PRINTF_INIT(dfp_puts_fn puts) {
    return dfp_init(&default_dfp, puts);
}

/// This is the function to be used by user.
int DFP_PRINTF(const char *fmt, ...) {
    va_list va;
    int ret;

    va_start(va, fmt);
    ret = dfp_vprintf(&default_dfp, fmt, va);
    va_end(va);

    return ret;
}
