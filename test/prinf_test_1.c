#include "dfp_lib.h"
#include <assert.h>
#include <stdio.h>

int my_puts(const char *s);

int main()
{
	long long tmp_ll;
	float tmp_f;
	int tmp;

	assert(!DFP_PRINTF_INIT(my_puts));

	tmp = printf("< string:\t\t\thello, world %u %llu %%\n",
			0, (unsigned long long)10);

	printf("< size: %d\n", tmp);
	assert(tmp == 32);

	tmp = DFP_PRINTF("> string:\t\t\thello, world %u %llu %%\n",
			0, (unsigned long long)10);

	DFP_PRINTF("> size: %d\n", tmp);
	assert(tmp == 32);

	/* 0x1234567812345678 == 1311768465173141112 */
	tmp_ll = 0x1234567812345678;

	tmp = printf("< long long:\t\t%d, %lld\n", (int)tmp_ll, tmp_ll);
	printf("< size: %d\n", tmp);
	assert(tmp == 45);

	tmp = DFP_PRINTF("> long long:\t\t%d, %lld\n", (int)tmp_ll, tmp_ll);
	DFP_PRINTF("> size: %d\n", tmp);
	assert(tmp == 45);

	tmp_f = 3.1415926;

	assert(printf("< float:\t\t\t%f\n", tmp_f) != -1);
	assert(DFP_PRINTF("> float:\t\t\t%f\n", tmp_f) != -1);

	/* invalid placeholder %t */
	printf("unkown %t.\n");

	/* DFP will print it directly unlike printf. (for debugging) */
	assert(DFP_PRINTF("unkown %t.\n") == 11);

	return 0;
}

int my_puts(const char *s)
{
	return fputs(s, stdout);
}
