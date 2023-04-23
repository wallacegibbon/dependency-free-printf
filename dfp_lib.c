#include "dfp_lib.h"
#include <stddef.h>

#define ABS_NUM_AND_RET_SIGN(num) (((num) < 0) ? (num = -num, 1) : 0)

#define ABS_NUM_AND_PUT_CHAR(self, value) ABS_NUM_AND_RET_SIGN(value) \
	? (DFP_putc(self, '-'), 1) \
	: 0

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

int DFP_putc(struct DFP *self, int c) {
	char buf[2] = { c, '\0' };

	self->puts(buf);
	return c;
}

static void reverse(char *buf, int size) {
	int i, j, k, tmp;

	for (i = 0, j = size / 2; i < j; i++) {
		k = size - i - 1;
		tmp = buf[i];
		buf[i] = buf[k];
		buf[k] = tmp;
	}
}

int DFP_print_integer(struct DFP *self, int value) {
	int i, n;

	for (i = 0; i < DFP_BUFFER_SIZE && value > 0; value /= 10, i++)
		self->buffer[i] = value % 10 + '0';

	n = i;
	reverse(self->buffer, n);
	self->buffer[n] = '\0';

	self->puts(self->buffer);

	return n;
}

int DFP_print_d(struct DFP *self, int value) {
	int n = 0;

	n += ABS_NUM_AND_PUT_CHAR(self, value);
	n += DFP_print_integer(self, value);

	return n;
}

int DFP_print_ll(struct DFP *self, long long value) {
	int tmp1, tmp2;
	int n = 0;

	n += ABS_NUM_AND_PUT_CHAR(self, value);

	tmp1 = value / 10000000000;
	value = value % 10000000000;
	n += DFP_print_integer(self, tmp1);

	tmp1 = value / 1000;
	tmp2 = value % 1000;
	n += DFP_print_integer(self, tmp1);

	n += DFP_print_integer(self, tmp2);

	return n;
}

int DFP_print_f(struct DFP *self, float value) {
	int n = 0;
	long long ltmp;
	int tmp;

	ltmp = (long long) value;
	n += DFP_print_ll(self, ltmp);

	DFP_putc(self, '.');
	n++;

	tmp = (int) ((value - ltmp) * 1000);
	n += DFP_print_integer(self, tmp);

	return n;
}

int DFP_handle_normal(struct DFP *self) {
	const char current_char = *self->fmt;
	int n = 0;

	switch (current_char) {
	case '%':
		self->state = DFP_STATE_FLAG;
		self->fmt++;
		break;
	default:
		DFP_putc(self, current_char);
		self->fmt++;
		n++;
	}
	return n;
}

/// return 1 when s1 and s2 equals
static int cmp(const char *s1, const char *s2, int size) {
	int cmp_value = 0;

	for (; !cmp_value && size; size--)
		cmp_value = *s1++ - *s2++;

	return !cmp_value && size == 0;
}

int DFP_handle_flag(struct DFP *self) {
	int n = 0;

	switch (*self->fmt) {
	case 'l':
		if (cmp(self->fmt, "lld", 3)) {
			self->fmt += 3;
			n = DFP_print_ll(self, va_arg(self->ap, long long));
		} else {
			self->error = DFP_INVALID_FLAG;
		}
		break;
	case 'd':
		self->fmt++;
		n = DFP_print_d(self, va_arg(self->ap, int));
		break;
	case 'f':
		self->fmt++;
		n = DFP_print_f(self, va_arg(self->ap, FLOAT_PASSING_TYPE));
		break;
	case 's':
		self->fmt++;
		n = self->puts(va_arg(self->ap, const char *));
		break;
	case 'c':
		self->fmt++;
		DFP_putc(self, va_arg(self->ap, int));
		n++;
		break;
	default:
		self->error = DFP_INVALID_FLAG;
	}

	self->state = DFP_STATE_NORMAL;
	return n;
}

int DFP_step(struct DFP *self) {
	if (self->error != DFP_NO_ERROR)
		return 0;

	switch (self->state) {
	case DFP_STATE_NORMAL:
		return DFP_handle_normal(self);
	case DFP_STATE_FLAG:
		return DFP_handle_flag(self);
	case DFP_STATE_WIDTH:
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

int DFP_PRINTF_INIT() {
	DFP_initialize(&my_dfp);
}

int DFP_REGISTER_PUTS(int (*puts)(const char *)) {
	DFP_register_puts(&my_dfp, puts);
}

