#include "dfp_lib.h"
#include <stddef.h>
#include <stdint.h>

#define ABS_NUM_AND_RET_SIGN(num) (((num) < 0) ? (num = -num, 1) : 0)
#define ABS_NUM_AND_PUT_CHAR(self, value) (ABS_NUM_AND_RET_SIGN(value) ? dfp_putc(self, '-') : 0)

#ifndef FLOAT_PASSING_TYPE
#define FLOAT_PASSING_TYPE double
#endif

/// Using a global variable to avoid `malloc` (to support more platforms)
static struct dfp default_dfp;

static void __memcpy(void *dest, void *src, size_t n) {
	char *s, *d;
	int i;
	for (i = 0, s = src, d = dest; i < n; i++)
		*d++ = *s++;
}

static inline int swap_arr_by_index(void *arr, int elem_size, int i1, int i2) {
	uint8_t tmp[elem_size];
	__memcpy(tmp, arr + i1 * elem_size, elem_size);
	__memcpy(arr + i1 * elem_size, arr + i2 * elem_size, elem_size);
	__memcpy(arr + i2 * elem_size, tmp, elem_size);
	return 0;
}

void dfp_initialize(struct dfp *self) {
	self->state = DFP_STATE_NORMAL;
	self->error = DFP_NO_ERROR;
	self->fmt = NULL;
	self->puts = NULL;
}

int dfp_register_puts(struct dfp *self, int (*puts)(const char *)) {
	self->puts = puts;
	return 0;
}

/// Unlike `putc` in standard C, `DFP_putc` returns 1
int dfp_putc(struct dfp *self, int c) {
	char buf[2];
	buf[0] = c;
	buf[1] = '\0';
	self->puts(buf);
	return 1;
}

static void reverse(char *buf, int size) {
	int i, j, tmp;

	for (i = 0, j = size / 2; i < j; i++)
		swap_arr_by_index(buf, 1, i, size - i - 1);
}

int dfp_print_positive_integer(struct dfp *self, unsigned int value, int full_width) {
	int i, n;

	for (i = 0; i < DFP_BUFFER_SIZE && value > 0; value /= 10, i++)
		self->buffer[i] = value % 10 + '0';

	if (full_width > i) {
		n = full_width - i;
		for (; i < DFP_BUFFER_SIZE && n > 0; i++, n--)
			self->buffer[i] = '0';
	}

	n = i;
	reverse(self->buffer, n);
	self->buffer[n] = '\0';

	self->puts(self->buffer);
	return n;
}

int dfp_print_integer(struct dfp *self, unsigned int value, int full_width) {
	int i;

	if (value < 0)
		return 0;
	if (value > 0)
		return dfp_print_positive_integer(self, value, full_width);

	if (full_width == 0)
		return dfp_putc(self, '0');

	for (i = 0; i < DFP_BUFFER_SIZE && i < full_width; i++)
		self->buffer[i] = '0';

	self->buffer[i] = '\0';
	self->puts(self->buffer);

	return full_width;
}

int dfp_print_integer_signed(struct dfp *self, int value) {
	int n;

	n = 0;
	n += ABS_NUM_AND_PUT_CHAR(self, value);
	n += dfp_print_integer(self, value, 0);
	return n;
}

int dfp_print_long_integer(struct dfp *self, unsigned long long value) {
	unsigned int tmp1, tmp2, n;

	n = 0;
	/// 10000000000 > 2**32, tmp1 will fit into a 32bit integer.
	tmp1 = value / 10000000000;
	value = value % 10000000000;
	if (tmp1 > 0)
		n += dfp_print_integer(self, tmp1, 0);

	/// The 10 here can also be 100, 1000, 10000... just to make the value fit into tmp1.
	/// But if you change it, you need to change the 9, 1 in the following dfp_print_integer, too.
	tmp1 = value / 10;
	tmp2 = value % 10;
	if (n > 0)
		n += dfp_print_integer(self, tmp1, 9);
	else if (tmp1 > 0)
		n += dfp_print_integer(self, tmp1, 0);
	else
		/// when n == 0 && tmp1 == 0, do not print anything.
		(void)n;

	if (n > 0)
		n += dfp_print_integer(self, tmp2, 1);
	else
		n += dfp_print_integer(self, tmp2, 0);

	return n;
}

int dfp_print_long_integer_signed(struct dfp *self, long long value) {
	int n;

	n = 0;
	n += ABS_NUM_AND_PUT_CHAR(self, value);
	n += dfp_print_long_integer(self, value);
	return n;
}

int dfp_print_p(struct dfp *self) {
	self->fmt++;
	return dfp_print_long_integer(self, va_arg(self->ap, unsigned long));
}

int dfp_print_u(struct dfp *self) {
	self->fmt++;
	return dfp_print_integer(self, va_arg(self->ap, unsigned int), 0);
}

