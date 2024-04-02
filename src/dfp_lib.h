#ifndef __DFP_LIB_H
#define __DFP_LIB_H

#include "fmt_parser.h"
#include <stdarg.h>

#ifndef DFP_BUFFER_SIZE
#define DFP_BUFFER_SIZE 32
#endif

typedef int (*dfp_puts_fn)(const char *);

struct dfp {
	char buffer[DFP_BUFFER_SIZE + 1];
	struct fmt_parser parser;
	va_list ap;
	int error;
	int (*puts)(const char *s);
};

int dfp_init(struct dfp *self, dfp_puts_fn puts);
int dfp_replace_puts(struct dfp *self, dfp_puts_fn puts);
int dfp_vprintf(struct dfp *self, const char *fmt, va_list ap);

int DFP_PRINTF_INIT(dfp_puts_fn puts);
int DFP_PRINTF(const char *fmt, ...);

#endif
