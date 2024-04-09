#include "fmt_parser.h"
#include <stddef.h>

static int cmp_n(const char *s1, const char *s2, int size);

struct placeholder_table_item {
	const char *name;
	/// The number of placeholder strings will never be more than 256.
	unsigned char size;
	enum fmt_parser_data_t type;
};

static struct placeholder_table_item placeholder_table[] = {
	{"d", 1, FMT_PLACEHOLDER_D},
	{"i", 1, FMT_PLACEHOLDER_D},
	{"ld", 2, FMT_PLACEHOLDER_LD},
	{"li", 2, FMT_PLACEHOLDER_LD},
	{"lld", 3, FMT_PLACEHOLDER_LLD},
	{"lli", 3, FMT_PLACEHOLDER_LLD},
	{"u", 1, FMT_PLACEHOLDER_U},
	{"lu", 2, FMT_PLACEHOLDER_LU},
	{"llu", 3, FMT_PLACEHOLDER_LLU},
	{"p", 1, FMT_PLACEHOLDER_P},
	{"f", 1, FMT_PLACEHOLDER_F},
	{"s", 1, FMT_PLACEHOLDER_S},
	{"c", 1, FMT_PLACEHOLDER_C},
};

#define PLACEHOLDER_TABLE_SIZE (sizeof(placeholder_table) / sizeof(placeholder_table[0]))

static int find_placeholder_item(const char *s, struct placeholder_table_item **result) {
	struct placeholder_table_item *tmp;
	unsigned char i;
	for (i = 0; i < PLACEHOLDER_TABLE_SIZE; i++) {
		tmp = &placeholder_table[i];
		if (!cmp_n(s, tmp->name, tmp->size)) {
			*result = tmp;
			return 0;
		}
	}
	*result = NULL;
	return 1;
}

static int fmt_parser_placeholder(struct fmt_parser *self, struct fmt_parser_chunk *result) {
	struct placeholder_table_item *item;

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
