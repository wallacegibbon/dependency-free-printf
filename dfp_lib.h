#ifndef __DFP_LIB_H
#define __DFP_LIB_H

#ifndef DFP_BUFFER_SIZE
#define DFP_BUFFER_SIZE 32
#endif

#include <stdarg.h>

enum DFP_ReadState { DFP_STATE_NORMAL, DFP_STATE_FLAG, DFP_STATE_WIDTH };
enum DFP_ErrorType { DFP_NO_ERROR, DFP_INVALID_FLAG, DFP_WIDTH_UNSUPPORTED };

struct DFP {
	char buffer[DFP_BUFFER_SIZE];
	enum DFP_ReadState state;
	enum DFP_ErrorType error;
	const char *fmt;
	va_list ap;
	int (*puts)(const char *s);
};

int DFP_PRINTF(const char *fmt, ...);
int DFP_PRINTF_INIT();
int DFP_REGISTER_PUTS(int (*puts)(const char *));

#endif

