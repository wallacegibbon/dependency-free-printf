#include "../dfp_lib.h"
#include <assert.h>
#include <stdio.h>

int my_puts(const char *s);

int main(int argc, const char **argv) {
	int tmp;
	long long tmp_ll;
	float tmp_f;

	assert(!__dfp_printf_init());
	assert(!__dfp_register_puts(my_puts));

	tmp = printf("string:\t\t\thello, world %u %llu ~", 0, (unsigned long long)10);
	printf("\tsize: %d\n", tmp);

	tmp = __dfp_printf("string:\t\t\thello, world %u %llu ~", 0, (unsigned long long)10);
	__dfp_printf("\tsize: %d\n", tmp);

	/// 0x1234567812345678 == 1311768465173141112
	tmp_ll = 0x1234567812345678;

	tmp = printf("long long:\t\t%d, %lld,\t", (int)tmp_ll, tmp_ll);
	printf("size: %d\n", tmp);

	tmp = __dfp_printf("long long:\t\t%d, %lld,\t", (int)tmp_ll, tmp_ll);
	__dfp_printf("size: %d\n", tmp);

	tmp_f = 3.1415926;

	printf("float:\t\t\t%f\n", tmp_f);
	__dfp_printf("float:\t\t\t%f\n", tmp_f);

	/// Special value to test the `0`s.
	printf("special long long:\t%llu\n", 140723709965000LU);
	__dfp_printf("special long long:\t%llu\n", 140723709965000LU);

	return 0;
}

int my_puts(const char *s) {
	return fputs(s, stdout);
}
