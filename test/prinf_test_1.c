#include "../src/dfp_lib.h"
#include <assert.h>
#include <stdio.h>

int my_puts(const char *s);

int main() {
	int tmp;
	long long tmp_ll;
	float tmp_f;

	assert(!DFP_PRINTF_INIT(my_puts));

	tmp = printf("string:\t\t\thello, world %u %llu ~\n", 0, (unsigned long long)10);
	// printf("\tsize: %d\n", tmp);
	assert(tmp == 30);

	tmp = DFP_PRINTF("string:\t\t\thello, world %u %llu ~\n", 0, (unsigned long long)10);
	// __dfp_printf("\tsize: %d\n", tmp);
	assert(tmp == 30);

	/// 0x1234567812345678 == 1311768465173141112
	tmp_ll = 0x1234567812345678;

	tmp = printf("long long:\t\t%d, %lld,\n", (int)tmp_ll, tmp_ll);
	// printf("size: %d\n", tmp);
	assert(tmp == 44);

	tmp = DFP_PRINTF("long long:\t\t%d, %lld,\n", (int)tmp_ll, tmp_ll);
	// __dfp_printf("size: %d\n", tmp);
	assert(tmp == 44);

	tmp_f = 3.1415926;

	printf("float:\t\t\t%f\n", tmp_f);
	DFP_PRINTF("float:\t\t\t%f\n", tmp_f);

	/// Special value to test the `0`s.
	printf("special long long:\t%llu\n", 140723709965000LU);
	DFP_PRINTF("special long long:\t%llu\n", 140723709965000LU);

	return 0;
}

int my_puts(const char *s) {
	return fputs(s, stdout);
}
