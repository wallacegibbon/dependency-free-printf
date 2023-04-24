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

	a = printf("hello, world %u %llu ~\n", 0, (unsigned long long) 10);
	printf("\t>> size: %d\n", a);

	a = DFP_PRINTF("hello, world %u %llu ~\n", 0, (unsigned long long) 10);
	DFP_PRINTF("\t>> size: %d\n", a);

	/// 0x1234567812345678 == 1311768465173141112
	b = 0x1234567812345678;
	a = printf("%d, %lld\n", (int) b, b);
	printf("\t>> size: %d\n", a);

	a = DFP_PRINTF("%d, %lld\n", (int) b, b);
	DFP_PRINTF("\t>> size: %d\n", a);

	c = 3.1415926;
	printf("%f\n", c);
	DFP_PRINTF("%f\n", c);

	printf("<*> %p %c\n", &c, '!');
	DFP_PRINTF("<*> %p %c\n", &c, '!');

	return 0;
}