int dfp_print_d(struct dfp *self) {
	self->fmt++;
	return dfp_print_integer_signed(self, va_arg(self->ap, int));
}

/// return 1 when s1 and s2 equals
static int cmp(const char *s1, const char *s2, int size) {
	int cmp_value;

	for (cmp_value = 0; !cmp_value && size; size--)
		cmp_value = *s1++ - *s2++;

	return !cmp_value && size == 0;
}

int dfp_print_lld(struct dfp *self) {
	self->fmt += 3;
	return dfp_print_long_integer_signed(self, va_arg(self->ap, long long));
}

int dfp_print_llu(struct dfp *self) {
	self->fmt += 3;
	return dfp_print_long_integer(self, va_arg(self->ap, unsigned long long));
}

int dfp_print_ld(struct dfp *self) {
	self->fmt += 2;
	return dfp_print_long_integer_signed(self, va_arg(self->ap, long));
}

int dfp_print_lu(struct dfp *self) {
	self->fmt += 2;
	return dfp_print_long_integer(self, va_arg(self->ap, unsigned long));
}

int dfp_print_l(struct dfp *self) {
	if (cmp(self->fmt, "lld", 3))
		return dfp_print_lld(self);
	if (cmp(self->fmt, "llu", 3))
		return dfp_print_llu(self);
	if (cmp(self->fmt, "ld", 2))
		return dfp_print_ld(self);
	if (cmp(self->fmt, "lu", 2))
		return dfp_print_lu(self);

	self->error = DFP_INVALID_FLAG;
	self->fmt++;
	return 0;
}

int dfp_print_f(struct dfp *self) {
	long long ltmp;
	int tmp, n;
	float value;

	self->fmt++;
	value = va_arg(self->ap, FLOAT_PASSING_TYPE);

	n = 0;
	ltmp = (long long)value;
	n += dfp_print_long_integer_signed(self, ltmp);

	dfp_putc(self, '.');
	n++;

	tmp = (int)((value - ltmp) * 1000000);
	n += dfp_print_integer(self, tmp, 0);

	return n;
}

int dfp_print_s(struct dfp *self) {
	self->fmt++;
	return self->puts(va_arg(self->ap, const char *));
}

int dfp_print_c(struct dfp *self) {
	self->fmt++;
	return dfp_putc(self, va_arg(self->ap, int));
}

int dfp_handle_normal(struct dfp *self) {
	char current_char;

	current_char = *self->fmt;
	self->fmt++;
	if (current_char == '%') {
		self->state = DFP_STATE_FLAG;
		return 0;
	}

	return dfp_putc(self, current_char);
}

int dfp_handle_flag(struct dfp *self) {
	self->state = DFP_STATE_NORMAL;

	switch (*self->fmt) {
	case 'l':
		return dfp_print_l(self);
	case 'u':
		return dfp_print_u(self);
	case 'p':
		return dfp_print_p(self);
	case 'i':
	case 'd':
		return dfp_print_d(self);
	case 'f':
		return dfp_print_f(self);
	case 's':
		return dfp_print_s(self);
	case 'c':
		return dfp_print_c(self);
	default:
		self->error = DFP_INVALID_FLAG;
		return 0;
	}
}

int dfp_step(struct dfp *self) {
	if (self->error != DFP_NO_ERROR)
		return 0;

	switch (self->state) {
	case DFP_STATE_NORMAL:
		return dfp_handle_normal(self);
	case DFP_STATE_FLAG:
		return dfp_handle_flag(self);
	case DFP_STATE_WIDTH:
	default:
		self->error = DFP_WIDTH_UNSUPPORTED;
		return 0;
	}
}

/// The `va_copy` macro is from C99 standard, old compilers may not support it.
#if !defined(va_copy) && defined(__va_copy)
#define va_copy(dst, src) __va_copy(dst, src)
#elif !defined(va_copy)
// #define va_copy(dst, src) ((dst) = (src)) // this cause some errors on some compilers
#define va_copy(dst, src) memcpy_((&dst), (&src), sizeof(va_list))
#endif

int dfp_vprintf(struct dfp *self, const char *fmt, va_list ap) {
	int n;

	self->fmt = fmt;
	va_copy(self->ap, ap);

	n = 0;
	while (*self->fmt != '\0' && self->error == DFP_NO_ERROR)
		n += dfp_step(self);

	return n;
}

/// This is the function to be used by user.
int __dfp_printf(const char *fmt, ...) {
	va_list args;
	int ret;

	va_start(args, fmt);

	ret = dfp_vprintf(&default_dfp, fmt, args);
	if (default_dfp.error != DFP_NO_ERROR)
		ret = -1;

	va_end(args);

	return ret;
}

int __dfp_printf_init() {
	dfp_initialize(&default_dfp);
	return 0;
}

int __dfp_register_puts(int (*puts)(const char *)) {
	if (dfp_register_puts(&default_dfp, puts))
		return 1;
	else
		return 0;
}
