#ifndef __FMT_PARSER_H
#define __FMT_PARSER_H

enum fmt_parser_data_t {
	FMT_PLACEHOLDER_D,
	FMT_PLACEHOLDER_LD,
	FMT_PLACEHOLDER_LLD,
	FMT_PLACEHOLDER_U,
	FMT_PLACEHOLDER_LU,
	FMT_PLACEHOLDER_LLU,
	FMT_PLACEHOLDER_P,
	FMT_PLACEHOLDER_F,
	FMT_PLACEHOLDER_S,
	FMT_PLACEHOLDER_C,
	FMT_CHAR,
	FMT_INVALID,
};

struct fmt_parser_chunk {
	enum fmt_parser_data_t type;
	union {
		char c;
		/// when width got supported, put the width data here
	};
};

struct fmt_parser {
	const char *fmt;
};

int fmt_parser_init(struct fmt_parser *self, const char *fmt);
int fmt_parser_step(struct fmt_parser *self, struct fmt_parser_chunk *result);

#endif
