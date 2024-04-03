#include "dfp_lib.h"
#include "fmt_parser.h"
#include <stdint.h>

/// According to <https://en.cppreference.com/w/c/language/conversion>:
/// Each argument of integer type undergoes integer promotion (see below),
/// and each argument of type float is implicitly converted to the type double.

/// Polyfill: The `va_copy` macro is from C99 standard, old compilers may not support it.
#if !defined(va_copy) && defined(__va_copy)
#define va_copy(dst, src) __va_copy(dst, src)
#elif !defined(va_copy)
// #define va_copy(dst, src) ((dst) = (src)) // this cause errors on some compilers
#define va_copy(dst, src) memcpy_((&dst), (&src), sizeof(va_list))
#endif

/// Using a global variable to avoid `malloc` (to support more platforms)
static struct dfp default_dfp;

static void memcpy_n(void *dest, void *src, int n) {
	char *s, *d;
	int i;
	for (i = 0, s = src, d = dest; i < n; i++)
		*d++ = *s++;
}

static inline int swap_arr_by_index(void *arr, int elem_size, int i1, int i2) {
	uint8_t tmp[elem_size];
	memcpy_n(tmp, arr + i1 * elem_size, elem_size);
	memcpy_n(arr + i1 * elem_size, arr + i2 * elem_size, elem_size);
	memcpy_n(arr + i2 * elem_size, tmp, elem_size);
	return 0;
}

static void reverse_arr(char *buf, int size) {
	int i, middle;

	middle = size / 2;
	for (i = 0; i < middle; i++)
		swap_arr_by_index(buf, 1, i, size - i - 1);
}

/// Unlike `putc` in standard C, `DFP_putc` returns 1
static int dfp_putc(struct dfp *self, int c) {
	char buf[2] = {0};
	buf[0] = c;
	self->puts(buf);
	return 1;
}

static int dfp_print_int_with_width(struct dfp *self, unsigned long long value, int width) {
	/// The decimal string of 2**64 have 20 characters, so 32B is enough for any 64-bit integer.
	/// And integer won't be bigger than 64-bit in the near future.
#define INT_BUFFER_SIZE 32
	char buffer[INT_BUFFER_SIZE];
	int i, j;

	if (value == 0 && width == 0) {
		dfp_putc(self, '0');
		return 1;
	}

	for (i = 0; i < INT_BUFFER_SIZE - 1 && value > 0; i++) {
		buffer[i] = value % 10 + '0';
		value /= 10;
	}

	if (width > i) {
		for (j = width - i; j > 0 && i < INT_BUFFER_SIZE - 1; j--)
			buffer[i++] = '0';
	}

	reverse_arr(buffer, i);
	buffer[i] = '\0';

	self->puts(buffer);
	return i;
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

static int dfp_step(struct dfp *self, struct fmt_parser_chunk *chunk, int *error) {
	if (chunk->type == FMT_CHAR)
		return dfp_putc(self, chunk->c);
	if (chunk->type == FMT_PLACEHOLDER_C)
		return dfp_putc(self, va_arg(self->args, int));
	if (chunk->type == FMT_PLACEHOLDER_LLD)
		return dfp_print_int_signed(self, va_arg(self->args, long long));
	if (chunk->type == FMT_PLACEHOLDER_LD)
		return dfp_print_int_signed(self, va_arg(self->args, long));
	if (chunk->type == FMT_PLACEHOLDER_D)
		return dfp_print_int_signed(self, va_arg(self->args, int));
	if (chunk->type == FMT_PLACEHOLDER_LLU)
		return dfp_print_int(self, va_arg(self->args, unsigned long long));
	if (chunk->type == FMT_PLACEHOLDER_LU)
		return dfp_print_int(self, va_arg(self->args, unsigned long));
	if (chunk->type == FMT_PLACEHOLDER_U)
		return dfp_print_int(self, va_arg(self->args, unsigned int));
	if (chunk->type == FMT_PLACEHOLDER_P)
		return dfp_print_int(self, va_arg(self->args, uintptr_t));
	if (chunk->type == FMT_PLACEHOLDER_F)
		return dfp_print_float(self, va_arg(self->args, double));
	if (chunk->type == FMT_PLACEHOLDER_S)
		return self->puts(va_arg(self->args, const char *));

	*error = 1;
	return 0;
}

int dfp_vprintf(struct dfp *self, const char *fmt, va_list args) {
	struct fmt_parser_chunk chunk;
	int n = 0;
	int error = 0;

	va_copy(self->args, args);
	if (fmt_parser_init(&self->parser, fmt))
		return -1;

	while (!fmt_parser_step(&self->parser, &chunk) && !error)
		n += dfp_step(self, &chunk, &error);

	/// dfp error (caused by `dfp_step`) or parser error (unfinished fmt)
	if (error || !fmt_parser_finished(&self->parser))
		n = -1;

	va_end(self->args);

	return n;
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
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = dfp_vprintf(&default_dfp, fmt, args);
	va_end(args);

	return ret;
}
