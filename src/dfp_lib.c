#include "dfp_lib.h"
#include "fmt_parser.h"
#include <stddef.h>
#include <stdint.h>

#define ABS_NUM_AND_RET_SIGN(num) (((num) < 0) ? (num = -num, 1) : 0)
#define ABS_NUM_AND_PUT_CHAR(self, value) (ABS_NUM_AND_RET_SIGN(value) ? dfp_putc(self, '-') : 0)

#ifndef FLOAT_PASSING_TYPE
#define FLOAT_PASSING_TYPE double
#endif

/// The `va_copy` macro is from C99 standard, old compilers may not support it.
#if !defined(va_copy) && defined(__va_copy)
#define va_copy(dst, src) __va_copy(dst, src)
#elif !defined(va_copy)
// #define va_copy(dst, src) ((dst) = (src)) // this cause errors on some compilers
#define va_copy(dst, src) memcpy_((&dst), (&src), sizeof(va_list))
#endif

/// Using a global variable to avoid `malloc` (to support more platforms)
static struct dfp default_dfp;

static void memcpy_n(void *dest, void *src, size_t n) {
	char *s, *d;
	size_t i;
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
	char buf[2];
	buf[0] = c;
	buf[1] = '\0';
	self->puts(buf);
	return 1;
}

static int dfp_print_integer(struct dfp *self, unsigned int value, int full_width) {
	int i, j;

	if (value == 0 && full_width == 0) {
		dfp_putc(self, '0');
		return 1;
	}

	for (i = 0; i < DFP_BUFFER_SIZE && value > 0; i++) {
		self->buffer[i] = value % 10 + '0';
		value /= 10;
	}

	if (full_width > i) {
		for (j = full_width - i; j > 0 && i < DFP_BUFFER_SIZE; j--)
			self->buffer[i++] = '0';
	}

	reverse_arr(self->buffer, i);
	self->buffer[i] = '\0';

	self->puts(self->buffer);
	return i;
}

static int dfp_print_integer_signed(struct dfp *self, int value) {
	int n;

	n = 0;
	n += ABS_NUM_AND_PUT_CHAR(self, value);
	n += dfp_print_integer(self, value, 0);
	return n;
}

static int dfp_print_long_integer(struct dfp *self, unsigned long long value) {
	uint32_t tmp, n;

	n = 0;
	/// tmp1 will fit into a 32bit integer since 10000000000 > 2**32.
	tmp = value / 10000000000;
	value = value % 10000000000;
	if (tmp > 0)
		n += dfp_print_integer(self, tmp, 0);

	/// The 10 here can also be 100, 1000, 10000... just to make the value fit into tmp1.
	/// But if you change it, you need to change the 9, 1 in the following dfp_print_integer, too.
	tmp = value / 10;
	value = value % 10;
	if (n > 0)
		n += dfp_print_integer(self, tmp, 9);
	else if (tmp > 0)
		n += dfp_print_integer(self, tmp, 0);
	else
		(void)n;

	if (n > 0)
		n += dfp_print_integer(self, value, 1);
	else
		n += dfp_print_integer(self, value, 0);

	return n;
}

static int dfp_print_long_integer_signed(struct dfp *self, long long value) {
	int n;

	n = 0;
	n += ABS_NUM_AND_PUT_CHAR(self, value);
	n += dfp_print_long_integer(self, value);
	return n;
}

static int dfp_print_float(struct dfp *self, FLOAT_PASSING_TYPE value) {
	int tmp, n;
	long long tmp_l;

	n = 0;
	tmp_l = (long long)value;
	n += dfp_print_long_integer_signed(self, tmp_l);

	dfp_putc(self, '.');
	n++;

	tmp = (int)((value - tmp_l) * 1000000);
	n += dfp_print_integer(self, tmp, 0);

	return n;
}

static int dfp_step(struct dfp *self, struct fmt_parser_chunk *chunk) {
	if (self->error)
		return 0;
	if (chunk->type == FMT_CHAR)
		return dfp_putc(self, chunk->c);
	if (chunk->type == FMT_PLACEHOLDER_C)
		return dfp_putc(self, va_arg(self->ap, int));
	if (chunk->type == FMT_PLACEHOLDER_LLD)
		return dfp_print_long_integer_signed(self, va_arg(self->ap, long long));
	if (chunk->type == FMT_PLACEHOLDER_LD)
		return dfp_print_long_integer_signed(self, va_arg(self->ap, long));
	if (chunk->type == FMT_PLACEHOLDER_D)
		return dfp_print_integer_signed(self, va_arg(self->ap, int));
	if (chunk->type == FMT_PLACEHOLDER_LLU)
		return dfp_print_long_integer(self, va_arg(self->ap, unsigned long long));
	if (chunk->type == FMT_PLACEHOLDER_LU)
		return dfp_print_long_integer(self, va_arg(self->ap, unsigned long long));
	if (chunk->type == FMT_PLACEHOLDER_U)
		return dfp_print_integer(self, va_arg(self->ap, unsigned int), 0);
	if (chunk->type == FMT_PLACEHOLDER_P)
		return dfp_print_long_integer(self, va_arg(self->ap, uintptr_t));
	if (chunk->type == FMT_PLACEHOLDER_F)
		return dfp_print_float(self, va_arg(self->ap, FLOAT_PASSING_TYPE));
	if (chunk->type == FMT_PLACEHOLDER_S)
		return self->puts(va_arg(self->ap, const char *));

	self->error = 1;
	return 0;
}

int dfp_vprintf(struct dfp *self, const char *fmt, va_list ap) {
	struct fmt_parser_chunk chunk;
	int n;

	va_copy(self->ap, ap);
	if (fmt_parser_init(&self->parser, fmt))
		return -1;

	n = 0;
	while (!fmt_parser_step(&self->parser, &chunk) && !self->error)
		n += dfp_step(self, &chunk);

	if (self->error)
		n = -1;

	va_end(self->ap);

	return n;
}

int dfp_init(struct dfp *self, dfp_puts_fn puts) {
	self->puts = puts;
	self->error = 0;
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
