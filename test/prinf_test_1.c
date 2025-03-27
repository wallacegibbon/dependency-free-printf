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

	tmp = printf("< string:\t\thello, world %u %llu %%\n",
			0, (unsigned long long)10);

	printf("< size: %d\n", tmp);

	tmp = DFP_PRINTF("> string:\t\thello, world %u %llu %%\n",
			0, (unsigned long long)10);

	DFP_PRINTF("> size: %d\n", tmp);

	/* 0x1234567812345678 == 1311768465173141112 */
	tmp_ll = 0x1234567812345678;

	tmp = printf("< long long:\t\t%d, %lld\n", (int)tmp_ll, tmp_ll);
	printf("< size: %d\n", tmp);
	assert(tmp == 45);

	tmp = DFP_PRINTF("> long long:\t\t%d, %lld\n", (int)tmp_ll, tmp_ll);
	DFP_PRINTF("> size: %d\n", tmp);
	assert(tmp == 45);

	tmp = printf("< long long:\t\t%x, %llx\n", (int)tmp_ll, tmp_ll);
	printf("< size: %d\n", tmp);

	tmp = DFP_PRINTF("> long long:\t\t%x, %llx\n", (int)tmp_ll, tmp_ll);
	DFP_PRINTF("> size: %d\n", tmp);

	tmp_f = 3.1415926;

	assert(printf("< float:\t\t%f\n", tmp_f) != -1);
	assert(DFP_PRINTF("> float:\t\t%f\n", tmp_f) != -1);

	assert(printf("< pointer:\t\t%p\n", &main) != -1);
	assert(DFP_PRINTF("< pointer:\t\t%p\n", &main) != -1);

	assert(printf("< pointer:\t\t%p\n", NULL) != -1);
	assert(DFP_PRINTF("< pointer:\t\t%p\n", NULL) != -1);

	/* invalid specifier %t */
	printf("unkown %t.\n");

	/* DFP will print it directly unlike printf. (for debugging) */
	assert(DFP_PRINTF("unkown %t.\n") == 11);

	/* DFP treat %x as %u to save code space */
	printf("< %%x test: %x\n", 0xab);
	DFP_PRINTF("> %%x test: %x\n", 0xab);

	return 0;
}

int my_puts(const char *s)
{
	return fputs(s, stdout);
}
