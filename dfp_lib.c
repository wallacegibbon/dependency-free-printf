#include "dfp_lib.h"
#include <stddef.h>

#define ABS_NUM_AND_RET_SIGN(num) (((num) < 0) ? (num = -num, 1) : 0)

#define ABS_NUM_AND_PUT_CHAR(self, value) (ABS_NUM_AND_RET_SIGN(value) \
	? DFP_putc(self, '-') \
	: 0)

#define swap_arr_by_index(arr, tmp, i1, i2) \
	do { tmp = arr[i1]; arr[i1] = arr[i2]; arr[i2] = tmp; } while (0)

#ifndef FLOAT_PASSING_TYPE
#define FLOAT_PASSING_TYPE double
#endif

/// Using a global variable to avoid `malloc` (to support more platforms)
static struct DFP my_dfp;

void DFP_initialize(struct DFP *self) {
	self->state = DFP_STATE_NORMAL;
	self->error = DFP_NO_ERROR;
	self->fmt = NULL;
	self->puts = NULL;
}

void DFP_register_puts(struct DFP *self, int (*puts)(const char *)) {
	self->puts = puts;
}

/// Unlike `putc` in standard C, `DFP_putc` returns 1
int DFP_putc(struct DFP *self, int c) {
	char buf[2] = { c, '\0' };
	self->puts(buf);
	return 1;
}

static void reverse(char *buf, int size) {
	int i, j;
	char tmp;

	for (i = 0, j = size / 2; i < j; i++)
		swap_arr_by_index(buf, tmp, i, size - i - 1);
}

int DFP_print_positive_integer(struct DFP *self, unsigned int value) {
	int i, n;

	for (i = 0; i < DFP_BUFFER_SIZE && value > 0; value /= 10, i++)
		self->buffer[i] = value % 10 + '0';

	n = i;
	reverse(self->buffer, n);
	self->buffer[n] = '\0';

	self->puts(self->buffer);
	return n;
}

int DFP_print_integer(struct DFP *self, unsigned int value) {
	if (value == 0)
		return DFP_putc(self, '0');
	else
		return DFP_print_positive_integer(self, value);
}

int DFP_print_integer_signed(struct DFP *self, int value) {
	int n = 0;

	n += ABS_NUM_AND_PUT_CHAR(self, value);
	n += DFP_print_integer(self, value);
	return n;
}

int DFP_print_long_integer(struct DFP *self, unsigned long long value) {
	int tmp1, tmp2, n = 0;

	tmp1 = value / 10000000000;
	value = value % 10000000000;
	if (tmp1 > 0) n += DFP_print_integer(self, tmp1);

	tmp1 = value / 1000;
	tmp2 = value % 1000;
	if (tmp1 > 0) n += DFP_print_integer(self, tmp1);

	n += DFP_print_integer(self, tmp2);
	return n;
}

int DFP_print_long_integer_signed(struct DFP *self, long long value) {
	int n = 0;

	n += ABS_NUM_AND_PUT_CHAR(self, value);
	n += DFP_print_long_integer(self, value);
	return n;
}

int DFP_print_p(struct DFP *self) {
	self->fmt++;
	return DFP_print_long_integer(self, va_arg(self->ap, unsigned long));
}

int DFP_print_u(struct DFP *self) {
	self->fmt++;
	return DFP_print_integer(self, va_arg(self->ap, unsigned int));
}

int DFP_print_d(struct DFP *self) {
	self->fmt++;
	return DFP_print_integer_signed(self, va_arg(self->ap, int));
}

/// return 1 when s1 and s2 equals
static int cmp(const char *s1, const char *s2, int size) {
	int cmp_value = 0;

	for (; !cmp_value && size; size--)
		cmp_value = *s1++ - *s2++;

	return !cmp_value && size == 0;
}

int DFP_print_lld(struct DFP *self) {
	self->fmt += 3;
	return DFP_print_long_integer_signed(self, va_arg(self->ap, long long));
}

