#ifndef __DFP_LIB_H
#define __DFP_LIB_H

#include <stdarg.h>

#ifndef DFP_BUFFER_SIZE
#define DFP_BUFFER_SIZE 32
#endif

// clang-format off
enum dfp_read_state { DFP_STATE_NORMAL, DFP_STATE_FLAG, DFP_STATE_WIDTH };
enum dfp_error_type { DFP_NO_ERROR, DFP_INVALID_FLAG, DFP_WIDTH_UNSUPPORTED };
// clang-format on

struct dfp {
	char buffer[DFP_BUFFER_SIZE];
	enum dfp_read_state state;
	enum dfp_error_type error;
	const char *fmt;
	va_list ap;
	int (*puts)(const char *s);
};

int DFP_PRINTF(const char *fmt, ...);
void DFP_PRINTF_INIT();
void DFP_REGISTER_PUTS(int (*puts)(const char *));

#endif
