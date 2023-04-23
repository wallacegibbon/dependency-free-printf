#include "dfp_lib.h"
#include <stdio.h>

int my_puts(const char *s) {
	fputs(s, stdout);
}

int main(int argc, const char **argv) {
	int a;
	long long b;
	float c;

	DFP_PRINTF_INIT();
	DFP_REGISTER_PUTS(my_puts);

	a = printf("hello, world\n");
	printf("\t>> size: %d\n", a);

	a = DFP_PRINTF("hello, world\n");
	DFP_PRINTF("\t>> size: %d\n", a);

	/// 0x1234567812345678 == 1311768465173141112
	b = 0x1234567812345678;
	printf("%d, %lld\n", (int) b, b);
	DFP_PRINTF("%d, %lld\n", (int) b, b);

	c = 3.1415926;
	printf("%f\n", c);
	DFP_PRINTF("%f\n", c);

	return 0;
}

