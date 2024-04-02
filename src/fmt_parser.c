#include "fmt_parser.h"
#include <stddef.h>

static int cmp_n(const char *s1, const char *s2, int size);

struct placeholder_table_item {
	int size;
	const char *fmt;
	enum fmt_parser_data_t type;
};

static const struct placeholder_table_item placeholder_table[] = {
	{1, "d", FMT_PLACEHOLDER_D},
	{1, "i", FMT_PLACEHOLDER_D},
	{2, "ld", FMT_PLACEHOLDER_LD},
	{2, "li", FMT_PLACEHOLDER_LD},
	{3, "lld", FMT_PLACEHOLDER_LLD},
	{3, "lli", FMT_PLACEHOLDER_LLD},
	{1, "u", FMT_PLACEHOLDER_U},
	{2, "lu", FMT_PLACEHOLDER_LU},
	{3, "llu", FMT_PLACEHOLDER_LLU},
	{1, "p", FMT_PLACEHOLDER_P},
	{1, "f", FMT_PLACEHOLDER_F},
	{1, "s", FMT_PLACEHOLDER_S},
	{1, "c", FMT_PLACEHOLDER_C},
};

#define PLACEHOLDER_TABLE_SIZE (sizeof(placeholder_table) / sizeof(placeholder_table[0]))

static int find_placeholder_item(const char *s, const struct placeholder_table_item **result) {
	const struct placeholder_table_item *tmp;
	size_t i;
	for (i = 0; i < PLACEHOLDER_TABLE_SIZE; i++) {
		tmp = &placeholder_table[i];
		if (!cmp_n(s, tmp->fmt, tmp->size)) {
			*result = tmp;
			return 0;
		}
	}
	*result = NULL;
	return 1;
}

static int fmt_parser_placeholder(struct fmt_parser *self, struct fmt_parser_chunk *result) {
	const struct placeholder_table_item *item;

	if (find_placeholder_item(self->fmt, &item))
		return 1;

	self->fmt += item->size;
	result->type = item->type;
	return 0;
}

static int fmt_parser_normal_char(struct fmt_parser *self, struct fmt_parser_chunk *result) {
	result->type = FMT_CHAR;
	result->c = *self->fmt;
	self->fmt++;
	return 0;
}

int fmt_parser_step(struct fmt_parser *self, struct fmt_parser_chunk *result) {
	if (fmt_parser_finished(self))
		return 1;

	if (*self->fmt != '%')
		return fmt_parser_normal_char(self, result);

	self->fmt++;

	/// '%%'
	if (*self->fmt == '%')
		return fmt_parser_normal_char(self, result);

	return fmt_parser_placeholder(self, result);
}

int fmt_parser_finished(struct fmt_parser *self) {
	return *self->fmt == '\0';
}

int fmt_parser_init(struct fmt_parser *self, const char *fmt) {
	self->fmt = fmt;
	return 0;
}

static int cmp_n(const char *s1, const char *s2, int size) {
	int tmp;
	for (tmp = 0; !tmp && size; size--)
		tmp = *s1++ - *s2++;

	return tmp;
}
