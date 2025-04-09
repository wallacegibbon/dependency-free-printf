#ifndef __FMT_PARSER_H
#define __FMT_PARSER_H

enum fmt_parser_chunk_type {
	FMT_SPECIFIER_D,
	FMT_SPECIFIER_LD,
	FMT_SPECIFIER_LLD,
	FMT_SPECIFIER_U,
	FMT_SPECIFIER_LU,
	FMT_SPECIFIER_LLU,
	FMT_SPECIFIER_X,
	FMT_SPECIFIER_LX,
	FMT_SPECIFIER_LLX,
	FMT_SPECIFIER_P,
	FMT_SPECIFIER_F,
	FMT_SPECIFIER_S,
	FMT_SPECIFIER_C,
	FMT_CHAR,
};

struct fmt_parser_chunk {
	enum fmt_parser_chunk_type type;
	union {
		char c;
		/* when width got supported, put the width data here */
	};
};

struct fmt_parser {
	const char *fmt;
};

int fmt_parser_init(struct fmt_parser *self, const char *fmt);
int fmt_parser_step(struct fmt_parser *self, struct fmt_parser_chunk *result);
int fmt_parser_finished(struct fmt_parser *self);

#endif