int DFP_print_llu(struct DFP *self) {
	self->fmt += 3;
	return DFP_print_long_integer(self, va_arg(self->ap, unsigned long long));
}

int DFP_print_ld(struct DFP *self) {
	self->fmt += 2;
	return DFP_print_long_integer_signed(self, va_arg(self->ap, long));
}

int DFP_print_lu(struct DFP *self) {
	self->fmt += 2;
	return DFP_print_long_integer(self, va_arg(self->ap, unsigned long));
}

int DFP_print_l(struct DFP *self) {
	if (cmp(self->fmt, "lld", 3)) return DFP_print_lld(self);
	if (cmp(self->fmt, "llu", 3)) return DFP_print_llu(self);
	if (cmp(self->fmt, "ld", 2)) return DFP_print_ld(self);
	if (cmp(self->fmt, "lu", 2)) return DFP_print_lu(self);

	self->error = DFP_INVALID_FLAG;
	self->fmt++;
	return 0;
}

int DFP_print_f(struct DFP *self) {
	int tmp, n = 0;
	long long ltmp;
	float value;

	self->fmt++;
	value = va_arg(self->ap, FLOAT_PASSING_TYPE);

	ltmp = (long long) value;
	n += DFP_print_long_integer_signed(self, ltmp);

	DFP_putc(self, '.');
	n++;

	tmp = (int) ((value - ltmp) * 1000000);
	n += DFP_print_integer(self, tmp);

	return n;
}

int DFP_print_s(struct DFP *self) {
	self->fmt++;
	return self->puts(va_arg(self->ap, const char *));
}

int DFP_print_c(struct DFP *self) {
	self->fmt++;
	return DFP_putc(self, va_arg(self->ap, int));
}

int DFP_handle_normal(struct DFP *self) {
	const char current_char = *self->fmt;

	self->fmt++;
	if (current_char == '%') {
		self->state = DFP_STATE_FLAG;
		return 0;
	}

	return DFP_putc(self, current_char);
}

int DFP_handle_flag(struct DFP *self) {
	self->state = DFP_STATE_NORMAL;

	switch (*self->fmt) {
	case 'l': return DFP_print_l(self);
	case 'u': return DFP_print_u(self);
	case 'p': return DFP_print_p(self);
	case 'i':
	case 'd': return DFP_print_d(self);
	case 'f': return DFP_print_f(self);
	case 's': return DFP_print_s(self);
	case 'c': return DFP_print_c(self);
	default:
		self->error = DFP_INVALID_FLAG;
		return 0;
	}
}

int DFP_step(struct DFP *self) {
	if (self->error != DFP_NO_ERROR)
		return 0;

	switch (self->state) {
	case DFP_STATE_NORMAL: return DFP_handle_normal(self);
	case DFP_STATE_FLAG: return DFP_handle_flag(self);
	case DFP_STATE_WIDTH:
	default:
		self->error = DFP_WIDTH_UNSUPPORTED;
		return 0;
	}
}

int DFP_vprintf(struct DFP *self, const char *fmt, va_list ap) {
	int n = 0;

	self->fmt = fmt;
	va_copy(self->ap, ap);

	while (*self->fmt != '\0' && self->error == DFP_NO_ERROR)
		n += DFP_step(self);

	return n;
}

/// This is the function to be used by user.
int DFP_PRINTF(const char *fmt, ...) {
	va_list args;
	int ret;

	va_start(args, fmt);

	ret = DFP_vprintf(&my_dfp, fmt, args);
	if (my_dfp.error != DFP_NO_ERROR)
		ret = -1;

	va_end(args);

	return ret;
}

void DFP_PRINTF_INIT() {
	DFP_initialize(&my_dfp);
}

void DFP_REGISTER_PUTS(int (*puts)(const char *)) {
	DFP_register_puts(&my_dfp, puts);
}

