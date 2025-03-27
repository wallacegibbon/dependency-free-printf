#include "fmt_parser.h"
#include <stddef.h>

static int cmp_n(const char *s1, const char *s2, int size);

struct specifier_unit {
	const char *name;
	/* The number of specifier chars is less than 3 */
	unsigned char size;
	enum fmt_parser_chunk_type type;
};

static const struct specifier_unit specifier_table[] = {
	{"d", 1, FMT_SPECIFIER_D},
	{"i", 1, FMT_SPECIFIER_D},
	{"ld", 2, FMT_SPECIFIER_LD},
	{"li", 2, FMT_SPECIFIER_LD},
	{"lld", 3, FMT_SPECIFIER_LLD},
	{"lli", 3, FMT_SPECIFIER_LLD},
	{"u", 1, FMT_SPECIFIER_U},
	{"lu", 2, FMT_SPECIFIER_LU},
	{"llu", 3, FMT_SPECIFIER_LLU},
	{"x", 1, FMT_SPECIFIER_X},
	{"lx", 2, FMT_SPECIFIER_LX},
	{"llx", 3, FMT_SPECIFIER_LLX},
	{"p", 1, FMT_SPECIFIER_P},
	{"f", 1, FMT_SPECIFIER_F},
	{"s", 1, FMT_SPECIFIER_S},
	{"c", 1, FMT_SPECIFIER_C},
};

#define SPECIFIER_TABLE_SIZE \
		(sizeof(specifier_table) / sizeof(specifier_table[0]))

static int find_specifier_item(const char *s,
		const struct specifier_unit **result)
{
	const struct specifier_unit *tmp;
	unsigned char i;
	for (i = 0; i < SPECIFIER_TABLE_SIZE; i++) {
		tmp = &specifier_table[i];
		if (!cmp_n(s, tmp->name, tmp->size)) {
			*result = tmp;
			return 0;
		}
	}
	*result = NULL;
	return 1;
}

static int fmt_parser_specifier(struct fmt_parser *self,
		struct fmt_parser_chunk *result)
{
	const struct specifier_unit *item;

	if (find_specifier_item(self->fmt, &item))
		return 1;

	self->fmt += item->size;
	result->type = item->type;
	return 0;
}

static int fmt_parser_normal_char(struct fmt_parser *self,
		struct fmt_parser_chunk *result)
{
	result->type = FMT_CHAR;
	result->c = *self->fmt;
	self->fmt++;
	return 0;
}

int fmt_parser_step(struct fmt_parser *self, struct fmt_parser_chunk *result)
{
	if (fmt_parser_finished(self))
		return 1;

	if (*self->fmt != '%')
		return fmt_parser_normal_char(self, result);

	self->fmt++;

	/* '%%' */
	if (*self->fmt == '%')
		return fmt_parser_normal_char(self, result);

	if (fmt_parser_specifier(self, result) == 0)
		return 0;

	/* unknown control char, just print it as normal char */
	self->fmt--;
	return fmt_parser_normal_char(self, result);
}

int fmt_parser_finished(struct fmt_parser *self)
{
	return *self->fmt == '\0';
}

int fmt_parser_init(struct fmt_parser *self, const char *fmt)
{
	self->fmt = fmt;
	return 0;
}

static int cmp_n(const char *s1, const char *s2, int size)
{
	int tmp;
	for (tmp = 0; !tmp && size; size--)
		tmp = *s1++ - *s2++;

	return tmp;
}
