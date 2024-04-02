#ifndef __DFP_LIB_H
#define __DFP_LIB_H

#include "fmt_parser.h"
#include <stdarg.h>

#ifndef DFP_BUFFER_SIZE
#define DFP_BUFFER_SIZE 32
#endif

typedef int (*dfp_puts_fn)(const char *);

struct dfp {
	struct fmt_parser parser;
	int (*puts)(const char *s);
	va_list ap;
	char buffer[DFP_BUFFER_SIZE + 1];
};

int dfp_init(struct dfp *self, dfp_puts_fn puts);
int dfp_replace_puts(struct dfp *self, dfp_puts_fn puts);
int dfp_vprintf(struct dfp *self, const char *fmt, va_list ap);

int DFP_PRINTF_INIT(dfp_puts_fn puts);
int DFP_PRINTF(const char *fmt, ...);

#endif
