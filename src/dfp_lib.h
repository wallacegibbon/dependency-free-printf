#ifndef __DFP_LIB_H
#define __DFP_LIB_H

#include "fmt_parser.h"
#include <stdarg.h>

typedef int (*dfp_puts_fn)(const char *);

struct dfp {
	int (*puts)(const char *s);
	va_list args;
};

int dfp_init(struct dfp *self, dfp_puts_fn puts);
int dfp_replace_puts(struct dfp *self, dfp_puts_fn puts);
int dfp_vprintf(struct dfp *self, const char *fmt, va_list ap);

int DFP_PRINTF_INIT(dfp_puts_fn puts);
int DFP_PRINTF(const char *fmt, ...);

#endif
