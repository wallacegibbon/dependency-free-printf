#include "../src/fmt_parser.h"
#include <assert.h>

int main() {
	struct fmt_parser parser;
	struct fmt_parser_chunk chunk;

	fmt_parser_init(&parser, "%lld,%lli,%llu,%s\n");

	assert(!fmt_parser_step(&parser, &chunk));
	assert(chunk.type == FMT_PLACEHOLDER_LLD);

	assert(!fmt_parser_step(&parser, &chunk));
	assert(chunk.type == FMT_CHAR);
	assert(chunk.c == ',');

	assert(!fmt_parser_step(&parser, &chunk));
	assert(chunk.type == FMT_PLACEHOLDER_LLD);

	assert(!fmt_parser_step(&parser, &chunk));
	assert(chunk.type == FMT_CHAR);
	assert(chunk.c == ',');

	assert(!fmt_parser_step(&parser, &chunk));
	assert(chunk.type == FMT_PLACEHOLDER_LLU);

	assert(!fmt_parser_step(&parser, &chunk));
	assert(chunk.type == FMT_CHAR);
	assert(chunk.c == ',');

	assert(!fmt_parser_step(&parser, &chunk));
	assert(chunk.type == FMT_PLACEHOLDER_S);

	assert(!fmt_parser_step(&parser, &chunk));
	assert(chunk.type == FMT_CHAR);
	assert(chunk.c == '\n');

	assert(fmt_parser_step(&parser, &chunk));

	return 0;
}
